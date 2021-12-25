#include "netapp.h"

void onRun()
{
	IPAddress srvrAddress;

	// Create server address
	IPHost host("192.168.1.9", "8881", SOCK_DGRAM);
	//IPHost host = IPHost::create("localhost", "8081", , SOCK_DGRAM);

	if (host.isValid())
	{
		srvrAddress = host.getAddress(0);
	}
	else {
		printf("Could not connect to host\n");
		return;
	}

	// Create a socket to talk to server
	IPV4Address localaddress(INADDR_ANY, 8881);
	ASocket sendingSocket(SOCK_DGRAM, AF_INET, IPPROTO_UDP, false);
	sendingSocket.setReuseAddress();
	sendingSocket.bindTo(localaddress);

	constexpr int bigbufflen = 64 * 1024;
	char bigbuff[bigbufflen];

	int commandCount = 1;
	IPAddress receivedAddress;

	while (true) {

		commandCount++;

		sprintf_s(bigbuff, "Echo This: %d", commandCount);
		int commandLen = strlen(bigbuff);

		int res = sendingSocket.sendTo(srvrAddress, bigbuff, commandLen);

		printf("sendTo %d (%d): \n", commandCount, res);

		if (res == SOCKET_ERROR)
			break;

		res = sendingSocket.receiveFrom(receivedAddress, bigbuff, bigbufflen);

		if (res == SOCKET_ERROR)
		{
			printf("receiveFrom Error: %d\n", sendingSocket.fLastError);
			break;
		}

		bigbuff[res] = 0;
		printf("receiveFrom (%d) ==> %s\n", res, bigbuff);


	}
}