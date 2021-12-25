/*
	Minimalist example of what's required to broadcast
	UDP packets on a network.

	You must minimally specify a port number
	You must also specify an address.  By default,
	we're using the INADDR_BROADCAST address, so this 
	will hit every machine on the local area network.

	It will probably be safer to use a local subnet mask
	so as to limit the potential spread of these 
	broadcast packets.  In reality, there's not much concern
	because most routers and firewalls will not forward such
	broadcast packets.
*/
#include "netapp.h"
#include <cstdio>

constexpr uint16_t MYPORT = 9991;

void onRun()
{
	// Setup a UDP Sender socket
	ASocket sender(SOCK_DGRAM, AF_INET, IPPROTO_UDP, false);

	// set socket to allow broadcast
	// if you don't do this, when you go to send
	// you'll get an error saying the socket doesn't
	// have permissions.
	auto res = sender.setBroadcast();

	// Construct address we're going to be broadcasting to
	IPAddress bcastAddr = IPV4Address(INADDR_BROADCAST, MYPORT,  AF_INET);

	// This is the buffer we're using to stuff our
	// broadcast command into
	int commandCount = 0;
	constexpr int bigbuffLen = 1400;
	char bigbuff[bigbuffLen];

	// Just constantly blast stuff out into 
	// the multi-cast channel
	while (true)
	{
		commandCount++;

		sprintf_s(bigbuff, "Command: %d", commandCount);
		int packetLen = strlen(bigbuff);

		int res = sender.sendTo(bcastAddr, bigbuff, packetLen);

		//printf("sender.sendTo: %d\n", res);

		// If there's any kind of error, just stop
		if (SOCKET_ERROR == res)
			break;
	}
}
