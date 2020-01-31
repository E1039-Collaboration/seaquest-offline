#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TEveManager.h>
#include <TEveBrowser.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <evt_filter/EvtFilter.h>
#include "PHEventDisplay.h"
#include "EventDispUI.h"
using namespace std;

EventDispUI::EventDispUI(const bool auto_mode)
  : m_run(0)
  , m_n_evt(0)
  , m_i_evt(0)
  , m_fr_main(0)
  , m_fr_menu(0)
  , m_fr_evt_nav(0)
  , m_lbl_run(0)
  , m_lbl_n_evt(0)
  , m_ne_evt_id(0)
  , m_ne_trig(0)
  , m_auto_mode(auto_mode)
  , m_tid1(0)
{
  ;
}

std::string EventDispUI::GetDstPath(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "/data2/e1039/onlmon/evt_disp/run_" << setw(6) << run << "_evt_disp.root";
  return oss.str();
}

bool EventDispUI::FindNewRuns()
{
  int nn = m_list_run.size();
  int run = nn > 0  ? m_list_run[nn-1] : RUN_MIN;
  while (++run < RUN_MAX) {
    string fname = GetDstPath(run);
    if (! gSystem->AccessPathName(fname.c_str())) { // if exists
      m_list_run.push_back(run);
    }
  }
  return m_list_run.size() > nn;
}

int EventDispUI::FetchNumEvents(const int run)
{
  int ret = -1;
  TFile* file = new TFile(GetDstPath(run).c_str());
  if (file->IsOpen()) {
    TTree* tree = (TTree*)file->Get("T");
    if (tree) ret = tree->GetEntries();
  }
  delete file;
  return ret;
}

int EventDispUI::OpenRunFile(const int run)
{
  cout << "EventDispUI::OpenRunFile(): run = " << run << endl;
  m_run = run;
  m_n_evt = FetchNumEvents(run);
  m_i_evt = 0;
  UpdateLabels();
  Fun4AllInputManager *in = Fun4AllServer::instance()->getInputManager("DSTIN");
  return in->fileopen(GetDstPath(run));
}

void EventDispUI::NextEvent()
{
  if (m_auto_mode) {
    cout << "NextEvent(): Ignored in auto mode." << endl;
    return;
  }
  if (m_i_evt >= m_n_evt) {
    cout << "NextEvent(): No next event." << endl;
    return;
  }
  Fun4AllServer* se = Fun4AllServer::instance();
  m_i_evt++;
  cout << "Next: " << m_i_evt << " / " << m_n_evt << endl;
  UpdateLabels();
  se->run(1, true);
}

void EventDispUI::PrevEvent()
{
  if (m_auto_mode) {
    cout << "PrevEvent(): Ignored in auto mode." << endl;
    return;
  }
  if (m_i_evt < 2) {
    cout << "PrevEvent(): No previous events." << endl;
    return;
  }
  int i_new = m_i_evt - 1;
  cout << "Prev: " << i_new << " / " << m_n_evt << endl;
  if (OpenRunFile(m_run) != 0) {
    cout << "PrevEvent(): Cannot reopen DST." << endl;
    return;
  }
  m_i_evt = i_new;
  UpdateLabels();
  Fun4AllServer* se = Fun4AllServer::instance();
  if (m_i_evt > 1) se->skip(m_i_evt - 1); // First move to the previous-to-previous event.
  se->run(1, true); // Then read the previous event.
}

void EventDispUI::MoveEvent(const int i_evt)
{
  if (i_evt > m_n_evt) {
    OpenRunFile(m_run);
    if (i_evt > m_n_evt) {
      cout << "Unexpected!!" << endl;
      return;
    }
  }
  m_i_evt = i_evt;
  UpdateLabels();
  Fun4AllServer* se = Fun4AllServer::instance();
  if (m_i_evt > 1) se->skip(m_i_evt - 1); // First move to the previous-to-previous event.
  se->run(1, true); // Then read the previous event.
}

void EventDispUI::ReqEvtID()
{
  int num = (int)m_ne_evt_id->GetNumberEntry()->GetIntNumber();
  cout << "ReqEvtID: " << num << endl;
  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
  filter->set_event_id_req(num);
}

void EventDispUI::ReqTrig()
{
  int num = (int)m_ne_trig->GetNumberEntry()->GetIntNumber();
  cout << "ReqTrig: " << num << endl;
  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
  filter->set_trigger_req(num);
}

void EventDispUI::ViewTop()
{
  cout << "Top View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_top();
}

void EventDispUI::ViewSide()
{
  cout << "Side View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_side();
}

void EventDispUI::View3D()
{
  cout << "3D View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_3d();
}

void EventDispUI::UpdateLabels()
{
  ostringstream oss;
  oss << "Run " << m_run;
  m_lbl_run->SetText(oss.str().c_str());
  oss.str("");
  oss << "Event " << m_i_evt << " / " << m_n_evt;
  m_lbl_n_evt->SetText(oss.str().c_str());
}

void EventDispUI::SetAutoMode(bool value)
{
  m_auto_mode = value;
  if (value) m_fr_menu->HideFrame(m_fr_evt_nav);
  else       m_fr_menu->ShowFrame(m_fr_evt_nav);
  m_fr_menu->Resize(); // (m_fr_menu->GetDefaultSize());
  m_fr_menu->MapWindow();
}

void EventDispUI::Run()
{
  BuildInterface();

  if (FindNewRuns()) {
    int run = m_list_run.back();
    OpenRunFile(run);
    MoveEvent(1); // Need process one event here, before calling "FuncNewEventCheck"
  } else {
    cout << "EventDispUI::Run(): Found no run.  Probably fail." << endl;
  }

  pthread_create(&m_tid1, NULL, FuncNewEventCheck, this);
}

void EventDispUI::BuildInterface()
{
  TEveBrowser* browser = gEve->GetBrowser();
  browser->StartEmbedding(TRootBrowser::kLeft);

  m_fr_main = new TGMainFrame(gClient->GetRoot(), 1000, 600);
  m_fr_main->SetWMPosition(0, 0); // Often ignored by the window manager
  m_fr_main->SetWindowName("Event Display");
  m_fr_main->SetCleanup(kDeepCleanup);

  m_fr_menu = new TGVerticalFrame(m_fr_main);

  TGLabel* lab = 0;
  lab = new TGLabel(m_fr_menu, "- - - Event Info - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

  m_lbl_run = new TGLabel(m_fr_menu, "Run ??????");
  m_fr_menu->AddFrame(m_lbl_run, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

  m_lbl_n_evt = new TGLabel(m_fr_menu, "Event ?? / ??");
  m_fr_menu->AddFrame(m_lbl_n_evt, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

  lab = new TGLabel(m_fr_menu, "- - - Event Navigation - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,10,2));

  TGCheckButton* check = new TGCheckButton(m_fr_menu, new TGHotString("Auto mode"), 99);
  check->SetToolTipText("When checked, the last sampled event is automatically shown.");
  check->SetState(m_auto_mode ? kButtonDown : kButtonUp);
  check->Connect("Toggled(Bool_t)", "EventDispUI", this, "SetAutoMode(Bool_t)");
  m_fr_menu->AddFrame(check, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
  
  TGTextButton* butt;
  { // m_fr_evt_nav
    m_fr_evt_nav = new TGCompositeFrame(m_fr_menu);

    { // frm1
      TGHorizontalFrame* frm1 = 0;
      frm1 = new TGHorizontalFrame(m_fr_evt_nav);
      lab = new TGLabel(frm1, "Event ID");
      frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
      m_ne_evt_id = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
                                      TGNumberFormat::kNEAAnyNumber,
                                      TGNumberFormat::kNELLimitMinMax,
                                      -999999, 999999);
      m_ne_evt_id->Connect("ValueSet(Long_t)", "EventDispUI", this, "ReqEvtID()");
      frm1->AddFrame(m_ne_evt_id, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
      m_fr_evt_nav->AddFrame(frm1);
      
      frm1 = new TGHorizontalFrame(m_fr_evt_nav);
      lab = new TGLabel(frm1, "Trigger");
      frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
      m_ne_trig = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber,
                                    TGNumberFormat::kNELLimitMinMax,
                                    -999, 999);
      m_ne_trig->Connect("ValueSet(Long_t)", "EventDispUI", this, "ReqTrig()");
      frm1->AddFrame(m_ne_trig, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
      m_fr_evt_nav->AddFrame(frm1);
    }
    
    butt = new TGTextButton(m_fr_evt_nav, "Next Event");
    m_fr_evt_nav->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    butt->Connect("Clicked()", "EventDispUI", this, "NextEvent()");
    
    butt = new TGTextButton(m_fr_evt_nav, "Previous Event");
    m_fr_evt_nav->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    butt->Connect("Clicked()", "EventDispUI", this, "PrevEvent()");

    m_fr_menu->AddFrame(m_fr_evt_nav);
  }

  lab = new TGLabel(m_fr_menu, "- - - View Navigation - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,10,2));
  
  butt = new TGTextButton(m_fr_menu, "  Top View  ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "ViewTop()");
  
  butt = new TGTextButton(m_fr_menu, " Side View ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "ViewSide()");

  butt = new TGTextButton(m_fr_menu, "   3D View   ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "View3D()");

  TGTextButton* fExit = new TGTextButton(m_fr_menu, "Exit","gApplication->Terminate(0)");
  m_fr_menu->AddFrame(fExit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,2,10,2));

  m_fr_main->AddFrame(m_fr_menu);

  m_fr_main->MapSubwindows();
  m_fr_main->Resize();
  m_fr_main->MapWindow();

  browser->StopEmbedding();
  browser->SetTabTitle("Event Control", 0);

  SetAutoMode(m_auto_mode);
}

void* EventDispUI::FuncNewEventCheck(void* arg)
{
  EventDispUI* ui = (EventDispUI*)arg;
  ui->ExecNewEventCheck();
}

void EventDispUI::ExecNewEventCheck()
{
  while (true) {
    if (m_auto_mode) {
      cout << "AutoMode()" << endl;
      if (FindNewRuns()) {
        cout << "  New run." << endl;
        OpenRunFile(m_list_run.back());
      }
      int n_now = FetchNumEvents(m_run);
      if (n_now > m_i_evt) {
        cout << "  New event." << endl;
        MoveEvent(n_now);
      }
    }
    sleep(15);
  }
}
