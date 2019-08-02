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

#include "stdafx.h"
#include "LELSender.h"
#include <process.h>
#pragma comment(lib, "wsock32.lib")

//CRITICAL_SECTION cs;

void getConnection(void* x = NULL)
{
	printf("size of short %d\n", sizeof(short));
	LELSender *sender = (LELSender*)x;
	if (sender == NULL)
	{
		printf("error making sender object in thread\n");
		return;
	}
	TCPServerSocket *serverSock = new TCPServerSocket(sender->getPort(),1);
	if (serverSock == NULL)
	{
		printf("error making server at port %d\n",sender->getPort());
		return;
	}
	while(!sender->isDone())
	{
		printf("waiting for connection on port %d\n\a", sender->getPort());
		TCPSocket* sock = serverSock->accept();
		sender->addConnection(sock);
		Sleep(100);
	}
};

void receiveRequest(void* x = NULL)
{
	LELSender *sender = (LELSender*)x;
	if (sender == NULL)
	{
		printf("error making sender object in thread\n");
		return;
	}

	char flag;
	while(!sender->isDone())
	{
		bool bReady;
		sender->isReady(bReady);

		bool bHasSentData;
		sender->hasSentData(bHasSentData);
		
		if (sender->getConnectionVector().size() >= 0 && bHasSentData && !bReady)
		{
			std::vector<TCPSocket*> connections = sender->getConnectionVector();
			//printf("rec");
			for(unsigned int i = 0; i < connections.size(); i++)
			{
				//printf("receive %d: ",i);
				try
				{
					int test = connections[i]->recv(&flag,1);
				} 
				catch (const std::exception& e) 
				{

				}
				//printf("%d\n",test);
			}

			//only set this if we are recieving from a full set of senders?
			if (sender->hasAllConnections())
				sender->setReady();
		}
		Sleep(10);
	}
}

LELSender::LELSender(int _numRequestedConnections, int _port)
{
	exit=false;
	ready=false;
	firstSend=true;
	port=_port;
	numRequestedConnections=_numRequestedConnections;

	memset(&currentPacket, 0, sizeof(KinectPacket));

	//_beginthread(getConnection,0,this);
	//_beginthread(receiveRequest,0,this);
	//InitializeCriticalSection(&cs);
	//LeaveCriticalSection(&cs);
}

void LELSender::start(void)
{
	_beginthread(getConnection,0,this);
	_beginthread(receiveRequest,0,this);
}

LELSender::~LELSender()
{
	exit = true;
	//DeleteCriticalSection(&cs);
}

void LELSender::addConnection(TCPSocket* sock)
{
	//EnterCriticalSection(&cs);	//!!!!
		connections.push_back(sock);
		printf("Connection Added %d\n",connections.size());

		//if we have reached the number of connections we
		//want, we can start sending
		if (this->hasAllConnections())
		{
			ready=true;
			printf("now ready to send\n");
		}
	//LeaveCriticalSection(&cs);	//!!!!
}

void LELSender::removeAllConnections()
{
	std::vector<TCPSocket*> tempconnections;
		for(unsigned int i = 0; i < connections.size(); i++) {
				delete connections[i];
		}

		connections = tempconnections;

		printf("Connection Removed, %d remaining\n\a",connections.size());

		if (connections.size() == 0)
		{
			ready=false;
			firstSend=true;
			printf("back to the start\n" );
		}
}

void LELSender::removeConnection(TCPSocket* sock)
{
	//EnterCriticalSection(&cs);
		std::vector<TCPSocket*> tempconnections;
		for(unsigned int i = 0; i < connections.size(); i++) {
			if (connections[i] != sock)
				tempconnections.push_back(connections[i] );
			else
				delete connections[i];
		}

		connections = tempconnections;

		printf("Connection Removed, %d remaining\n\a",connections.size());

		if (connections.size() == 0)
		{
			ready=false;
			firstSend=true;
			printf("back to the start\n" );
		}
	//LeaveCriticalSection(&cs);
}

bool LELSender::send(void *buf, unsigned int size)
{
	//make a duplicate of the data
	void *sendBuffer = malloc(size);
	memcpy(sendBuffer, buf, size);

	bool status=false;
	//NOTE: Should I prevent sending here?? 
	//or should I force the people to call 
	//is ready?
	bool bReady;
	isReady(bReady);
	if (bReady)
	{
		for(unsigned int i = 0; i < connections.size(); i++) 
		{
			try 
			{
				connections.at(i)->send(sendBuffer, size);
			} 
			catch (const std::exception& e) 
			{
				removeConnection(connections[i]);
			}
		}
		status = true;
		
		//do this we can buffer one package
		if (firstSend && (connections.size() > 0))
		{
			firstSend=false;
		}
		//else
		ready=false;
	}

	free(sendBuffer);

	return status;
}

void LELSender::isReady(bool &b)
{
	b=ready; 
}

void LELSender::setReady(void)
{
	ready=true;
}

void LELSender::hasSentData(bool &b)
{
	b= (!firstSend);
}

bool LELSender::SendKinectPacket(void)
{
	//make a duplicate of the data
	size_t size = sizeof(KinectPacket);
	void *sendBuffer = malloc(size);
	memcpy(sendBuffer, &currentPacket, size);

	bool status=false;
	//NOTE: Should I prevent sending here?? 
	//or should I force the people to call 
	//is ready?
	if(ready)
	{
		for(unsigned int i = 0; i < connections.size(); i++) 
		{
			try 
			{
				connections.at(i)->send(sendBuffer, size);
			} 
			catch (const std::exception& e) 
			{
				removeConnection(connections[i]);
			}
		}
		status = true;
		
		//do this we can buffer one package
		if (firstSend && (connections.size() > 0))
		{
			firstSend=false;
		}
		//else
		ready=false;
		
		//reset speech value
		currentPacket.speechVal = 0.f;
	}

	free(sendBuffer);

	return status;
}

void LELSender::SetSpeechValue(float value)
{
	currentPacket.speechVal = value;
}

void LELSender::SetBalanceBoardValues(float *f4)
{
	memcpy(currentPacket.balanceBoard, f4, sizeof(float)*4);
}

void LELSender::StoreSkeleton(float *f60)
{
	memcpy(currentPacket.skelPositions, f60, sizeof(float)*60);
}