/*
 *   Sender for LEL CAVE
 *   Copyright (C) 2002
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LELSENDER_INCLUDED__
#define __LELSENDER_INCLUDED__

#include "PracticalSocket.h"
#include <vector>
#include <Windows.h>
#pragma comment(lib, "wsock32.lib")
#include <NuiApi.h>

struct KinectPacket
{
	float speechVal;
	float skelPositions[NUI_SKELETON_POSITION_COUNT*3];
	float balanceBoard[4];
}; 

class LELSender
{
private:
	bool exit;
	bool ready;
	bool firstSend;
	
	std::vector<TCPSocket*> connections;
	
	int numRequestedConnections;
	int port;

	KinectPacket currentPacket;

public:
	//LELSender(int _numRequestedConnections=4, int _port = 19954);	//CAVE
	LELSender(int _numRequestedConnections=1, int _port = 19954);	//DEV LAB
	~LELSender();

	void start(void);

	int getPort()									{ return port; }
	bool isDone()									{ return exit; }
	void isReady(bool &b);
	void hasSentData(bool &b);
	bool hasAllConnections()						{ return (connections.size() == numRequestedConnections);}

	void setReady();
	void setNumRequestedConnections(int i)			{ numRequestedConnections=i; }
	
	std::vector<TCPSocket*> & getConnectionVector() { return connections; }

	void addConnection(TCPSocket* sock);
	void removeConnection(TCPSocket* sock);
	void removeAllConnections();

	bool send(void *buf, unsigned int size);

	bool SendKinectPacket(void);
	void SetSpeechValue(float value);
	void SetBalanceBoardValues(float *f4);
	void StoreSkeleton(float *f60);

	void storeDepthFrame(const unsigned short* data);
	void storeRGBFrame(const unsigned char* data);
};

#endif //__LELSENDER_INCLUDED__
