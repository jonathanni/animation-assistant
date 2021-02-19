#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/sizer.h>
#include <wx/glcanvas.h>

#include "ahcanvas.h"

BEGIN_EVENT_TABLE(AHCanvas, wxGLCanvas) //
EVT_MOTION(AHCanvas::mouseMoved)
EVT_LEFT_DOWN(AHCanvas::mouseDown)
EVT_LEFT_UP(AHCanvas::mouseReleased)
EVT_RIGHT_DOWN(AHCanvas::rightClick)
EVT_LEAVE_WINDOW(AHCanvas::mouseLeftWindow)
EVT_SIZE(AHCanvas::resized)
EVT_KEY_DOWN(AHCanvas::keyPressed)
EVT_KEY_UP(AHCanvas::keyReleased)
EVT_MOUSEWHEEL(AHCanvas::mouseWheelMoved)
EVT_PAINT(AHCanvas::render)
END_EVENT_TABLE()

void AHCanvas::mouseMoved(wxMouseEvent &event) {
}
void AHCanvas::mouseDown(wxMouseEvent &event) {
}
void AHCanvas::mouseWheelMoved(wxMouseEvent &event) {
}
void AHCanvas::mouseReleased(wxMouseEvent &event) {
}
void AHCanvas::rightClick(wxMouseEvent &event) {
}
void AHCanvas::mouseLeftWindow(wxMouseEvent &event) {
}
void AHCanvas::keyPressed(wxKeyEvent &event) {
}
void AHCanvas::keyReleased(wxKeyEvent &event) {
}

AHCanvas::AHCanvas(wxFrame *parent, int *args) :
		wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize,
		wxFULL_REPAINT_ON_RESIZE), m_context(new wxGLContext(this)), texID(0), current_img(
				nullptr), tex_gen(false) {
	// To avoid flashing on MSW
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

AHCanvas::~AHCanvas() {
	delete m_context;
}

void AHCanvas::resized(wxSizeEvent &evt) {
	Refresh();
}

/** Inits the OpenGL viewport for drawing in 2D. */
void AHCanvas::prepare2DViewport(int topleft_x, int topleft_y,
		int bottomright_x, int bottomright_y) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
	// textures
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glViewport(topleft_x, topleft_y, bottomright_x - topleft_x,
			bottomright_y - topleft_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(topleft_x, bottomright_x, bottomright_y, topleft_y);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int AHCanvas::getWidth() {
	return GetSize().x;
}

int AHCanvas::getHeight() {
	return GetSize().y;
}

bool AHCanvas::setTexture(wxImage *image) {
	if (image == nullptr || (current_img != nullptr && image == current_img))
		return false;

	std::unique_lock<std::mutex> lock(mut_img);

	current_img = image;
	tex_gen.store(true);

	return true;
}

void AHCanvas::render(wxPaintEvent &evt) {
	if (!IsShown())
		return;

	wxGLCanvas::SetCurrent(*m_context);
	wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	prepare2DViewport(0, 0, getWidth(), getHeight());

	if (init) {
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		init = false;
	}

	std::unique_lock<std::mutex> lock(mut_img);

	if (current_img == nullptr)
		return;

	if (tex_gen.load()) {
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, current_img->GetWidth(),
				current_img->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,
				current_img->GetData());
		glBindTexture(GL_TEXTURE_2D, 0);

		tex_gen.store(false);
	}

	glBindTexture(GL_TEXTURE_2D, texID);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, 0);
		glVertex2i(0, 0); // topleft
		glTexCoord2f(1, 0);
		glVertex2i(current_img->GetWidth(), 0); // topright
		glTexCoord2f(1, 1);
		glVertex2i(current_img->GetWidth(), current_img->GetHeight()); // bottomright
		glTexCoord2f(0, 1);
		glVertex2i(0, current_img->GetHeight()); // bottomleft
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFlush();
	SwapBuffers();
}
