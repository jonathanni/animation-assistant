/*
 * ahlogwindow.cpp
 *
 *  Created on: Jul 24, 2019
 *      Author: Jonathan Ni
 */

#include "ahlogwindow.h"

wxBEGIN_EVENT_TABLE(AHLogWindow, wxFrame) //
EVT_CLOSE(AHLogWindow::OnClose)
wxEND_EVENT_TABLE()

AHLogWindow::AHLogWindow() :
		wxFrame(nullptr, wxID_ANY, wxT("Debug Window"), wxPoint(100, 100),
				wxSize(600, 400)), box(
				new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
						wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY)) {
}

void AHLogWindow::SetText(std::string text) {
	box->SetValue(wxString(text));
}

void AHLogWindow::Clear() {
	box->Clear();
}

void AHLogWindow::OnClose(wxCloseEvent &event) {
	this->Show(false);
}
