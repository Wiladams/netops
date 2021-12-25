#include "netapp.h"

constexpr uint16_t MYPORT = 8881;

void onRun()
{
	// Create a UDP server socket
	IPV4Address srvraddress(INADDR_ANY, MYPORT);
	ASocket srvrsocket(SOCK_DGRAM, AF_INET,IPPROTO_UDP,false);
	//srvrsocket.setReuseAddress();
	srvrsocket.bindTo(srvraddress);

	// keep looping waiting to receive something to send back
	uint8_t bigbuff[512];
	int bigbufflen = 512;

	// Do a continuous loop waiting for something
	// to send back
	while (true) {
		IPAddress senderAddress;
		int res = srvrsocket.receiveFrom(senderAddress, bigbuff, bigbufflen);

		if (SOCKET_ERROR == res)
			break;

		bigbuff[res] = 0;
		printf("receiveFrom (%d): %s\n", res, bigbuff);

		// Send message back to sender
		res = srvrsocket.sendTo(senderAddress, bigbuff, res);

		printf("sendTo (%d)\n", res);
	}
}