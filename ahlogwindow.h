/*
 * ahlogwindow.h
 *
 *  Created on: Jul 24, 2019
 *      Author: Jonathan Ni
 */

#ifndef AHLOGWINDOW_H_
#define AHLOGWINDOW_H_

#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/event.h>

class AHLogWindow : public wxFrame {
public:
	AHLogWindow();
	void SetText(std::string text);
	void Clear();
private:
	wxTextCtrl *box;

	void OnClose(wxCloseEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif /* AHLOGWINDOW_H_ */
