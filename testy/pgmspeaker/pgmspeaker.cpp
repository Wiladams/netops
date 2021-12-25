#include "netapp.h"
#include "pgm.h"

#include <cstdio>

void onRun()
{
	// Setup a PGM Sender socket
	PGMSender sender("224.25.25.10");

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

		auto [err, bytesSent] = sender.send(bigbuff, packetLen);
		
		printf("sender.send: %d  %d\n", err, bytesSent);

		if (err != 0)
			break;
	}
}