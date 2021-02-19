/*
 * ahimagedb.cpp
 *
 *  Created on: Jul 22, 2019
 *      Author: Jonathan Ni
 */
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dir.h>
#include <wx/string.h>
#include <wx/filename.h>

#include <sstream>
#include <chrono>
#include <unordered_set>

#include "ahframe.h"
#include "ahcanvas.h"
#include "ahimagedb.h"
#include "ahutil.h"
#include "ahlogwindow.h"
#include "parallel_hashmap/phmap.h"

AHImageDB::AHImageDB(AHFrame *frame) :
		m_frame(frame), ordered_names(new std::multiset<wxString>), memory_used(
				0), current_index(0) {
	images = new phmap::parallel_flat_hash_map<wxString, wxImage*,
			hash_default_hash<wxString>, hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, wxImage*>>, 4, std::mutex>;
	image_times = new phmap::parallel_flat_hash_map<wxString, time_t,
			hash_default_hash<wxString>, hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, time_t>>, 4, std::mutex>;
	image_dirty = new phmap::parallel_flat_hash_map<wxString, imageFlag,
			hash_default_hash<wxString>, hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, imageFlag>>, 4, std::mutex>;
	current = ordered_names->end();

	std::future<void> fut_cwd = prom_cwd.get_future(), fut_log =
			prom_log.get_future(), fut_img = prom_img.get_future(), fut_loop =
			prom_loop.get_future();

	watch_cwd = std::thread(&AHImageDB::Watch<void (AHImageDB::*)()>, this,
			std::move(fut_cwd), 15000, &AHImageDB::RefreshCwd);
	//watch_log = std::thread(&AHImageDB::Watch<void (AHImageDB::*)()>, this,
	//		std::move(fut_log), 1000, &AHImageDB::RefreshLog);
	watch_img = std::thread(&AHImageDB::Watch<void (AHImageDB::*)()>, this,
			std::move(fut_img), 1000, &AHImageDB::RefreshImg);
	watch_loop = std::thread(&AHImageDB::LoopImg, this, std::move(fut_loop));
}

AHImageDB::~AHImageDB() {
	prom_cwd.set_value();
	prom_log.set_value();
	prom_img.set_value();

	// AHFrame not deleted (deletes itself)

	for (auto it = images->begin(); it != images->end(); it++)
		delete (*it).second;

	delete images;
	delete image_times;
	delete image_dirty;

	delete ordered_names;
}

template<typename F>
void AHImageDB::Watch(std::future<void> fObj, unsigned int milli, F callback) {
	while (fObj.wait_for(std::chrono::milliseconds(1))
			== std::future_status::timeout) {
		m_frame->SetStatusText("Status: Refreshing");
		(this->*callback)();
		m_frame->SetStatusText("Status:");
		std::this_thread::sleep_for(std::chrono::milliseconds(milli));
	}
}

void AHImageDB::RefreshLog() {
	std::ostringstream stream;
	auto f_dirty = [this](wxString &key) {
		switch (image_dirty->at(key)) {
		case IMAGE_CLEAN:
			return "IMAGE_CLEAN";
		case IMAGE_DIRTY:
			return "IMAGE_DIRTY";
		case IMAGE_UNLOAD:
			return "IMAGE_UNLOAD";
		case IMAGE_REMOVE:
			return "IMAGE_REMOVE";
		}
		return "null";
	};

	std::multiset<wxString> *names;

	{
		std::unique_lock<std::mutex> lock(mut_ord);
		names = new std::multiset<wxString>(*ordered_names);
	}

	for (wxString key : *names)
		stream << key.c_str() << " : " << image_times->at(key) << " "
				<< f_dirty(key) << std::endl;

	m_frame->log_window->SetText(stream.str());
}

void AHImageDB::RefreshCwd() {
	std::unique_lock<std::mutex> lock(mut_rcwd);

	wxString cwd;

	{
		std::unique_lock<std::mutex> lock(mut_cwd);
		cwd = wxGetCwd();
	}

	wxDir cur_folder(cwd);

	if (!cur_folder.IsOpened()) {
		wxMessageBox("Could not open directory.", "Error", wxOK | wxICON_ERROR);
		return;
	}

	if (!cur_folder.HasFiles()) {
		wxMessageBox("Directory has no files.", "Error",
		wxOK | wxICON_INFORMATION);
		return;
	}

	wxArrayString *names = new wxArrayString;
	std::unordered_set<wxString> *us_names = new std::unordered_set<wxString>;

	m_frame->SetStatusText("Status: Loading file names...");

	size_t total = wxDir::GetAllFiles(wxGetCwd(), names, wxEmptyString,
			wxDIR_FILES);

	for (unsigned int i = 0; i < total; i++)
		us_names->insert((*names)[i]);

	// Mark old files for removal
	for (wxString key : *ordered_names)
		if (us_names->find(key) == us_names->end()
				|| (wxFileModificationTime(key) != this->image_times->at(key)))
			(*image_dirty)[key] = IMAGE_REMOVE;

	std::ostringstream status;
	wxLogNull log_no;

	// Add new or modified files
	for (unsigned int i = 0; i < total; i++) {
		wxString fullName = (*names)[i];

		if (image_times->find(fullName) != image_times->end())
			continue; // Already in map

		status << "Status: " << (i + 1) << "/" << total << " "
				<< fullName.c_str();

		if (ahutil::isPNG(fullName)) {
			status << " OK";
			(*images)[fullName] = NULL;
			(*image_times)[fullName] = wxFileModificationTime(fullName);
			(*image_dirty)[fullName] = IMAGE_UNLOAD;

			std::unique_lock<std::mutex> lock(mut_ord);
			ordered_names->insert(fullName);
		}

		status.flush();

		m_frame->SetStatusText(wxString(status.str()));

		status.str("");
	}

	bool refresh = false;
	{
		std::unique_lock<std::mutex> lock1(mut_current, std::defer_lock);
		std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

		std::lock(lock1, lock2);

		if (current == ordered_names->end()) {
			current = ordered_names->begin();
			current_index = 0;
			refresh = true;
		}
	}

	if (refresh)
		m_frame->RefreshIndex();
}

void AHImageDB::PrevImage() {
	std::unique_lock<std::mutex> lock1(mut_current, std::defer_lock);
	std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

	std::lock(lock1, lock2);

	if (ordered_names->size() == 0)
		return;

	if (current != ordered_names->begin()) {
		current--;
		current_index--;
		return;
	}

	if (m_frame->loop->GetValue()) {
		current = ordered_names->end();
		current--;
		current_index = ordered_names->size() - 1;
	}
}

void AHImageDB::NextImage() {
	std::unique_lock<std::mutex> lock1(mut_current, std::defer_lock);
	std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

	std::lock(lock1, lock2);

	if (ordered_names->size() == 0)
		return;

	if (std::next(current) != ordered_names->end()) {
		current++;
		current_index++;
		return;
	}

	if (m_frame->loop->GetValue()) {
		current = ordered_names->begin();
		current_index = 0;
	}
}

void AHImageDB::LoadImageFile(wxString path) {
	(*images)[path] = new wxImage(path, wxBITMAP_TYPE_PNG);
	memory_used += ahutil::getImageSize(images->at(path));
}

void AHImageDB::RefreshImg() {
	{
		std::unique_lock<std::mutex> lock(mut_img);

		// Remove old files
		for (auto it = images->begin(); it != images->end();) {
			const wxString key = it->first;
			if (image_dirty->at(key) == IMAGE_REMOVE) {
				if (images->at(key) != nullptr)
					memory_used -= ahutil::getImageSize(images->at(key)); // Reduce mem

				std::unique_lock<std::mutex> lock1(mut_current,
						std::defer_lock);
				std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

				std::lock(lock1, lock2);

				if (key.IsSameAs(*current)) {
					if (current == ordered_names->begin()) {
						std::cout << key << std::endl;
						current++;
					} else {
						current--;
						current_index--;
					}
				}
				it = images->erase(it);
			} else {
				if (image_dirty->at(key) == IMAGE_UNLOAD
						&& images->at(key) != nullptr) {
					memory_used -= ahutil::getImageSize(images->at(key)); // Reduce mem
					delete images->at(key);
					(*images)[key] = nullptr;
				}
				++it;
			}
		}

		{
			std::unique_lock<std::mutex> lock1(mut_ord);
			ahutil::erase_if(*ordered_names, [this](const wxString key) {
				return this->image_dirty->at(key) == IMAGE_REMOVE;
			});
		}

		ahutil::erase_if(*image_times,
				[this](const std::pair<wxString, time_t> item) {
					const wxString key = item.first;
					return this->image_dirty->at(key) == IMAGE_REMOVE;
				});
	}

	{
		//std::unique_lock<std::mutex> lock1(mut_current, std::defer_lock);
		//std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

		//std::lock(lock1, lock2);

		unsigned int i = 0;
		for (auto it = ordered_names->begin(); it != ordered_names->end();
				it++, i++) {
			if ((int) i
					< (int) current_index
							- (int) (AH_BACK_MEMORY_SEC * m_frame->fps)
					|| i >= current_index + AH_FWD_MEMORY_SEC * m_frame->fps) {
				if (image_dirty->at(*it) == IMAGE_CLEAN)
					(*image_dirty)[*it] = IMAGE_DIRTY;
				else if (image_dirty->at(*it) == IMAGE_DIRTY)
					(*image_dirty)[*it] = IMAGE_UNLOAD;
			}

			if (memory_used.load() > AH_MEMORY_LIMIT)
				continue;

			if (i >= current_index
					&& i < current_index + AH_FWD_MEMORY_SEC * m_frame->fps) {
				if (image_dirty->at(*it) == IMAGE_UNLOAD
						&& images->at(*it) == nullptr) {
					std::thread th(&AHImageDB::LoadImageFile, this, *it);
					th.detach();
				}

				(*image_dirty)[*it] = IMAGE_CLEAN;
			}
		}
	}
}

void AHImageDB::LoopImg(std::future<void> fObj) {
	while (fObj.wait_for(std::chrono::milliseconds(1))
			== std::future_status::timeout) {
		if (!GetCurrent().IsSameAs(wxEmptyString)) {
			if (m_frame->canvas->setTexture(images->at(GetCurrent())))
				m_frame->canvas->Refresh();
		}

		unsigned long long sleep;
		if (m_frame->IsPlay()) {
			sleep = 1000000LL / m_frame->fps.load();
			{
				std::unique_lock<std::mutex> lock1(mut_current,
						std::defer_lock);
				std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

				std::lock(lock1, lock2);

				std::multiset<wxString>::iterator old(current);

				++current_index;
				if (++current == ordered_names->end()) {
					if (m_frame->loop->GetValue()) {
						current = ordered_names->begin();
						current_index.store(0);
					} else {
						current = std::multiset<wxString>::iterator(old);
						current_index.store(ordered_names->size() - 1);
						m_frame->PlayPause();
					}
				}
			}

			m_frame->RefreshIndex();
		} else
			sleep = AH_DEFAULT_IMG_REFRESH_MS * 1000LL;

		std::this_thread::sleep_for(std::chrono::microseconds(sleep));
	}
}

wxString AHImageDB::GetCurrent() {
	std::unique_lock<std::mutex> lock1(mut_current, std::defer_lock);
	std::unique_lock<std::mutex> lock2(mut_ord, std::defer_lock);

	std::lock(lock1, lock2);

	if (current == ordered_names->end())
		return wxEmptyString;

	return *current;
}
