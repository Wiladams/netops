/*
	This is the minimalist example of what's needed to listen
	for UDP broadcast packets.

	At a bare minimum, you need a port to listen to
	On top of that, you must specify an address.
	In the default case, we're using INADDR_ANY so that
	we'll receive all broadcast packets on the network
	that are headed to port MYPORT

	If we want to narrow it down, we can specify some
	broadcast address of a subnet, like 192.168.1.255
	As long as it matches whatever the server side is 
	sending out.
*/
#include "netapp.h"

constexpr uint16_t MYPORT = 9991;

void onRun()
{
	ASocket receiver(SOCK_DGRAM, AF_INET, IPPROTO_UDP, false);

	// The address we'll bind to so we can listen
	// binding to a particular port is necessary
	// so that we can use receiveFrom()
	// it tells the OS that you care to receive packets that are 
	// meant for this particular port.
	// You can go further and specify a particular IP address as well if 
	// you only want to receive messages meant for a particular network interface
	// using INADDR_ANY, means you'll take any messages that are headed
	// to any of the interfaces
	IPAddress bindAddr = IPV4Address(INADDR_ANY, MYPORT,  AF_INET);

	receiver.bindTo(bindAddr);

	constexpr int bigbuffLen = 1400;
	char bigbuff[bigbuffLen];
	IPAddress bcastAddr;

	while (true) {
		bcastAddr.reset();
		
		// receive a broadcast packet
		int res = receiver.receiveFrom(bcastAddr, bigbuff, bigbuffLen);

		if (SOCKET_ERROR == res)
			break;

		bigbuff[res] = 0;
		printf("Heard: %s\n", bigbuff);
	}
}