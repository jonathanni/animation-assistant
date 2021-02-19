/*
 * AHFrame.h
 *
 *  Created on: Jul 21, 2019
 *      Author: Jonathan Ni
 */

#ifndef AHFRAME_H_
#define AHFRAME_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dir.h>
#include <wx/dnd.h>

#include <atomic>
#include <mutex>

class AHCanvas;
class AHImageDB;
class AHLogWindow;

class AHFrame: public wxFrame, public wxFileDropTarget {
public:
	std::vector<wxString> *files;
	AHLogWindow *log_window;
	AHCanvas *canvas;

	wxTextCtrl *index_text, *fps_text;
	wxCheckBox *loop;
	wxButton *play;

	std::atomic<unsigned int> fps;

	AHFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

	void OnPrev(wxCommandEvent &event);
	void OnNext(wxCommandEvent &event);
	void OnPlayPause(wxCommandEvent &event);

	void PlayPause();

	bool IsPlay();

	void RefreshIndex();
private:
	AHImageDB *db;

	std::atomic<unsigned int> is_play;

	std::mutex mut_play;

	void OnOpen(wxCommandEvent &event);
	void OnExit(wxCommandEvent &event);
	void OnDebug(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);

	virtual bool OnDropFiles(wxCoord x, wxCoord y,
			const wxArrayString &filenames);

	void OpenFolder(wxString path);

wxDECLARE_EVENT_TABLE();
};

#endif /* AHFRAME_H_ */
