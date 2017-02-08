#pragma once

#include "wx\wx.h"
#include "wx\odcombo.h"
#include "wx\docview.h"
#include "Options.hpp"
#include "wx\docmdi.h"
#include "wx\cmdline.h"
#include "wx\config.h"

class MainFrame : public wxFrame
{
public:
	MainFrame();
	~MainFrame();
	wxButton* Start;
	wxButton* filbladdrarknapp;
	wxCheckBox* Fullscreen;
	wxCheckBox* Forcecpu;
	//wxCheckBox* Multiplayer;
	wxTextCtrl* Nickname;
	wxTextCtrl* Servername;
	wxTextCtrl* Serverport;
	wxOwnerDrawnComboBox* Resolution;
	wxArrayString ResolutionArray;
	wxFileDialog *filbladdare;
	wxTextCtrl* scenariostring;
	wxStaticBitmap* backGround;
	std::string ImagePath = "../Game/content/bg.png";
	wxBoxSizer* bSizer1;
	int i;
	int width;
	int height;
	Options options;
	void startButton(wxCommandEvent& WXUNUSED(event));
	void filfunc(wxCommandEvent& WXUNUSED(event));
	DECLARE_EVENT_TABLE()
	std::pair<std::string, std::string> split(std::string str, char token);
};