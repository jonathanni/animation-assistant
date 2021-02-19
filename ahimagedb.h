/*
 * ahimagedb.h
 *
 *  Created on: Jul 22, 2019
 *      Author: Jonathan Ni
 */

#ifndef AHIMAGEDB_H_
#define AHIMAGEDB_H_

#include <vector>
#include <wx/string.h>
#include <future>
#include <thread>
#include <mutex>
#include <set>
#include <atomic>

#include "parallel_hashmap/phmap_fwd_decl.h"

using namespace phmap::container_internal;

class AHFrame;

enum imageFlag {
	IMAGE_CLEAN, IMAGE_DIRTY, IMAGE_UNLOAD, IMAGE_REMOVE
};

class AHImageDB {
public:
	std::mutex mut_cwd;

	AHImageDB(AHFrame *frame);
	~AHImageDB();

	void RefreshCwd();
	void RefreshLog();
	void RefreshImg();

	wxString GetCurrent();

	void PrevImage();
	void NextImage();

	void LoadImageFile(wxString path);
private:
	AHFrame *m_frame;
	phmap::parallel_flat_hash_map<wxString, wxImage*,
			hash_default_hash<wxString>, hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, wxImage*>>, 4, std::mutex> *images;
	phmap::parallel_flat_hash_map<wxString, time_t, hash_default_hash<wxString>,
			hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, time_t>>, 4, std::mutex> *image_times;
	phmap::parallel_flat_hash_map<wxString, imageFlag,
			hash_default_hash<wxString>, hash_default_eq<wxString>,
			Allocator<std::pair<const wxString, imageFlag>>, 4, std::mutex> *image_dirty;

	std::multiset<wxString> *ordered_names;
	std::multiset<wxString>::iterator current;

	std::promise<void> prom_cwd, prom_log, prom_img, prom_loop;
	std::thread watch_cwd, watch_log, watch_img, watch_loop;

	std::mutex mut_rcwd, mut_img, mut_current, mut_ord;

	std::atomic<unsigned long long> memory_used;
	std::atomic<unsigned int> current_index;

	template<typename F>
	void Watch(std::future<void> fObj, unsigned int milli, F callback);

	void LoopImg(std::future<void> fObj);
};

#endif /* AHIMAGEDB_H_ */
