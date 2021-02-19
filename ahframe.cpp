/*
 * ahframe.cpp
 *
 *  Created on: Jul 22, 2019
 *      Author: Jonathan Ni
 */

#include <sstream>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/log.h>

#include "main.h"
#include "ahcanvas.h"
#include "ahframe.h"
#include "ahimagedb.h"
#include "ahlogwindow.h"

AHFrame::AHFrame(const wxString &title, const wxPoint &pos, const wxSize &size) :
		wxFrame(nullptr, wxID_ANY, title, pos, size), log_window(
				new AHLogWindow), index_text(nullptr), fps_text(nullptr), loop(
				nullptr), play(nullptr), fps(14), is_play(0) {
	db = nullptr;
	files = new std::vector<wxString>;

	int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
	canvas = new AHCanvas(this, args);

	wxMenu *menu_file = new wxMenu;
	wxMenu *menu_help = new wxMenu;

	menu_file->Append(wxID_OPEN);
	menu_file->Append(wxID_EXIT);
	menu_help->Append(wxID_ABOUT);
	menu_help->Append(wxID_PREFERENCES);

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(menu_file, "&File");
	menu_bar->Append(menu_help, "&Help");

	SetDropTarget(this);
	SetMenuBar(menu_bar);
	CreateStatusBar();
	SetStatusText("Status:");
}

void AHFrame::RefreshIndex() {
	index_text->SetValue(wxFileName(db->GetCurrent()).GetName());
}

bool AHFrame::OnDropFiles(wxCoord x, wxCoord y,
		const wxArrayString &filenames) {
	if (filenames.Count() != 1) {
		wxLogError
		("Application opens one folder only");
		return false;
	}

	if (!wxDirExists(filenames[0])) {
		wxLogError
		("Application opens folders only");
		return false;
	}

	OpenFolder(filenames[0]);
	return true;
}

void AHFrame::OpenFolder(wxString path) {
	{
		if (db != nullptr)
			std::unique_lock<std::mutex> lock(db->mut_cwd);
		wxSetWorkingDirectory(path);
	}

	if (db == nullptr)
		db = new AHImageDB(this);
	else
		db->RefreshCwd();
}

bool AHFrame::IsPlay() {
	return is_play.load() == 1;
}

void AHFrame::OnPrev(wxCommandEvent&WXUNUSED(event)) {
	db->PrevImage();
	RefreshIndex();
}

void AHFrame::OnNext(wxCommandEvent&WXUNUSED(event)) {
	db->NextImage();
	RefreshIndex();
}

void AHFrame::OnPlayPause(wxCommandEvent&WXUNUSED(event)) {
	PlayPause();
}

void AHFrame::PlayPause() {
	is_play ^= 1;

	std::unique_lock<std::mutex> lock(mut_play);

	if (is_play.load() == 1)
		play->SetLabel(wxT("\u275a\u275a"));
	else
		play->SetLabel(wxT("\u25b6"));
}

void AHFrame::OnOpen(wxCommandEvent&WXUNUSED(event)) {
	wxDirDialog open_dir_dialog(nullptr, wxT("Select directory..."),
			wxEmptyString,
			wxDD_DIR_MUST_EXIST);

	if (open_dir_dialog.ShowModal() == wxID_CANCEL)
		return;

	OpenFolder(open_dir_dialog.GetPath());
}

void AHFrame::OnDebug(wxCommandEvent&WXUNUSED(event)) {
	log_window->Show(true);
}

void AHFrame::OnExit(wxCommandEvent&WXUNUSED(event)) {
	this->Close(true);
}

void AHFrame::OnAbout(wxCommandEvent&WXUNUSED(event)) {
	wxMessageBox("Created in 2019 to help preview animations.",
			"About Animation Helper", wxOK | wxICON_INFORMATION);
}
