/*
 * AHCanvas.h
 *
 *  Created on: Jul 21, 2019
 *      Author: Jonathan Ni
 */

#ifndef AHCANVAS_H_
#define AHCANVAS_H_

// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include <GL/glext.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/glcanvas.h>

#include <mutex>
#include <atomic>

class AHCanvas: public wxGLCanvas {
	wxGLContext *m_context;

public:
	AHCanvas(wxFrame *parent, int *args);
	virtual ~AHCanvas();

	void resized(wxSizeEvent &evt);

	int getWidth();
	int getHeight();

	void render(wxPaintEvent &evt);
	void prepare2DViewport(int topleft_x, int topleft_y, int bottomright_x,
			int bottomright_y);
	bool setTexture(wxImage *image);

	// events
	void mouseMoved(wxMouseEvent &event);
	void mouseDown(wxMouseEvent &event);
	void mouseWheelMoved(wxMouseEvent &event);
	void mouseReleased(wxMouseEvent &event);
	void rightClick(wxMouseEvent &event);
	void mouseLeftWindow(wxMouseEvent &event);
	void keyPressed(wxKeyEvent &event);
	void keyReleased(wxKeyEvent &event);

DECLARE_EVENT_TABLE()

private:
	GLuint texID;

	wxImage *current_img;

	std::atomic<bool> tex_gen;

	std::mutex mut_img;

	bool init = true;
};

#endif /* AHCANVAS_H_ */
