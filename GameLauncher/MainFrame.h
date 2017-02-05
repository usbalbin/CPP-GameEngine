#pragma once

#include "wx\wx.h"
#include "wx\odcombo.h"
#include "Options.hpp"
class MainFrame : public wxFrame
{
public:
	MainFrame();
	wxButton* Start;
	wxCheckBox* Fullscreen;
	wxTextCtrl* Nickname;
	wxTextCtrl* Servername;
	wxTextCtrl* Serverport;
	wxOwnerDrawnComboBox* Resolution;
	wxArrayString ResolutionArray;
	int i;
	Options options;
	void startButton(wxCommandEvent& WXUNUSED(event));
	DECLARE_EVENT_TABLE()
};