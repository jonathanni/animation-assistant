/*
 * AnimationHelper.h
 *
 *  Created on: Jul 21, 2019
 *      Author: Jonathan Ni
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ahframe.h"
#include "ahcanvas.h"

class AnimationHelper: public wxApp {
public:
	virtual bool OnInit();
private:
	AHFrame *frame;
	AHCanvas *canvas;

	void IncFPS(wxCommandEvent &event);
	void DecFPS(wxCommandEvent &event);

	void PrevImage(wxCommandEvent &event);
	void NextImage(wxCommandEvent &event);
};

#endif /* MAIN_H_ */
