
#include "stdafx.h"

#include "CaveMaster.h"

#define DEBUG_PRINT 0



CaveMaster::~CaveMaster(void)
{
	for(int i = 0; i < numSlaves; ++i)
	{
		delete connections[i];
		connections[i] = 0;
	}

	delete[] connections;
}

void CaveMaster::ListenForSlaves(void)
{
	//TODO - check that the connection receive'd IP matches the ones listed above...?
	CaveSocket listener;
	
	listener .Create();
	listener .Listen(port);

	connections = new CaveSocket*[numSlaves];

	for(int i = 0; i < numSlaves; ++i)
	{
#if DEBUG_PRINT > 0
		::OutputDebugStringW(_T("Master waiting for connection from slave\n"));
		printf("Master waiting for connection from slave %i\n", i);			
#endif
		CaveSocket *pAccept = listener.Accept();
		connections[i] = new CaveSocket();
		connections[i]->InitFromSocket(pAccept->_socket);
#if DEBUG_PRINT > 0
		::OutputDebugStringW(_T("Master accepted connection from slave \n"));
		printf("Master accepted connection from slave %i\n", i);			
#endif
	}
}

void CaveMaster::Handshake(void)
{
	for(int i = 0; i < numSlaves; ++i)
	{
		//this receive blocks..
		//when all slaves contact the master, we continue
		char wakeUpBit;
#if DEBUG_PRINT > 1
		printf("waiting for handshake from slave # %d\n",i);
		::OutputDebugStringW(_T("waiting for handshake from slave \n"));
#endif
		connections[i]->Receive(&wakeUpBit, 1, 0);
	}
}

void CaveMaster::SendCavePacket(char *buffer, int len)
{
#if DEBUG_PRINT > 1
	printf("finally sending data: %d\n",len);
	::OutputDebugStringW(_T("finally sending data: \n"));
#endif
	for(int i = 0; i < numSlaves; ++i)
	{
		//send the current packet to the slaves
		connections[i]->Send(buffer, len, 0);
	}
}

void CaveMaster::WakeupSlaves(void)
{
	char wakeUpBit = '\0';
	for(int i = 0; i < numSlaves; ++i)
	{
#if DEBUG_PRINT > 1
		printf("waking up slaves: slave # %d\n",i);
		::OutputDebugStringW(_T("waking up slaves: slave \n"));
#endif
		//send the current packet to the slaves
//		connections[i]->Send(&wakeUpBit, 1, 0);
		connections[i]->Send(&wakeUpBit, 1);
	}
}

void TestPacket::SetPayload(float fTest)
{
	w.test = fTest;
}

void TestPacket::SetPayloadFromBuffer(char *buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
}

void UpdateKinectData::SetPayload(float kinectData[140])
{
	memcpy((void*)&w, (void*)kinectData, 140 * sizeof(float));
}

void UpdateKinectData::SetPayloadFromBuffer(char * buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
} 

void UpdateWiiFitData::SetPayload(float wiiFitData[4])
{
	memcpy((void*)&w, (void*)wiiFitData, 4 * sizeof(float));
}

void UpdateWiiFitData::SetPayloadFromBuffer(char * buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
}

void UpdateEMGData::SetPayload(unsigned int emg[4], float fVoltage)
{
	for(int i = 0; i < 4; ++i)
	{
		w.values[i] = emg[i];
	}
	w.fVoltage = fVoltage;
}

void UpdateEMGData::SetPayloadFromBuffer(char * buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
}


void UpdateSpeechData::SetPayload(float sV)
{
	w.speechVal = sV;
}

void UpdateSpeechData::SetPayloadFromBuffer(char * buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
}

void UpdateLeapData::CopyHand(const Leap::Hand *pHand, LeapData::HandData & ourHand)
{
	Leap::Vector handDir = pHand->direction();
	Leap::Vector handPos = pHand->palmPosition();
	Leap::Vector handVel = pHand->palmVelocity();
	Leap::Vector palmDir = pHand->palmNormal();
	Leap::Vector sphereC = pHand->sphereCenter();

	static const float toMeters = 0.001f;
	ourHand.valid = true;
	ourHand.handDirection[0] = handDir.x;
	ourHand.handDirection[1] = handDir.y;
	ourHand.handDirection[2] = handDir.z;
	ourHand.handPosition[0] = handPos.x * toMeters;
	ourHand.handPosition[1] = handPos.y * toMeters;
	ourHand.handPosition[2] = handPos.z * toMeters;
	ourHand.handVelocity[0] = handVel.x * toMeters;
	ourHand.handVelocity[1] = handVel.y * toMeters;
	ourHand.handVelocity[2] = handVel.z * toMeters;
	ourHand.palmNormal[0] = palmDir.x;
	ourHand.palmNormal[1] = palmDir.y;
	ourHand.palmNormal[2] = palmDir.z;
	ourHand.sphereCenter[0] = sphereC.x * toMeters;
	ourHand.sphereCenter[1] = sphereC.y * toMeters;
	ourHand.sphereCenter[2] = sphereC.z * toMeters;

	ourHand.sphereRadius = pHand->sphereRadius() * toMeters;

	//now copy optional fingers...
	int fingerCount = pHand->fingers().count();
	if(fingerCount > 0)
	{
		for(int i = 0; i < fingerCount; ++i)
		{
			CopyFinger(&(pHand->fingers()[i]), ourHand.fingers[i]);
		}
	}
}

void UpdateLeapData::CopyFinger(const Leap::Finger *pFinger, LeapData::HandData::FingerData & ourFinger)
{
	Leap::Vector fingerDir = pFinger->direction();
	Leap::Vector fingerPos = pFinger->tipPosition();
	Leap::Vector fingerVel = pFinger->tipVelocity();

	static const float toMeters = 0.001f;
	ourFinger.valid = true;
	ourFinger.tipDirection[0] = fingerDir.x;
	ourFinger.tipDirection[1] = fingerDir.y;
	ourFinger.tipDirection[2] = fingerDir.z;
	ourFinger.length = pFinger->length() * toMeters;
	ourFinger.tipPosition[0] = fingerPos.x * toMeters;
	ourFinger.tipPosition[1] = fingerPos.y * toMeters;
	ourFinger.tipPosition[2] = fingerPos.z * toMeters;
	ourFinger.tipVelocity[0] = fingerVel.x * toMeters;
	ourFinger.tipVelocity[1] = fingerVel.y * toMeters;
	ourFinger.tipVelocity[2] = fingerVel.z * toMeters;
}

void UpdateLeapData::SetPayload(const Leap::Hand *pHand1, const Leap::Hand *pHand2, const Leap::Gesture::Type &gestureType1, const Leap::Gesture::State &state1, const Leap::Gesture::Type &gestureType2, const Leap::Gesture::State &state2)
{
	memset(&w, 0, sizeof(LeapData));

	w.hand1.valid = false;
	w.hand2.valid = false;
	w.hand1.t = gestureType1;
	w.hand2.t = gestureType2;
	w.hand1.s = state1;
	w.hand2.s = state2;
	w.hand1.fingers[0].valid = false;
	w.hand1.fingers[1].valid = false;
	w.hand1.fingers[2].valid = false;
	w.hand1.fingers[3].valid = false;
	w.hand1.fingers[4].valid = false;
	w.hand2.fingers[0].valid = false;
	w.hand2.fingers[1].valid = false;
	w.hand2.fingers[2].valid = false;
	w.hand2.fingers[3].valid = false;
	w.hand2.fingers[4].valid = false;
	
	if(pHand1 != 0)
	{	
		CopyHand(pHand1, w.hand1);
	}

	if(pHand2 != 0)
	{
		CopyHand(pHand2, w.hand2);
	}
}

void UpdateLeapData::SetPayloadFromBuffer(char * buf, int size)
{
	memcpy((void*)&w, (void*)buf, size);
}
