#include "MainFrame.h"
#include "Utils.hpp"
#include <wx/filefn.h> 

MainFrame::MainFrame() :
	wxFrame(NULL, -1, "GAMELAUNCHER", wxDefaultPosition, wxSize(640, 480)),
	options("content/options.cfg"){
	
	wxImage::AddHandler( new wxPNGHandler);

	bSizer1 = new wxBoxSizer(wxVERTICAL);
	bSizer1->Add(0, 0, 1, wxEXPAND, 5);
	backGround = new wxStaticBitmap(this, wxID_ANY, wxBitmap(ImagePath, wxBITMAP_TYPE_PNG), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(backGround, 0, wxALIGN_CENTER, 5);
	bSizer1->Add(0, 0, 1, wxEXPAND, 5);
	this->SetSizer(bSizer1);
	this->Layout();
	this->GetSize(&width, &height);

	urlToBugReport = new wxHyperlinkCtrl(backGround, wxID_ANY, "FOUND ANY BUGS? CLICK HERE TO REPORT", "https://github.com/usbalbin/CPP-GameEngine/issues", wxPoint(width - 300, height - 150), wxDefaultSize);
	filbladdrarknapp = new wxButton(backGround, wxID_HIGHEST + 1, "choose scenario file", wxPoint(width - 250, 50), wxSize(200, 40));
	Start = new wxButton(backGround, wxID_HIGHEST + 2, "Save and Start", wxPoint(width - 218, height - 80), wxSize(200, 40));
	
	Fullscreen = new wxCheckBox(backGround, wxID_HIGHEST + 3, "Fullscreen", wxPoint(60, 100), wxDefaultSize);
	Fullscreen->SetValue((bool)options.fullScreen);
	
	Forcecpu = new wxCheckBox(backGround, wxID_HIGHEST + 4, "Force CPU Graphics", wxPoint(60, 80), wxDefaultSize);
	Forcecpu->SetValue((bool)options.forceCpu);

	//Multiplayer = new wxCheckBox(this, wxID_HIGHEST + 5, "Multiplayer", wxPoint(60, 80), wxDefaultSize);
	//Multiplayer->SetValue((bool)options.forceCpu);

	Nickname = new wxTextCtrl(backGround, wxID_ANY, options.playerName, wxPoint(60, 120), wxDefaultSize, wxTE_PROCESS_ENTER);
	Nickname->SetForegroundColour(*wxBLACK);
	Nickname->SetBackgroundColour(*wxWHITE);
	Nickname->SetHint("Nickname");

	Servername = new wxTextCtrl(backGround, wxID_ANY, options.serverAddress, wxPoint(60, 150), wxDefaultSize, wxTE_PROCESS_ENTER);
	Servername->SetForegroundColour(*wxBLACK);
	Servername->SetBackgroundColour(*wxWHITE);
	Servername->SetHint("Servername");

	Serverport = new wxTextCtrl(backGround, wxID_ANY,std::to_string(options.serverPort), wxPoint(60, 180), wxDefaultSize, wxTE_PROCESS_ENTER);
	Serverport->SetForegroundColour(*wxBLACK);
	Serverport->SetBackgroundColour(*wxWHITE);
	Serverport->SetHint("Serverport");

	ResolutionArray.Add("100x100");
	ResolutionArray.Add("200x200");
	ResolutionArray.Add("640x480");
	ResolutionArray.Add("1024x768");
	ResolutionArray.Add("1280x1024");
	ResolutionArray.Add("1920x1200");
	
	Resolution = new wxOwnerDrawnComboBox(backGround, wxID_ANY, std::to_string(options.width)+"x"+std::to_string(options.height),
		wxPoint(60, 55), wxSize(100, 20),
		ResolutionArray,
		(long)0 // wxCB_SORT // wxNO_BORDER | wxCB_READONLY
	);

	filbladdare = new wxFileDialog(backGround, "Choose Scenario","","","Scene File (*.scene)|*.scene", wxFD_OPEN | wxFD_FILE_MUST_EXIST);	
	scenariostring = new wxTextCtrl(backGround, wxID_ANY, "Please Select a scenario", wxPoint(width - 270, 30), wxSize(240, 20), wxTE_READONLY);


}

MainFrame::~MainFrame()
{
	delete Forcecpu;  // TO DO!
} 

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(wxID_HIGHEST + 1, MainFrame::filfunc)
EVT_BUTTON(wxID_HIGHEST + 2, MainFrame::startButton)
END_EVENT_TABLE()
class my_first_app : public wxApp
{
public:
	wxFrame* window;
	bool OnInit()
	{
		window = new MainFrame();

		window->Show();
		return true;
	}

};

void MainFrame::startButton(wxCommandEvent& WXUNUSED(event))
{
	std::string resres;
	resres = Resolution->GetValue();
	auto res = split(resres, 'x');
	options.width =  std::stoi(res.first);
	options.height = std::stoi(res.second);
	options.playerName = Nickname->GetValue();
	options.serverAddress = Servername->GetValue();
	options.serverPort = wxAtoi(Serverport->GetValue());
	options.forceCpu = Forcecpu->GetValue();
	options.fullScreen = Fullscreen->GetValue();
	options.save("content/options.cfg");
	
	this->Close();
	#ifdef _DEBUG
		system("start ../x64/Debug/Game");
	#else
		system("start ../x64/Release/Game");
	#endif
}

void MainFrame::filfunc(wxCommandEvent& WXUNUSED(event))
{
	if (filbladdare->ShowModal() == wxID_CANCEL)
		return;
	options.scenarioPath = absToRelPath(wxGetCwd().ToStdString(), filbladdare->GetPath().ToStdString());
	scenariostring->SetValue(filbladdare->GetPath());

}

std::pair<std::string, std::string> MainFrame::split(std::string str, char token) {
	std::pair<std::string, std::string> result;
	result.first = str.substr(0, str.find(token));
	result.second = str.substr(str.find(token)+1);
	return result;
}

IMPLEMENT_APP(my_first_app);
