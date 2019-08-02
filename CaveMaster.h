#ifndef _CAVEMASTER_H_
#define _CAVEMASTER_H_

#include "Socket.h"
#include "leap/Leap.h"

class CaveMaster
{
	public:
		CaveMaster(int slaves=0, int p=0) : connections(0), numSlaves(slaves), port(p) {}
		~CaveMaster();
		
		void ListenForSlaves(void);
		void Handshake(void);
		void SendCavePacket(char *buffer, int len);
		void WakeupSlaves(void);

	private:
		CaveSocket **connections;
		int numSlaves;
		int port;
};
struct JoystickData
{
	int wandIdx;		//4 bytes
	float x;			//4 bytes
	float y;			//4 bytes
	float z;			//4 bytes
};

struct WandButtonData
{
	int buttonIdx;			//4 bytes
	int wandIdx;			//4 bytes
	int state;				//4 bytes
};

struct TrackerData
{
	int trackerIdx;		//4 bytes
	float pos[3];		//12 bytes
	float quat[4];		//16 bytes
};

struct KeyboardData
{
	unsigned int key;		//4 bytes
	unsigned int modifer;	//4 bytes
};

struct ConfigData
{
	float	lEye[3];
	float	rEye[3];
	float	trackerOffset[3];
	float	kevinOffset[3];
};

struct PhysicsData
{
	float timeStep;
	float currTime;
};

struct Test
{
	float test;
};

struct PacketHeader
{
	int type;	//type of following packet
	int size;	//size of following packet
};

struct KinectData
{
	// skeleton data
	float values[140];
};

struct KinectVideoData
{	
	unsigned long	colorFrame;
	// rgba
	unsigned char	color[640*480*4];
	
	unsigned long	depthFrame;
	// depth + player id -- look at NuiCameraImage
	unsigned short	depth[320*240];
};


struct WiiFitData
{
	float values[4];
};

struct EMGData
{
	unsigned int values[4];
	float fVoltage;
};

struct SpeechData
{
	float speechVal;
};

struct VMDData
{
	char vmdMsg[512];	//assuming a max length of 512 bytes...
};

struct TimeVaryingData
{
	int fileIndex;
	int bufferIndex;
};

struct FrameCountData
{
	unsigned long long frameCount;
};

#ifndef LINUX_BUILD
struct LeapData
{
	//possible leap hand data
	struct HandData
	{
		//possible leap finger data
		struct FingerData
		{
			bool valid;
			float length;
			float tipPosition[3];
			float tipDirection[3];
			float tipVelocity[3];
		};

		bool valid;
		float sphereRadius;
		float sphereCenter[3];
		float palmNormal[3];
		float handPosition[3];
		float handDirection[3];
		float handVelocity[3];
		
		Leap::Gesture::Type t;	//whether this hand had a gesture this frame (and if so what type?)
		Leap::Gesture::State s;	//the state of the gesture (started, in-progress, stopped)

		FingerData fingers[5];	//we allow up to 5 fingers for a hand
	};

	//two hands max for now..
	HandData hand1;
	HandData hand2;
};
#endif

struct CameraData
{
	float pos[3];
	float rot[4];
};

struct MatrixData
{
	unsigned int index;
	float p[16];
	float v[16];
};

//add more packet types here
struct ControllerPacket
{
    unsigned short                      wButtons;
    unsigned char                       bLeftTrigger;
    unsigned char                       bRightTrigger;
    short                               sThumbLX;
    short                               sThumbLY;
    short                               sThumbRX;
    short                               sThumbRY;
};

//add more packet types here

class BasePacket 
{
	public:

		typedef enum
		{
			UPDATE_TRACKER=0,
			UPDATE_JOYSTICK,
			UPDATE_WANDBUTTONS,
			UPDATE_KEYBOARD,
			UPDATE_CONFIG,
			UPDATE_PHYSICS,
			UPDATE_KINECT,
			UPDATE_WIIFIT,
			UPDATE_EMG,
			UPDATE_SPEECH,
			UPDATE_VMD,
			UPDATE_LEAP,
			UPDATE_KINECT_VIDEO,
			UPDATE_TIME_VARYING_FILE,
			UPDATE_CAMERA,
			UPDATE_FRAME_NUMBER,
			UPDATE_MATRIX,
			UPDATE_CONTROLLER,
			TEST,
			NUM_COMMAND_TYPES
		} commandType_t;

		typedef enum
		{
			PER_FRAME=0,
			EVENT_DRIVEN,
			APP_SPECIFIC,
			NO_TYPE,
			NUM_PACKET_TYPES
		} packetType_t;

		BasePacket() : packetType(NO_TYPE)
		{
			header.type = 0;
			header.size = 0;
		}

		BasePacket(unsigned int commandType, packetType_t p) : packetType(p)
		{	
			header.type = commandType;
			header.size = 0;
		}

		virtual ~BasePacket() {}

		unsigned int			GetCommandType(void) const	{ return header.type; }
		unsigned int			GetSize(void) const			{ return header.size; }
		void *					GetHeader(void)				{ return &header; }
		virtual void *			GetPayload(void)			=0;

	protected:
		PacketHeader header;
		packetType_t packetType;
};

class UpdateKinectData : public BasePacket
{
public:
	UpdateKinectData(unsigned int commandType=0) : BasePacket(commandType, PER_FRAME) { header.size = sizeof(w); }
	virtual ~UpdateKinectData() {}
	
	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(float fKinectData[140]);
	void					SetPayloadFromBuffer(char * buf, int size);
	const KinectData&		GetData(void) const { return w; }

protected:
	KinectData w;
};

class UpdateWiiFitData : public BasePacket
{
public:
	UpdateWiiFitData(unsigned int commandType=0) : BasePacket(commandType, PER_FRAME) { header.size = sizeof(w); }
	virtual ~UpdateWiiFitData() {}
	
	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(float fWiiFitData[4]);
	void					SetPayloadFromBuffer(char * buf, int size);
	const WiiFitData&		GetData(void) const { return w; }

protected:
	WiiFitData w;
};

class UpdateEMGData : public BasePacket
{
public:
	UpdateEMGData(unsigned int commandType=0) : BasePacket(commandType, PER_FRAME) { header.size = sizeof(w); }
	virtual ~UpdateEMGData() {}
	
	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(unsigned int emg[4], float fVoltage);
	void					SetPayloadFromBuffer(char * buf, int size);
	const EMGData&			GetData(void) const { return w; }

protected:
	EMGData w;
};

class UpdateSpeechData : public BasePacket
{
public:
	UpdateSpeechData(unsigned int commandType=0) : BasePacket(commandType, EVENT_DRIVEN) { header.size = sizeof(w); }
	virtual ~UpdateSpeechData() {}
	
	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(float sV);
	void					SetPayloadFromBuffer(char * buf, int size);
	const SpeechData&		GetData(void) const { return w; }

protected:
	SpeechData w;
};


class TestPacket : public BasePacket
{
public:
	TestPacket(unsigned int commandType=0) : BasePacket(commandType, PER_FRAME) { header.size = sizeof(w); }
	virtual ~TestPacket() {}

	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(float fTest);
	void					SetPayloadFromBuffer(char * buf, int size);
	const Test &			GetPacket(void) const { return w; }

protected:
	Test w;
};

class UpdateLeapData : public BasePacket
{
public:
	UpdateLeapData(unsigned int commandType=0) : BasePacket(commandType, PER_FRAME) { header.size = sizeof(w); }
	virtual ~UpdateLeapData() {}
	
	virtual void *			GetPayload(void)			{ return (void*)&w; }
	void					SetPayload(const Leap::Hand *pHand1, const Leap::Hand *pHand2, const Leap::Gesture::Type &gestureType1, const Leap::Gesture::State &state1, const Leap::Gesture::Type &gestureType2, const Leap::Gesture::State &state2);
	void					SetPayloadFromBuffer(char * buf, int size);
	const LeapData&			GetData(void) const			{ return w; }

protected:
	void					CopyHand(const Leap::Hand *pHand, LeapData::HandData & ourHand);
	void					CopyFinger(const Leap::Finger *pFinger, LeapData::HandData::FingerData & ourFinger);
	
	LeapData w;
};


class UpdateKinectVideoData : public BasePacket
{
public:
	UpdateKinectVideoData(const KinectVideoData& d) : BasePacket(UPDATE_KINECT_VIDEO, PER_FRAME), data(d) {header.size = sizeof(data); }

	void*	GetPayload() { return reinterpret_cast<void*>(&data); }
	inline KinectVideoData& GetData() { return data; }

	KinectVideoData			data;



};


#endif