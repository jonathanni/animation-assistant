//============================================================================
// Name        : AnimationHelper.cpp
// Author      : Jonathan Ni
// Version     :
// Copyright   : Copyright 2019
// Description : Hello World in C, Ansi-style
//============================================================================

#include <wx/gbsizer.h>
#include <wx/dirdlg.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/image.h>

#include <sstream>

#include "main.h"
#include "ahimagedb.h"

wxBEGIN_EVENT_TABLE(AHFrame, wxFrame) //
EVT_MENU(wxID_OPEN, AHFrame::OnOpen)
EVT_MENU(wxID_EXIT, AHFrame::OnExit)
EVT_MENU(wxID_ABOUT, AHFrame::OnAbout)
EVT_MENU(wxID_PREFERENCES, AHFrame::OnDebug)
wxEND_EVENT_TABLE()

bool AnimationHelper::OnInit() {
	wxInitAllImageHandlers();

	frame = new AHFrame(wxT("Animation Helper v0.01a"), wxPoint(100, 100),
			wxSize(1600, 900));
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel *controls = new wxPanel(frame);
	wxBoxSizer *controls_sizer = new wxBoxSizer(wxVERTICAL);

	controls->SetSizer(controls_sizer);

	wxPanel *ctrl_images = new wxPanel(controls), *ctrl_playback = new wxPanel(
			controls);
	wxBoxSizer *ctrl_images_sizer = new wxBoxSizer(wxHORIZONTAL),
			*ctrl_playback_sizer = new wxBoxSizer(wxHORIZONTAL);

	ctrl_images->SetSizer(ctrl_images_sizer);
	ctrl_playback->SetSizer(ctrl_playback_sizer);

	wxButton *left = new wxButton(ctrl_images, wxID_ANY, wxT("<"));
	wxButton *right = new wxButton(ctrl_images, wxID_ANY, wxT(">"));
	wxButton *up = new wxButton(ctrl_playback, wxID_ANY, wxT("\u25b2"));
	wxButton *down = new wxButton(ctrl_playback, wxID_ANY, wxT("\u25bc"));

	frame->play = new wxButton(ctrl_playback, wxID_ANY, wxT("\u25b6"));

	frame->loop = new wxCheckBox(ctrl_playback, wxID_ANY, wxT("Loop"));
	frame->loop->SetValue(true);

	frame->index_text = new wxTextCtrl(ctrl_images, wxID_ANY, wxT(""),
			wxDefaultPosition, wxSize(640, 26), wxTE_CENTRE);
	frame->fps_text = new wxTextCtrl(ctrl_playback, wxID_ANY,
			std::to_string(frame->fps), wxDefaultPosition, wxDefaultSize,
			wxTE_CENTRE | wxTE_READONLY);

	wxFont big_font = wxFont();
	big_font.SetPointSize(14);
	big_font.SetWeight(wxFONTWEIGHT_HEAVY);

	left->SetFont(big_font);
	right->SetFont(big_font);
	up->SetFont(big_font);
	down->SetFont(big_font);
	frame->play->SetFont(big_font);
	frame->index_text->SetFont(big_font);
	frame->fps_text->SetFont(big_font);

	up->Bind(wxEVT_BUTTON, &AnimationHelper::IncFPS, this);
	down->Bind(wxEVT_BUTTON, &AnimationHelper::DecFPS, this);
	left->Bind(wxEVT_BUTTON, &AHFrame::OnPrev, frame);
	right->Bind(wxEVT_BUTTON, &AHFrame::OnNext, frame);
	frame->play->Bind(wxEVT_BUTTON, &AHFrame::OnPlayPause, frame);

	ctrl_images_sizer->Add(left, 0, wxEXPAND);
	ctrl_images_sizer->Add(frame->index_text, 1, wxEXPAND);
	ctrl_images_sizer->Add(right, 0, wxEXPAND);

	ctrl_playback_sizer->Add(frame->play, 0, wxEXPAND);
	ctrl_playback_sizer->AddSpacer(4);
	ctrl_playback_sizer->Add(frame->loop, 0, wxEXPAND);
	ctrl_playback_sizer->Add(up, 0, wxEXPAND);
	ctrl_playback_sizer->Add(frame->fps_text, 1, wxEXPAND);
	ctrl_playback_sizer->Add(down, 0, wxEXPAND);

	controls_sizer->Add(ctrl_images, 0, wxALIGN_CENTRE_HORIZONTAL);
	controls_sizer->AddSpacer(4);
	controls_sizer->Add(ctrl_playback, 0, wxALIGN_CENTRE_HORIZONTAL);

	main_sizer->Add(frame->canvas, 1, wxEXPAND);
	main_sizer->AddSpacer(4);
	main_sizer->Add(controls, 0, wxALIGN_CENTRE_HORIZONTAL);
	main_sizer->AddSpacer(4);

	frame->SetSizer(main_sizer);
	frame->SetAutoLayout(true);
	frame->Show(true);

	return true;
}

void AnimationHelper::IncFPS(wxCommandEvent&WXUNUSED(event)) {
	if (frame->fps == (unsigned int) -1)
		return;

	++(frame->fps);
	frame->fps_text->SetValue(std::to_string(frame->fps));
}

void AnimationHelper::DecFPS(wxCommandEvent&WXUNUSED(event)) {
	if (frame->fps == 1)
		return;

	--(frame->fps);
	frame->fps_text->SetValue(std::to_string(frame->fps));
}

IMPLEMENT_APP(AnimationHelper);
