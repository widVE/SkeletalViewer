//------------------------------------------------------------------------------
// <copyright file="SkeletalViewer.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// This module provides sample code used to demonstrate Kinect NUI processing

// Note: 
//     Platform SDK lib path should be added before the VC lib
//     path, because uuid.lib in VC lib path may be older

#include "stdafx.h"
#include <strsafe.h>
#include "SkeletalViewer.h"
#include "PlayerVideoRecorder.h"
#include "resource.h"
#include <Guiddef.h>
#include <process.h>


//ROSS 12/3/2012 - integrating wireless EMG device..
#include "rs232.h"

//ROSS 9/6/2012 - ADDING FROM SPEECH
// This is the class ID we expect for the Microsoft Speech recognizer.
// Other values indicate that we're using a version of sapi.h that is
// incompatible with this sample.
DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);
//DEFINE_GUID(IID_ISpeechRecognizer, 2D5F1C0Ch, 0BD75h,4B08h,94h,78h,3Bh,11h,0FEh,0A2h,58h,6Ch);

/*EXTERN_C const CLSID CLSID_SpSharedRecognizer; 
#ifdef __cplusplus 
class DECLSPEC_UUID("3BEE4890-4FE9-4A37-8C1E-5E7E12791C1F") {};
#endif*/

#include "Colors.h"
// Global Variables:
CSkeletalViewerApp  g_skeletalViewerApp;  // Application class

void FionaLeap::onInit(const Leap::Controller& controller) {
  printf("Leap Initialized\n");
}

void FionaLeap::onConnect(const Leap::Controller& controller) {
   printf("Leap Connected\n");
/**
 * The Config class provides access to Leap Motion system configuration information.
 *
 * You can get and set gesture configuration parameters using the Config object
 * obtained from a connected Controller object. The key strings required to
 * identify a configuration parameter include:
 *
 * Key string | Value type | Default value | Units
 * -----------|------------|---------------|------
 * Gesture.Circle.MinRadius | float | 5.0 | mm
 * Gesture.Circle.MinArc | float | 1.5*pi | radians
 * Gesture.Swipe.MinLength | float | 150 | mm
 * Gesture.Swipe.MinVelocity | float | 1000 | mm/s
 * Gesture.KeyTap.MinDownVelocity | float | 50 | mm/s
 * Gesture.KeyTap.HistorySeconds | float | 0.1 | s
 * Gesture.KeyTap.MinDistance | float | 3.0 | mm
 * Gesture.ScreenTap.MinForwardVelocity  | float | 50 | mm/s
 * Gesture.ScreenTap.HistorySeconds | float | 0.1 | s
 * Gesture.ScreenTap.MinDistance | float | 5.0 | mm
**/
  controller.config().setInt32(std::string("Gesture.Circle.MinRadius"), 10);
  controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
  controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
}

void FionaLeap::onDisconnect(const Leap::Controller& controller) {
   printf("Leap Disconnected\n");
}

void FionaLeap::onExit(const Leap::Controller& controller) {
   printf("Leap Exiting\n");
}

void FionaLeap::onFrame(const Leap::Controller& controller) {
  // Get the most recent frame and report some basic information
  const Leap::Frame frame = controller.frame();

  if (!frame.hands().isEmpty()) 
  {
    // Get the first hand
    const Leap::Hand& hand = frame.hands()[0];
	if(frame.hands().count() > 1)
	{
		const Leap::Hand& hand2 = frame.hands()[1];
		Leap::Gesture::Type t = Leap::Gesture::TYPE_INVALID;
		Leap::Gesture::Type t2 = Leap::Gesture::TYPE_INVALID;
		Leap::Gesture::State s = Leap::Gesture::STATE_INVALID;
		Leap::Gesture::State s2 = Leap::Gesture::STATE_INVALID;

		UpdateLeapData *pUpdateLeap = new UpdateLeapData(BasePacket::UPDATE_LEAP);
		if(frame.gestures().count() > 1)
		{
			const Leap::Gesture& gesture = frame.gestures()[0];
			if(gesture.isValid())
			{
				t = gesture.type();
				s = gesture.state();
			}

			if(frame.gestures().count() > 2)
			{
				const Leap::Gesture& gesture = frame.gestures()[0];
				if(gesture.isValid())
				{
					t2 = gesture.type();
					s2 = gesture.state();
				}
			}
		}

		pUpdateLeap->SetPayload(&hand, &hand2, t, s, t2, s2);
		EnterCriticalSection(&CSkeletalViewerApp::cs);
		g_skeletalViewerApp.fionaPackets.push_back(pUpdateLeap);
		LeaveCriticalSection(&CSkeletalViewerApp::cs);
	}
	else
	{
		Leap::Gesture::Type t = Leap::Gesture::TYPE_INVALID;
		Leap::Gesture::Type t2 = Leap::Gesture::TYPE_INVALID;
		Leap::Gesture::State s = Leap::Gesture::STATE_INVALID;
		Leap::Gesture::State s2 = Leap::Gesture::STATE_INVALID;
		UpdateLeapData *pUpdateLeap = new UpdateLeapData(BasePacket::UPDATE_LEAP);
		if(frame.gestures().count() > 0)
		{
			const Leap::Gesture& gesture = frame.gestures()[0];
			if(gesture.isValid())
			{
				t = gesture.type();
				s = gesture.state();
			}
		}

		pUpdateLeap->SetPayload(&hand, 0, t, s, t2, s2);

		EnterCriticalSection(&CSkeletalViewerApp::cs);
		g_skeletalViewerApp.fionaPackets.push_back(pUpdateLeap);
		LeaveCriticalSection(&CSkeletalViewerApp::cs);
	}
  }
}

// configs:
#define USE_BEEPS_AND_DELAYS			// undefine to test library works without them
#define LOOK_FOR_ADDITIONAL_WIIMOTES	// tries to connect any extra wiimotes

//ROSS 9/14/2012 - ADDING WII-FIT BALANCE BOARD
// ------------------------------------------------------------------------------------
//  state-change callback example (we use polling for everything else):
// ------------------------------------------------------------------------------------
void on_state_change (wiimote			  &remote,
					  state_change_flags  changed,
					  const wiimote_state &new_state)
	{
	// we use this callback to set report types etc. to respond to key events
	//  (like the wiimote connecting or extensions (dis)connecting).
	
	// NOTE: don't access the public state from the 'remote' object here, as it will
	//		  be out-of-date (it's only updated via RefreshState() calls, and these
	//		  are reserved for the main application so it can be sure the values
	//		  stay consistent between calls).  Instead query 'new_state' only.

	// the wiimote just connected
	if(changed & CONNECTED)
		{
		// ask the wiimote to report everything (using the 'non-continous updates'
		//  default mode - updates will be frequent anyway due to the acceleration/IR
		//  values changing):

		// note1: you don't need to set a report type for Balance Boards - the
		//		   library does it automatically.
		
		// note2: for wiimotes, the report mode that includes the extension data
		//		   unfortunately only reports the 'BASIC' IR info (ie. no dot sizes),
		//		   so let's choose the best mode based on the extension status:
		if(new_state.ExtensionType != wiimote::BALANCE_BOARD)
			{
			if(new_state.bExtension)
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
			else
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);		//    IR dots
			}
		}
	// a MotionPlus was detected
	if(changed & MOTIONPLUS_DETECTED)
		{
		// enable it if there isn't a normal extension plugged into it
		// (MotionPlus devices don't report like normal extensions until
		//  enabled - and then, other extensions attached to it will no longer be
		//  reported (so disable the M+ when you want to access them again).
		if(remote.ExtensionType == wiimote_state::NONE) {
			bool res = remote.EnableMotionPlus();
			_ASSERT(res);
			}
		}
	// an extension is connected to the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_CONNECTED)
		{
		// We can't read it if the MotionPlus is currently enabled, so disable it:
		if(remote.MotionPlusEnabled())
			remote.DisableMotionPlus();
		}
	// an extension disconnected from the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
		{
		// enable the MotionPlus data again:
		if(remote.MotionPlusConnected())
			remote.EnableMotionPlus();
		}
	// another extension was just connected:
	else if(changed & EXTENSION_CONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(1000, 200);
#endif
		// switch to a report mode that includes the extension data (we will
		//  loose the IR dot sizes)
		// note: there is no need to set report types for a Balance Board.
		if(!remote.IsBalanceBoard())
			remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
		}
	// extension was just disconnected:
	else if(changed & EXTENSION_DISCONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(200, 300);
#endif
		// use a non-extension report mode (this gives us back the IR dot sizes)
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
		}
	}

//LPCWSTR CSkeletalViewerApp::GrammarFileName = L"SpeechBasics-D2D.grxml";
//LPCWSTR CSkeletalViewerApp::GrammarFileName = L"grammar_test.grxml";
//LPCWSTR CSkeletalViewerApp::GrammarFileName = L"nouns.grxml";
//LPCWSTR CSkeletalViewerApp::GrammarFileName = L"colors_test.grxml";
LPCWSTR CSkeletalViewerApp::GrammarFileName = L"colors_limited.grxml";
bool CSkeletalViewerApp::exiting = false;
bool CSkeletalViewerApp::fionaConnected = false;
LELSender CSkeletalViewerApp::sender;
CRITICAL_SECTION CSkeletalViewerApp::cs;
CaveMaster *CSkeletalViewerApp::fionaNetMaster = 0;
 std::vector<BasePacket*> CSkeletalViewerApp::fionaPackets;

//float CSkeletalViewerApp::senderValue=0.f;

void CSkeletalViewerApp::SendWithLELSender(void* x)
{
	//todo - combine skeleton and speech value into single structure..

	while(!exiting)
	{
		sender.SendKinectPacket();
		Sleep(1);
	}
}

void CSkeletalViewerApp::SendWithFiona(void* x)
{
	fionaNetMaster = new CaveMaster(1, 7568);
	fionaNetMaster->ListenForSlaves();	//blocks until machine connects..
	
	fionaConnected = true;

	while(!exiting)
	{
		//assemble packets we've gathered for sending and send (wii-fit, emg)
		_FionaUTSyncMasterSync();
		
		//wait for head node to know it's done process the packet..
		fionaNetMaster->Handshake();

		//send a bit back to head node to know it should continue..
		fionaNetMaster->WakeupSlaves();

		Sleep(1);
	}
}

void CSkeletalViewerApp::HandleEMGThread(void*x)
{
	static const unsigned int maxNumSize = 32;
	unsigned char buf[maxNumSize];
	unsigned int values[4];
	float fVoltage = 0.f;
	unsigned int bufSize = 0;
	unsigned char curChar = 0;
	int count = 0;

	while(!exiting)
	{
		memset(buf, 0, sizeof(unsigned char)*maxNumSize);	
		memset(values, 0, sizeof(unsigned int)*4);
		bufSize = 0;
		curChar = 0;
		count = 0;

		int read = PollComport(m_port, &curChar, 1);

		while(curChar != '\n')
		{
			if(curChar != 0)
			{
				if(curChar == '\t')
				{
					buf[bufSize]='\0';
					values[count] = atoi((const char*)buf);
					count++;
					bufSize = 0;
					memset(buf, 0, sizeof(unsigned char)*maxNumSize);
				}
				else if(curChar == 13)
				{
					buf[bufSize]='\0';
					fVoltage = (float)atof((const char*)buf);
					//values[count] = atoi((const char*)buf);
					//count++;
					bufSize = 0;
					memset(buf, 0, sizeof(unsigned char)*maxNumSize);
				}
				else
				{
					buf[bufSize] = curChar;
					bufSize++;
				}
			}

			read = PollComport(m_port, &curChar, 1);
			while(read != 1)
			{
				read = PollComport(m_port, &curChar, 1);
			}
		}

		values[0] = min(values[0], 1024);
		values[1] = min(values[1], 120);	//thresh*3 in the processing app but 40 is the value..
		
		EnterCriticalSection(&cs);
		UpdateEMGData *pEMG = new UpdateEMGData(BasePacket::UPDATE_EMG);
		pEMG->SetPayload(values, fVoltage);
		fionaPackets.push_back(pEMG);
		LeaveCriticalSection(&cs);
	}
}

//DONE ADDING


#define INSTANCE_MUTEX_NAME L"SkeletalViewerInstanceCheck"

HWND g_hWndApp;
/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    MSG       msg;
    WNDCLASS  wc;
        
    // Store the instance handle
    g_skeletalViewerApp.m_hInstance = hInstance;

    // Dialog custom window class
    ZeroMemory(&wc,sizeof(wc));
    wc.style=CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra=DLGWINDOWEXTRA;
    wc.hInstance=hInstance;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_SKELETALVIEWER));
    wc.lpfnWndProc=DefDlgProc;
    wc.lpszClassName=SZ_APPDLG_WINDOW_CLASS;
    if( !RegisterClass(&wc) )
    {
        return 0;
    }

	int numConnections = 1;

	if(strlen((const char*)lpCmdLine) > 0)
	{
		const char *sCmdLine = (const char *)lpCmdLine;
		int numChars = strlen(sCmdLine);
		bool parsingConnections = false;
		for(int i = 0; i < numChars; ++i)
		{
			std::string arg;
			if(sCmdLine[i] == ' ')
			{
				if(arg.length() > 0)
				{
					if(parsingConnections)
					{
						numConnections = atoi(arg.c_str());
					}

					if(strcmp(arg.c_str(), "numConnections")==0)
					{
						parsingConnections = true;
					}
				}

				arg.clear();
			}
			else
			{
				arg.push_back(sCmdLine[i]);
			}
		}
	}

    // Create main application window
    HWND hWndApp = CreateDialogParam(
        hInstance,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC) CSkeletalViewerApp::MessageRouter, 
        reinterpret_cast<LPARAM>(&g_skeletalViewerApp));

	g_hWndApp = hWndApp;

    // unique mutex, if it already exists there is already an instance of this app running
    // in that case we want to show the user an error dialog
    HANDLE hMutex = CreateMutex(NULL, FALSE, INSTANCE_MUTEX_NAME);
    if ( (hMutex != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS) ) 
    {
        //load the app title
        TCHAR szAppTitle[256] = { 0 };
        LoadStringW( hInstance, IDS_APPTITLE, szAppTitle, _countof(szAppTitle) );

        //load the error string
        TCHAR szRes[512] = { 0 };
        LoadStringW( hInstance, IDS_ERROR_APP_INSTANCE, szRes, _countof(szRes) );

        MessageBoxW( NULL, szRes, szAppTitle, MB_OK | MB_ICONHAND );

        CloseHandle(hMutex);
        return -1;
    }

    // Show window
    ShowWindow(hWndApp, nCmdShow);

	//ADDING FROM SPEECH APP Ross T 9/6/2012
	RECT rect;
	GetWindowRect(hWndApp, &rect);
	SetWindowPos(hWndApp, HWND_TOPMOST, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0);

	SetForegroundWindow(hWndApp);
	//SetFocus(hWndApp);
    const int eventCount = 1;
    HANDLE hEvents[eventCount];

	//END ADD

	//ADDING WII-FIT BALANCE BOARD INTEGRATION ROSS T 9/14/2012

	// in this demo we use a state-change callback to get notified of
	//  extension-related events, and polling for everything else
	// (note you don't have to use both, use whatever suits your app):
	g_skeletalViewerApp.remote.ChangedCallback		= on_state_change;
	//  notify us only when the wiimote connected sucessfully, or something
	//   related to extensions changes
	g_skeletalViewerApp.remote.CallbackTriggerFlags = (state_change_flags)(CONNECTED |
													   EXTENSION_CHANGED |
													   MOTIONPLUS_CHANGED);

    // Main message loop:
    while(GetMessage(&msg,NULL,0,0)) 
    {
		//ADDING FROM SPEECH APP - Ross T 9/6/2012
        hEvents[0] = g_skeletalViewerApp.m_hSpeechEvent;

        // Check to see if we have either a message (by passing in QS_ALLINPUT)
        // Or a speech event (hEvents)
        DWORD dwEvent = MsgWaitForMultipleObjectsEx(eventCount, hEvents, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

        // Check if this is an event we're waiting on and not a timeout or message
        if (WAIT_OBJECT_0 == dwEvent)
        {
			g_skeletalViewerApp.ProcessSpeech();
        }
		
		if(!g_skeletalViewerApp.remote.IsConnected())
		{
			bool bConnected = g_skeletalViewerApp.remote.Connect(wiimote::FIRST_AVAILABLE);
			if(bConnected)
			{
				g_skeletalViewerApp.remote.SetLEDs(0x0f);
			}

			if(bConnected)
			{
				SendDlgItemMessageW(hWndApp, IDC_WII, WM_SETTEXT, 0, (LPARAM)L"Wii Fit - Connected!");
			}
			else
			{
				SendDlgItemMessageW(hWndApp, IDC_WII, WM_SETTEXT, 0, (LPARAM)L"Wii Fit - Not Connected!");
			}
		}
		else
		{
			//if debugging values..
			/*wchar_t szMessage[512];
			memset(szMessage, 0, 512);

			swprintf(szMessage, L" TopLeft: %6.2f lb\n TopRight: %6.2f lb\n BottomLeft: %6.2f lb\n BottomRight: %6.2f lb\n", g_skeletalViewerApp.remote.BalanceBoard.Lb.TopL,
				g_skeletalViewerApp.remote.BalanceBoard.Lb.TopR, g_skeletalViewerApp.remote.BalanceBoard.Lb.BottomL, g_skeletalViewerApp.remote.BalanceBoard.Lb.BottomR);

			SendDlgItemMessageW(hWndApp, IDC_WII, WM_SETTEXT, 0, (LPARAM)szMessage);*/
		}
		//END ADD

        // If a dialog message will be taken care of by the dialog proc
        if ( (hWndApp != NULL) && IsDialogMessage(hWndApp, &msg) )
        {
            continue;
        }

        // otherwise do our window processing
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hMutex);
    return static_cast<int>(msg.wParam);
}


char CSkeletalViewerApp::masterBuffer[FIONA_NETWORK_BUFFER_SIZE];

/// <summary>
/// Constructor
/// </summary>
CSkeletalViewerApp::CSkeletalViewerApp(bool bUseFiona) : 
	m_hInstance(NULL),
    m_pKinectAudioStream(NULL),
    m_pSpeechStream(NULL),
    m_pSpeechRecognizer(NULL),
    m_pSpeechContext(NULL),
    m_pSpeechGrammar(NULL),
    m_hSpeechEvent(INVALID_HANDLE_VALUE),
	modelSearchOn(false),
	useFiona(true),			//these needs to be set to false when using python-based virtual lab stuff..
	useMicrophone(false),
	comportOpen(false),
	playerVideo(0),
	m_SendVideoFrames(true)	// disable this to save some net traffic 
{
    ZeroMemory(m_szAppTitle, sizeof(m_szAppTitle));
    LoadStringW(m_hInstance, IDS_APPTITLE, m_szAppTitle, _countof(m_szAppTitle));

    m_fUpdatingUi = false;
    Nui_Zero();

    // Init Direct2D
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	//size_t s = sizeof(_ACS_VideoHeader);
	//from Speech app:
	if(!useFiona)
	{
		sender.start();
		_beginthread(SendWithLELSender, 0, this);
	}
	else
	{
		InitializeCriticalSection(&cs);
		LeaveCriticalSection(&cs);
		_beginthread(SendWithFiona, 0, this);

		//start up the leap!
		m_leapController.addListener(m_leap);
	}


	playerVideo = new PlayerVideo();

	memset(masterBuffer, 0, FIONA_NETWORK_BUFFER_SIZE);
}

/// <summary>
/// Destructor
/// </summary>
CSkeletalViewerApp::~CSkeletalViewerApp()
{
	if(comportOpen)
	{
		CloseComport(m_port);
	}

	if(useFiona)
	{
		m_leapController.removeListener(m_leap);
	}

	//from speech app..
	exiting=true;
	Sleep(10);

	//disconnect wii fit
	remote.Disconnect();

	CoUninitialize();

    // Clean up Direct2D
    SafeRelease(m_pD2DFactory);

    Nui_Zero();
    SysFreeString(m_instanceId);

    SafeRelease(m_pKinectAudioStream);
    SafeRelease(m_pSpeechStream);
    SafeRelease(m_pSpeechRecognizer);
    SafeRelease(m_pSpeechContext);
    //SafeRelease(m_pSpeechGrammar);	//causing crash on exit for whatever reason...
}

/// <summary>
/// Clears the combo box for selecting active Kinect
/// </summary>
void CSkeletalViewerApp::ClearKinectComboBox()
{
    for (long i = 0; i < SendDlgItemMessage(m_hWnd, IDC_CAMERAS, CB_GETCOUNT, 0, 0); ++i)
    {
        SysFreeString( reinterpret_cast<BSTR>( SendDlgItemMessage(m_hWnd, IDC_CAMERAS, CB_GETITEMDATA, i, 0) ) );
    }
    SendDlgItemMessageW(m_hWnd, IDC_CAMERAS, CB_RESETCONTENT, 0, 0);
}

/// <summary>
/// Updates the combo box that lists Kinects available
/// </summary>
void CSkeletalViewerApp::UpdateKinectComboBox()
{
    m_fUpdatingUi = true;
    ClearKinectComboBox();

    int numDevices = 0;
    HRESULT hr = NuiGetSensorCount(&numDevices);

    if ( FAILED(hr) )
    {
        return;
    }

    long selectedIndex = 0;
    for (int i = 0; i < numDevices; ++i)
    {
        INuiSensor *pNui = NULL;
        HRESULT hr = NuiCreateSensorByIndex(i,  &pNui);
        if (SUCCEEDED(hr))
        {
            HRESULT status = pNui ? pNui->NuiStatus() : E_NUI_NOTCONNECTED;
            if (status == E_NUI_NOTCONNECTED)
            {
                pNui->Release();
                continue;
            }
            
            WCHAR kinectName[MAX_PATH];
            StringCchPrintfW(kinectName, _countof(kinectName), L"Kinect %d", i);
            long index = static_cast<long>( SendDlgItemMessage(m_hWnd, IDC_CAMERAS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kinectName)) );
            SendDlgItemMessageW( m_hWnd, IDC_CAMERAS, CB_SETITEMDATA, index, reinterpret_cast<LPARAM>(pNui->NuiUniqueId()) );
            if (m_pNuiSensor && pNui == m_pNuiSensor)
            {
                selectedIndex = index;
            }
            pNui->Release();
        }
    }

    SendDlgItemMessageW(m_hWnd, IDC_CAMERAS, CB_SETCURSEL, selectedIndex, 0);
    m_fUpdatingUi = false;
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSkeletalViewerApp::MessageRouter( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CSkeletalViewerApp *pThis = NULL;

    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CSkeletalViewerApp*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        NuiSetDeviceStatusCallback( &CSkeletalViewerApp::Nui_StatusProcThunk, pThis );
		g_skeletalViewerApp.comportOpen = false;
		if(g_skeletalViewerApp.useFiona)
		{
			int err = OpenComport(g_skeletalViewerApp.m_port, 57600);
			g_skeletalViewerApp.comportOpen = (bool)(err == 0);
			if(!g_skeletalViewerApp.comportOpen)
			{
				printf("Error opening comport %d:\n", g_skeletalViewerApp.m_port);
			}
			else
			{
				//begin a thread that continuously polls the EMG.
				::_beginthread(&HandleEMGThread, 0, 0);
			}
		}
    }
    else
    {
        pThis = reinterpret_cast<CSkeletalViewerApp*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (NULL != pThis)
    {
        return pThis->WndProc( hwnd, uMsg, wParam, lParam );
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSkeletalViewerApp::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch(message)
    {
        case WM_INITDIALOG:
        {
            // Clean state the class
            Nui_Zero();

            // Bind application window handle
            m_hWnd = hWnd;

            // Set the font for Frames Per Second display
            LOGFONT lf;
            GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
            lf.lfHeight *= 4;
            m_hFontFPS = CreateFontIndirect(&lf);
            SendDlgItemMessageW(hWnd, IDC_FPS, WM_SETFONT, (WPARAM)m_hFontFPS, 0);

            UpdateKinectComboBox();
            SendDlgItemMessageW(m_hWnd, IDC_CAMERAS, CB_SETCURSEL, 0, 0);

            TCHAR szComboText[512] = { 0 };

            // Fill combo box options for tracked skeletons

            LoadStringW(m_hInstance, IDS_TRACKEDSKELETONS_DEFAULT, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_TRACKEDSKELETONS_NEAREST1, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_TRACKEDSKELETONS_NEAREST2, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_TRACKEDSKELETONS_STICKY1, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_TRACKEDSKELETONS_STICKY2, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_SETCURSEL, 0, 0);
            // Fill combo box options for tracking mode

            LoadStringW(m_hInstance, IDS_TRACKINGMODE_DEFAULT, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKINGMODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_TRACKINGMODE_SEATED, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKINGMODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_TRACKINGMODE, CB_SETCURSEL, 0, 0);

            // Fill combo box options for range

            LoadStringW(m_hInstance, IDS_RANGE_DEFAULT, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_RANGE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            LoadStringW(m_hInstance, IDS_RANGE_NEAR, szComboText, _countof(szComboText));
            SendDlgItemMessageW(m_hWnd, IDC_RANGE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szComboText));

            SendDlgItemMessageW(m_hWnd, IDC_RANGE, CB_SETCURSEL, 0, 0);

			SetStatusMessage(L"Say a color or a word.");
        }
        break;

        case WM_SHOWWINDOW:
        {
            // Initialize and start NUI processing
            Nui_Init();

			HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

			if(FAILED(hr))
			{
				SetStatusMessage(L"Could not CoInitializeEx.");
				return hr;
			}

			//ADDING FROM SPEECH APP 9/6/2012 Ross T
			hr = InitializeAudioStream();
			if (FAILED(hr))
			{
				SetStatusMessage(L"Could not initialize audio stream.");
				return hr;
			}

			hr = CreateSpeechRecognizer();
			if (FAILED(hr))
			{
				SetStatusMessage(L"Could not create speech recognizer. Please ensure that Microsoft Speech SDK and other sample requirements are installed.");
				return hr;
			}

			hr = LoadSpeechGrammar();
			if (FAILED(hr))
			{
				SetStatusMessage(L"Could not load speech grammar. Please ensure that grammar configuration file was properly deployed.");
				return hr;
			}

			hr = StartSpeechRecognition();
			if (FAILED(hr))
			{
				SetStatusMessage(L"Could not start recognizing speech.");
				return hr;
			}
        }
        break;

        case WM_USER_UPDATE_FPS:
        {
            ::SetDlgItemInt( m_hWnd, static_cast<int>(wParam), static_cast<int>(lParam), FALSE );

        }
        break;

        case WM_USER_UPDATE_COMBO:
        {
            UpdateKinectComboBox();
        }
        break;

		//TODO - want to direct non-context commands through here too...
        case WM_COMMAND:
        {
            if ( HIWORD(wParam) == CBN_SELCHANGE )
            {
                switch (LOWORD(wParam))
                {
                    case IDC_CAMERAS:
                    {
                        LRESULT index = ::SendDlgItemMessageW(m_hWnd, IDC_CAMERAS, CB_GETCURSEL, 0, 0);

                        // Don't reconnect as a result of updating the combo box
                        if ( !m_fUpdatingUi )
                        {
                            Nui_UnInit();
                            Nui_Zero();
                            Nui_Init(reinterpret_cast<BSTR>(::SendDlgItemMessageW(m_hWnd, IDC_CAMERAS, CB_GETITEMDATA, index, 0)));
                        }
                    }
                    break;

                    case IDC_TRACKEDSKELETONS:
                    {
                        LRESULT index = ::SendDlgItemMessageW(m_hWnd, IDC_TRACKEDSKELETONS, CB_GETCURSEL, 0, 0);
                        UpdateTrackedSkeletonSelection( static_cast<int>(index) );
                    }
                    break;

                    case IDC_TRACKINGMODE:
                    {
                        LRESULT index = ::SendDlgItemMessageW(m_hWnd, IDC_TRACKINGMODE, CB_GETCURSEL, 0, 0);
                        UpdateTrackingMode( static_cast<int>(index) );
                    }
                    break;
                    case IDC_RANGE:
                    {
                        LRESULT index = ::SendDlgItemMessageW(m_hWnd, IDC_RANGE, CB_GETCURSEL, 0, 0);
                        UpdateRange( static_cast<int>(index) );
                    }
                    break;
                }
            }
        }
        break;

        // If the titlebar X is clicked destroy app
        case WM_CLOSE:
            if (NULL != m_pKinectAudioStream)
            {
                m_pKinectAudioStream->StopCapture();
            }

            if (NULL != m_pSpeechRecognizer)
            {
                m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE);
            }
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Uninitialize NUI
            Nui_UnInit();

            // Other cleanup
            ClearKinectComboBox();
            DeleteObject(m_hFontFPS);

            // Quit the main message pump
            PostQuitMessage(0);
            break;
    }

    return FALSE;
}

/// <summary>
/// Display a MessageBox with a string table table loaded string
/// </summary>
/// <param name="nID">id of string resource</param>
/// <param name="nType">type of message box</param>
/// <returns>result of MessageBox call</returns>
int CSkeletalViewerApp::MessageBoxResource( UINT nID, UINT nType )
{
    static TCHAR szRes[512];

    LoadStringW( m_hInstance, nID, szRes, _countof(szRes) );
    return MessageBoxW(m_hWnd, szRes, m_szAppTitle, nType);
}


/// <summary>
/// Initialize Kinect audio stream object.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT CSkeletalViewerApp::InitializeAudioStream()
{
    INuiAudioBeam*      pNuiAudioSource = NULL;
    IMediaObject*       pDMO = NULL;
    IPropertyStore*     pPropertyStore = NULL;
    IStream*            pStream = NULL;

    // Get the audio source
	HRESULT hr = 0;
	if(m_pNuiSensor != 0 && !useMicrophone)
	{
		hr = m_pNuiSensor->NuiGetAudioSource(&pNuiAudioSource);
		if (SUCCEEDED(hr))
		{
			hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

			if (SUCCEEDED(hr))
			{
				hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);
    
				// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
				// Possible values are:
				//   SINGLE_CHANNEL_AEC = 0
				//   OPTIBEAM_ARRAY_ONLY = 2
				//   OPTIBEAM_ARRAY_AND_AEC = 4
				//   SINGLE_CHANNEL_NSAGC = 5
				PROPVARIANT pvSysMode;
				PropVariantInit(&pvSysMode);
				pvSysMode.vt = VT_I4;
				pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
				pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
				PropVariantClear(&pvSysMode);

				// Set DMO output format
				WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
				DMO_MEDIA_TYPE mt = {0};
				MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    
				mt.majortype = MEDIATYPE_Audio;
				mt.subtype = MEDIASUBTYPE_PCM;
				mt.lSampleSize = 0;
				mt.bFixedSizeSamples = TRUE;
				mt.bTemporalCompression = FALSE;
				mt.formattype = FORMAT_WaveFormatEx;	
				memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));
    
				hr = pDMO->SetOutputType(0, &mt, 0);

				if (SUCCEEDED(hr))
				{
					m_pKinectAudioStream = new KinectAudioStream(pDMO);

					hr = m_pKinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

					if (SUCCEEDED(hr))
					{
						hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

						if (SUCCEEDED(hr))
						{
							hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
						}
					}
				}

				MoFreeMediaType(&mt);
			}
		}

		SafeRelease(pStream);
		SafeRelease(pPropertyStore);
		SafeRelease(pDMO);
		SafeRelease(pNuiAudioSource);
	}

    return hr;
}


/// <summary>
/// Create speech recognizer that will read Kinect audio stream data.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT CSkeletalViewerApp::CreateSpeechRecognizer()
{
    ISpObjectToken *pEngineToken = NULL;
    
	/*CComPtr<ISpRecognizer> g_cpEngine;
	HRESULT h = g_cpEngine.CoCreateInstance(CLSID_SpSharedRecognizer);*/

	/*CComPtr<ISpRecoContext> cpRecoContext;
	HRESULT hr = cpRecoContext.CoCreateInstance(CLSID_SpSharedRecoContext);*/

	//cpRecoContext->GetRecognizer(&m_pSpeechRecognizer);
   //HRESULT hr = CoCreateInstance(CLSID_SpSharedRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);
   HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);

    if (SUCCEEDED(hr))
    {
		if(!useMicrophone)
		{
			m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);
			hr = SpFindBestToken(SPCAT_RECOGNIZERS,L"Language=409;Kinect=True",NULL,&pEngineToken);

			if (SUCCEEDED(hr))
			{
				m_pSpeechRecognizer->SetRecognizer(pEngineToken);
				hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
			}
		}
		else
		{
			CComPtr<ISpObjectToken>      cpObjectToken;
			CComPtr<ISpAudio>            cpAudio;

			// Set up the inproc recognizer audio
			// input with an audio input object token.

			// Get the default audio input token.
			hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpObjectToken);
			if (SUCCEEDED(hr))
			{
			   // Set the audio input to our token.
			   hr = m_pSpeechRecognizer->SetInput(cpObjectToken, TRUE);
			}

			if (SUCCEEDED(hr))
			{
			   // Set up the inproc recognizer audio input with an audio input object.

			   // Create the default audio input object.
			   hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &cpAudio);
			}

			if (SUCCEEDED(hr))
			{
			   // Set the audio input to our object.
			   hr = m_pSpeechRecognizer->SetInput(cpAudio, TRUE);
			}

			if (SUCCEEDED(hr))
			{
			   // Ask the shared recognizer to re-check the default audio input token.
			   hr = m_pSpeechRecognizer->SetInput(NULL, TRUE);
			}

			if (SUCCEEDED(hr))
			{
			   // If hr = SPERR_ENGINE_BUSY, then retry later.
			   hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
			}
		}
    }

    SafeRelease(pEngineToken);

    return hr;
}

/// <summary>
/// Load speech recognition grammar into recognizer.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT CSkeletalViewerApp::LoadSpeechGrammar()
{
    HRESULT hr = m_pSpeechContext->CreateGrammar(0, &m_pSpeechGrammar);

    if (SUCCEEDED(hr))
    {
		//HRESULT h = m_pSpeechGrammar->LoadDictation(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Recognizers\\Tokens\\MS-1033-80-DESK\\Models\\1033\\L1033\\LMs\\Main\\Spelling", SPLO_STATIC);

		//m_pSpeechGrammar->SetDictationState(SPRS_ACTIVE);
		//DWORD err = GetLastError();
        // Populate recognition grammar from file
		//hr = m_pSpeechGrammar->LoadDictation(NULL, SPLO_STATIC);
		//m_pSpeechGrammar->SetDictationState(SPRS_ACTIVE);
		//hr = m_pSpeechGrammar->ResetGrammar(::SpGetUserDefaultUILanguage());
        hr = m_pSpeechGrammar->LoadCmdFromFile(GrammarFileName, SPLO_STATIC);
		//hr = m_pSpeechGrammar->LoadCmdFromProprietaryGrammar(
		//GrammarBuilder *dictation = new GrammarBuilder();
		//dictation.AppendDictation();
    }

    return hr;
}

/// <summary>
/// Start recognizing speech asynchronously.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT CSkeletalViewerApp::StartSpeechRecognition()
{
	HRESULT hr = 0;
	if(m_pKinectAudioStream != 0)
	{
		hr = m_pKinectAudioStream->StartCapture();

		if (SUCCEEDED(hr))
		{
			// Specify that all top level rules in grammar are now active
			m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

			// Specify that engine should always be reading audio
			m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

			// Specify that we're only interested in receiving recognition events
			m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

			// Ensure that engine is recognizing speech and not in paused state
			hr = m_pSpeechContext->Resume(0);
			if (SUCCEEDED(hr))
			{
				m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
			}
		}
	}
	else if(useMicrophone)
	{
		// Specify that all top level rules in grammar are now active
		m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

		// Specify that engine should always be reading audio
		m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

		// Specify that we're only interested in receiving recognition events
		m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

		// Ensure that engine is recognizing speech and not in paused state
		hr = m_pSpeechContext->Resume(0);
		if (SUCCEEDED(hr))
		{
			m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
		}
	}
    return hr;
}

/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void CSkeletalViewerApp::ProcessSpeech()
{
    const float ConfidenceThreshold = 0.3f;

    SPEVENT curEvent;
    ULONG fetched = 0;
    HRESULT hr = S_OK;

    m_pSpeechContext->GetEvents(1, &curEvent, &fetched);

    while (fetched > 0)
    {
        switch (curEvent.eEventId)
        {
            case SPEI_RECOGNITION:
                if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
                {
                    // this is an ISpRecoResult
                    ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                    SPPHRASE* pPhrase = NULL;
                    
                    hr = result->GetPhrase(&pPhrase);
                    if (SUCCEEDED(hr))
                    {
                        if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
                        {
                            const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
                            if (pSemanticTag->SREngineConfidence > ConfidenceThreshold)
                            {
								if(useFiona)
								{
									float fSenderValue = MapSpeechTagToColor(pSemanticTag->pszValue);
									//make a "speech" packet..
									if(fionaConnected)
									{
										UpdateSpeechData *pSpeech = new UpdateSpeechData(BasePacket::UPDATE_SPEECH);
										pSpeech->SetPayload(fSenderValue);
										EnterCriticalSection(&cs);
										fionaPackets.push_back(pSpeech);
										LeaveCriticalSection(&cs);
									}
								}
								else
								{
									float fSenderValue = MapSpeechTagToAction(pSemanticTag->pszValue);
									sender.SetSpeechValue(fSenderValue);
								}
                            }
                        }
                        ::CoTaskMemFree(pPhrase);
                    }
                }
                break;
        }

        m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    }

    return;
}

float CSkeletalViewerApp::MapSpeechTagToColor(LPCWSTR pszSpeechTag)
{
	SetStatusMessage(L"Say a color or a word.");

	float action = 0;
    for (int i = 0; i < COLOR_NAMES_MAX; ++i)
    {
		//get color name in upper case..
		wchar_t cName[256];
		memset(cName, 0, sizeof(wchar_t)*256);
		wcscpy(cName, color_data[i].name);
		wchar_t *sUpper = _wcsupr(cName);
		if(wcscmp(sUpper, pszSpeechTag)==0)
        {	
			action = (float)i;
			//SetStatusMessage(L"Found color!");
			SetStatusMessage(pszSpeechTag); 
            break;
        }
    }

	if(action == 0)
	{
		SetStatusMessage(pszSpeechTag); 
	}

	return action;
}

/// <summary>
/// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
/// </summary>
/// <returns>
/// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
/// </returns>
float CSkeletalViewerApp::MapSpeechTagToAction(LPCWSTR pszSpeechTag)
{
    struct SpeechTagToAction
    {
        LPCWSTR pszSpeechTag;
        float action;
    };
    /*const SpeechTagToAction Map[] =
    {
        {L"FORWARD", TurtleActionForward},
        {L"BACKWARD", TurtleActionBackward},
        {L"LEFT", TurtleActionTurnLeft},
        {L"RIGHT", TurtleActionTurnRight}
    };*/

	const SpeechTagToAction Map[] =
    {
		{L"SCALE",  1},
        {L"MOVE",   2},
		{L"ROTATE",  3},
        {L"GRAB",   4},
        {L"RELEASE",5},
		{L"RETURN", 6},
		{L"SELECT", 7},
		{L"STOP", 8},
		{L"MULTI-SELECT", 9},
		{L"KEEP", 10},	//paint select
		{L"CLEAR", 11},	//clear whole selection
		{L"DESELECT", 12},
		{L"BOX",	13},
		{L"SPHERE", 14},
		{L"CAPSULE", 15},
		{L"CYLINDER", 16},
		{L"PLANE", 17},
		{L"SAVE", 18},
		{L"LOAD", 19},
		{L"DELETE", 20},
		{L"DUPLICATE", 21},
		{L"COPY", 22},
		{L"PASTE", 23},
		{L"PAINT", 24},
		{L"MODEL SEARCH", 25}, 
		{L"SEARCH RESULTS", 26}
    };

    float action = 0;//TurtleActionNone;

    for (int i = 0; i < _countof(Map); ++i)
    {
        if (0 == wcscmp(Map[i].pszSpeechTag, pszSpeechTag))
        {
            action = Map[i].action;		
            break;
        }
    }

	if(action == 25)
	{
		modelSearchOn = true;
		action = -1;
	}

	if(action == 0 && modelSearchOn)
	{
		action = -1;
		//output the model search file...
		char newString[2048];
		memset(newString, 0, 2048);
		size_t convertedChars=0;
		//convert to regular string from wide..
		wcstombs_s(&convertedChars, newString, wcslen(pszSpeechTag)+1, pszSpeechTag, _TRUNCATE);
		FILE *f = fopen("..\\..\\loading_room\\model_search\\model_search.txt", "w");
		size_t len = strlen(newString);
		fwrite(&newString, len, 1, f);
		fclose(f);
		modelSearchOn = false;
	}

	SetStatusMessage(pszSpeechTag); 

    return action;
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void CSkeletalViewerApp::SetStatusMessage(const WCHAR* szMessage)
{
    SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}

void CSkeletalViewerApp::_FionaUTSyncPackPackets(char* buf, int sz)
{
	int p = sizeof(float);

	int nPacket = fionaPackets.size();
	int bufSize = 0;
	memcpy(buf, &nPacket, sizeof(int));
	bufSize += sizeof(int);
	memcpy(buf+bufSize, &sz, sizeof(int));
	bufSize += sizeof(int);
	
	for(int i=0; i<(int)nPacket; i++ )
	{
		memcpy(buf + bufSize, (char*)fionaPackets[i]->GetHeader(), sizeof(PacketHeader));
		bufSize += sizeof(PacketHeader);
		memcpy(buf + bufSize, (char*)fionaPackets[i]->GetPayload(), fionaPackets[i]->GetSize());
		bufSize += fionaPackets[i]->GetSize();
	}
#if DEBUG_PRINT > 1
	printf("Buf size %d\n", bufSize);
#endif
}

int CSkeletalViewerApp::_FionaUTSyncGetTotalSize(void)
{
	//assemble packet - current test code does this in the VPRN call back
	//assemble packet from queue
	//figure out total size and number of packets we're sending over...
	int numPackets = fionaPackets.size();
	
	int totalSize = sizeof(int) * 2;	//for num packets and total size
#if DEBUG_PRINT > 1
	printf("Counting size of %d packets ", numPackets);
#endif
	for(int i=0; i<(int)numPackets; i++ )
	{
		totalSize+=sizeof(PacketHeader)+fionaPackets[i]->GetSize();
	}
#if DEBUG_PRINT > 1
	printf("total size %d\n", totalSize);
#endif
	return totalSize;
}

void CSkeletalViewerApp::_FionaUTSyncClearPackets(void)
{
	for(int i=0; i<(int)fionaPackets.size(); i++)
	{ 
		delete fionaPackets[i]; 
		fionaPackets[i]=NULL; 
	}
	fionaPackets.clear();
}

// Sync functions called within _FionaUTFrame
void CSkeletalViewerApp::_FionaUTSyncMasterSync(void)
{
	EnterCriticalSection(&cs);

	int totalSize = _FionaUTSyncGetTotalSize();

	//ROSS TODO - could make this a statically sized buffer that's sufficiently large so we avoid the dynamic allocation here..
	
	//char * buf = new char[totalSize];
	//memset(buf, 0, totalSize);
	
	_FionaUTSyncPackPackets(masterBuffer, totalSize);
	fionaNetMaster->SendCavePacket(masterBuffer, totalSize);
	
	//delete[] buf;
	_FionaUTSyncClearPackets();
	
	LeaveCriticalSection(&cs);
}

