//------------------------------------------------------------------------------
// <copyright file="SkeletalViewer.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Declares of CSkeletalViewerApp class

#pragma once

#include "resource.h"
#include "NuiApi.h"
#include "DrawDevice.h"

#include "KinectAudioStream.h"

// For configuring DMO properties
#include <wmcodecdsp.h>

// For FORMAT_WaveFormatEx and such
#include <uuids.h>

// For speech APIs
// NOTE: To ensure that application compiles and links against correct SAPI versions (from Microsoft Speech
//       SDK), VC++ include and library paths should be configured to list appropriate paths within Microsoft
//       Speech SDK installation directory before listing the default system include and library directories,
//       which might contain a version of SAPI that is not appropriate for use together with Kinect sensor.
#include <sapi.h>
#include <sphelper.h>

#include "LELSender.h"
#include "wiimote.h"
#include "CaveMaster.h"
#include "leap/Leap.h"

class PlayerVideo;

#define SZ_APPDLG_WINDOW_CLASS          _T("SkeletalViewerAppDlgWndClass")
#define WM_USER_UPDATE_FPS              WM_USER
#define WM_USER_UPDATE_COMBO            WM_USER+1
#define WM_USER_UPDATE_TRACKING_COMBO   WM_USER+2

class FionaLeap : public Leap::Listener {
  public:
    virtual void onInit(const Leap::Controller&);
    virtual void onConnect(const Leap::Controller&);
    virtual void onDisconnect(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
};

class CSkeletalViewerApp
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CSkeletalViewerApp(bool bUseFiona=true);
    
    /// <summary>
    /// Destructor
    /// </summary>
    ~CSkeletalViewerApp();

    /// <summary>
    /// Initialize Kinect
    /// </summary>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT                 Nui_Init( );

    /// <summary>
    /// Initialize Kinect by instance name
    /// </summary>
    /// <param name="instanceName">instance name of Kinect to initialize</param>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT                 Nui_Init( OLECHAR * instanceName );

    /// <summary>
    /// Uninitialize Kinect
    /// </summary>
    void                    Nui_UnInit( );

    /// <summary>
    /// Zero out member variables
    /// </summary>
    void                    Nui_Zero( );

    /// <summary>
    /// Handle new color data
    /// </summary>
    /// <returns>true if a frame was processed, false otherwise</returns>
    bool                    Nui_GotColorAlert( );

    /// <summary>
    /// Handle new depth data
    /// </summary>
    /// <returns>true if a frame was processed, false otherwise</returns>
    bool                    Nui_GotDepthAlert( );
	
	/// Callback for new background-removed color frames
	bool					Nui_GotBgRemovedAlert();

    /// <summary>
    /// Handle new skeleton data
    /// </summary>
    bool                    Nui_GotSkeletonAlert( );

    /// <summary>
    /// Blank the skeleton display
    /// </summary>
    void                    Nui_BlankSkeletonScreen( );

    /// <summary>
    /// Draws a skeleton
    /// </summary>
    /// <param name="skel">skeleton to draw</param>
    /// <param name="windowWidth">width (in pixels) of output buffer</param>
    /// <param name="windowHeight">height (in pixels) of output buffer</param>
    void                    Nui_DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight );

    /// <summary>
    /// Draws a line between two bones
    /// </summary>
    /// <param name="skel">skeleton to draw bones from</param>
    /// <param name="bone0">bone to start drawing from</param>
    /// <param name="bone1">bone to end drawing at</param>
    void                    Nui_DrawBone( const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1 );

    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for the class instance
    /// </summary>
   /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Callback to handle Kinect status changes, redirects to the class callback handler
    /// </summary>
    /// <param name="hrStatus">current status</param>
    /// <param name="instanceName">instance name of Kinect the status change is for</param>
    /// <param name="uniqueDeviceName">unique device name of Kinect the status change is for</param>
    /// <param name="pUserData">additional data</param>
    static void CALLBACK    Nui_StatusProcThunk(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* pUserData);

    /// <summary>
    /// Callback to handle Kinect status changes
    /// </summary>
    /// <param name="hrStatus">current status</param>
    /// <param name="instanceName">instance name of Kinect the status change is for</param>
    /// <param name="uniqueDeviceName">unique device name of Kinect the status change is for</param>
    void CALLBACK           Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName );

    HWND                    m_hWnd;
    HINSTANCE               m_hInstance;

    int MessageBoxResource(UINT nID, UINT nType);
	
	// ROSS ADDED 9/6/2012 - made public
    // Event triggered when we detect speech recognition
    HANDLE                  m_hSpeechEvent;
    
	/// <summary>
    /// Process recently triggered speech recognition events.
    /// </summary>
    void                    ProcessSpeech();

	static LELSender		sender;

	static float			senderValue;
	static void				SendWithLELSender(void* x = NULL);
	static bool				exiting;

	// ROSS ADDED 9/14/2012 - wii fit balance board integration!
	wiimote remote;

	// ROSS ADDED 2.19.2013 - Leap Motion integration!

	FionaLeap				m_leap;
	Leap::Controller		m_leapController;

	bool					modelSearchOn;
	bool					useFiona;
	bool					useMicrophone;
	bool					comportOpen;
	static const int		m_port = 3;

	static std::vector<BasePacket*> fionaPackets;
	static CaveMaster		*fionaNetMaster;
	static bool				fionaConnected;

	static void				_FionaUTSyncMasterSync(void);

	void					HandleWiiFit(void);
	void					HandleEMG(void);
	static void				HandleEMGThread(void*x=0);

	static CRITICAL_SECTION cs;
	
	static const unsigned int FIONA_NETWORK_BUFFER_SIZE = 65536;
	static char masterBuffer[FIONA_NETWORK_BUFFER_SIZE];

private:
    /// <summary>
    /// Updates the combo box that lists Kinects available
    /// </summary>
    void                    UpdateKinectComboBox();

    /// <summary>
    /// Clears the combo box for selecting active Kinect
    /// </summary>
    void                    ClearKinectComboBox();

    /// <summary>
    /// Invoked when the user changes the tracking mode
    /// </summary>
    /// <param name="mode">tracking mode to switch to</param>
    void                    UpdateTrackingMode( int mode );

    /// <summary>
    /// Invoked when the user changes the range
    /// </summary>
    /// <param name="mode">range to switch to</param>
    void                    UpdateRange( int mode );

    /// <summary>
    /// Invoked when the user changes the selection of tracked skeletons
    /// </summary>
    /// <param name="mode">skelton tracking mode to switch to</param>
    void                    UpdateTrackedSkeletonSelection( int mode );
    
    /// <summary>
    /// Sets or clears the specified skeleton tracking flag
    /// </summary>
    /// <param name="flag">flag to set or clear</param>
    /// <param name="value">true to set, false to clear</param>
    void                    UpdateSkeletonTrackingFlag( DWORD flag, bool value );

    /// <summary>
    /// Sets or clears the specified depth stream flag
    /// </summary>
    /// <param name="flag">flag to set or clear</param>
    /// <param name="value">true to set, false to clear</param>
    void                    UpdateDepthStreamFlag( DWORD flag, bool value );

    /// <summary>
    /// Determines which skeletons to track and tracks them
    /// </summary>
    /// <param name="skel">skeleton frame information</param>
    void                    UpdateTrackedSkeletons( const NUI_SKELETON_FRAME & skel );

    /// <summary>
    /// Ensure necessary Direct2d resources are created
    /// </summary>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT                 EnsureDirect2DResources( );

    /// <summary>
    /// Dispose Direct2d resources 
    /// </summary>
    void                    DiscardDirect2DResources( );

    /// <summary>
    /// Converts a skeleton point to screen space
    /// </summary>
    /// <param name="skeletonPoint">skeleton point to tranform</param>
    /// <param name="width">width (in pixels) of output buffer</param>
    /// <param name="height">height (in pixels) of output buffer</param>
    /// <returns>point in screen-space</returns>
    D2D1_POINT_2F           SkeletonToScreen(Vector4 skeletonPoint, int width, int height);

    bool                    m_fUpdatingUi;
    TCHAR                   m_szAppTitle[256];    // Application title

    /// <summary>
    /// Thread to handle Kinect processing, calls class instance thread processor
    /// </summary>
    /// <param name="pParam">instance pointer</param>
    /// <returns>always 0</returns>
    static DWORD WINAPI     Nui_ProcessThread( LPVOID pParam );

    /// <summary>
    /// Thread to handle Kinect processing
    /// </summary>
    /// <returns>always 0</returns>
    DWORD WINAPI            Nui_ProcessThread( );

    // Current kinect
    INuiSensor *            m_pNuiSensor;
    BSTR                    m_instanceId;

	//ROSS ADDING 9/6/2012 - Speech stuff to combine the two demo apps..

    static LPCWSTR          GrammarFileName;

    // Audio stream captured from Kinect.
    KinectAudioStream*      m_pKinectAudioStream;

    // Stream given to speech recognition engine
    ISpStream*              m_pSpeechStream;

    // Speech recognizer
    ISpRecognizer*          m_pSpeechRecognizer;

    // Speech recognizer context
    ISpRecoContext*         m_pSpeechContext;

    // Speech grammar
    ISpRecoGrammar*         m_pSpeechGrammar;

	// default grammar
	//ISpGrammarBuilder*		m_pDefaultGrammar;

	//END ROSS ADDING 9/6/2012

	//ROSS ADDING 11/27/2012
	//integrating w/ fiona

	static void SendWithFiona(void* x);
	//END ROSS ADDING

    // Skeletal drawing
    ID2D1HwndRenderTarget *  m_pRenderTarget;
    ID2D1SolidColorBrush *   m_pBrushJointTracked;
    ID2D1SolidColorBrush *   m_pBrushJointInferred;
    ID2D1SolidColorBrush *   m_pBrushBoneTracked;
    ID2D1SolidColorBrush *   m_pBrushBoneInferred;
    D2D1_POINT_2F            m_Points[NUI_SKELETON_POSITION_COUNT];

    // Draw devices
    DrawDevice *            m_pDrawDepth;
    DrawDevice *            m_pDrawColor;
    ID2D1Factory *          m_pD2DFactory;

    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;
    
    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_hNextColorFrameEvent;
    HANDLE        m_hNextSkeletonEvent;
    HANDLE        m_pDepthStreamHandle;
    HANDLE        m_pVideoStreamHandle;


    HFONT         m_hFontFPS;
    BYTE          m_depthRGBX[640*480*4];
    DWORD         m_LastSkeletonFoundTime;
    bool          m_bScreenBlanked;
    int           m_DepthFramesTotal;
    DWORD         m_LastDepthFPStime;
    int           m_LastDepthFramesTotal;
    int           m_TrackedSkeletons;
    DWORD         m_SkeletonTrackingFlags;
    DWORD         m_DepthStreamFlags;

    DWORD         m_StickySkeletonIds[NUI_SKELETON_MAX_TRACKED_COUNT];


	PlayerVideo*  playerVideo;

	bool		  m_SendVideoFrames;




	//ROSS 9/6/2012 - ADDING FROM SPEECH EXAMPLE:
	/// <summary>
    /// Initialize Kinect audio stream object.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
	 HRESULT                 InitializeAudioStream();
    /// <summary>
    /// Create speech recognizer that will read Kinect audio stream data.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 CreateSpeechRecognizer();

    /// <summary>
    /// Load speech recognition grammar into recognizer.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 LoadSpeechGrammar();

    /// <summary>
    /// Start recognizing speech asynchronously.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 StartSpeechRecognition();


    /// <summary>
    /// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
    /// </summary>
    /// <returns>
    /// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
    /// </returns>
    float            MapSpeechTagToAction(LPCWSTR pszSpeechTag);

	float			MapSpeechTagToColor(LPCWSTR pszSpeechTag);


    /// <summary>
    /// Set the status bar message.
    /// </summary>
    /// <param name="szMessage">message to display.</param>
    void                    SetStatusMessage(const WCHAR* szMessage);

	static void _FionaUTSyncPackPackets(char* buf, int sz);
	static int _FionaUTSyncGetTotalSize(void);
	static void _FionaUTSyncClearPackets(void);
};

