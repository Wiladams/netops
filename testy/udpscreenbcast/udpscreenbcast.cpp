//
// screenserve
//
// Demonstrate the usage of the grayscale 2D sampler.
// This sampler wraps any other source sampler, and converts
// the color values to grayscale values.
//
#include "netapp.h"

#include "screensnapshot.h"
#include "qoi.h"
#include "binstream.h"
#include "network.h"
#include "timing.h"
#include "maths.h"

#include <memory>

//static const int captureWidth = 1280;
//static const int captureHeight = 1024;
static const int captureWidth = 800;
static const int captureHeight = 600;

static const int channels = 4;
constexpr uint16_t BCAST_PORT = 9991;

// This is the buffer that will be used to encode images
constexpr size_t bigbuffSize = captureWidth * captureHeight * (channels + 1) + sizeof(qoi_header_t) + 4;

uint8_t bigbuff[bigbuffSize];
BinStream bs(bigbuff, bigbuffSize);

// Make the screen snapshot thing
ScreenSnapshot snapper(0, 0, captureWidth, captureHeight);

bool sendChunk(ASocket& s, IPAddress& addrTo, char* buff, int buffLen)
{
	// The ideal size of chunk to send should
	// match the send buffer size of the underlying
	// socket
	int32_t overallSize = buffLen;

	//int32_t idealSize = s.getSendBufferSize();
	int32_t idealSize = s.getSendBufferSize() / 2;
	//int32_t idealSize = s.getSendBufferSize()/4;
	//int32_t idealSize = 1400;

	//printf("udpscreenserve, overallsize: %d\n", overallSize);
	//printf("udpscreenserve, ideal chunk size: %d\n", idealSize);

	// Get a stream on the chunk
	BinStream chunkStream(buff, buffLen);


	// write out the overall size to be sent
	// if we fail on this return false
	int res = s.sendTo(addrTo, (const char*)&overallSize, 4);
	res = s.sendTo(addrTo, (const char*)&idealSize, 4);

	if (SOCKET_ERROR == res)
		return false;

	// While there's still stuff to be sent
	// send it out
	while (!chunkStream.isEOF()) {
		// we'll write idealSize bytes at a time
		// start by writing the number of bytes
		// into the packet header
		int remains = chunkStream.remaining();
		int payloadSize = (int)maths::Min((double)idealSize, (double)remains);
		//printf("payloadSize: %d\n", payloadSize);

		// Write the payload out and advance
		int res = s.sendTo(addrTo, (const char*)chunkStream.getPositionPointer(), payloadSize);

		// If there was an error
		// return false
		if (SOCKET_ERROR == res)
		{
			printf("send error: %d\n", s.getLastError());
			return false;
		}

		overallSize -= res;

		chunkStream.skip(res);
		//printf("overallSize: %d\n", overallSize);
		timing::sleep(4);
	}

	//printf("Server FINISHED one frame\n");
	return true;

}

void sendCurrentScreen(ASocket& sendersock, IPAddress& clientAddr)
{
	// take a snapshot of screen
	snapper.next();

	bs.seek(0);

	QOICodec::encode(bs, snapper.getData(), snapper.width(), snapper.height(), snapper.channels());

	size_t outLength = bs.tell();

	sendChunk(sendersock, clientAddr, (char*)bigbuff, outLength);
}

void onRun()
{
	// Setup our receiving port
	IPV4Address srvraddress(INADDR_ANY, BCAST_PORT);
	ASocket serversock(AF_INET, SOCK_DGRAM, IPPROTO_UDP, false);
	//serversock.setReuseAddress();
	auto bindRes = serversock.bindTo(srvraddress);

	if (!serversock.isValid()) {
		// Could not create socket
		printf("Could not create server: %d\n", serversock.getLastError());
		return;
	}

	uint64_t commandCount = 0;	// just to keep track of commands coming in
	IPAddress clientAddr;		// the address of the client we received a datagram from
	char clientbuff[512];

	while (true)
	{
		clientAddr.reset();
		memset(bigbuff, 0, 512);
		clientbuff[0] = 0;

		// Client sends us a command, so read that
		// command should be 'get frame'
		//printf("waiting for client command:\n");

		int bytesTransferred = serversock.receiveFrom(clientAddr, bigbuff, 512);

		commandCount = commandCount + 1;

		clientAddr.toString(clientbuff, 511);
		//printf("server.receiveFrom: (%d) client: %s\n", bytesTransferred, clientbuff);


		if (SOCKET_ERROR == bytesTransferred)
		{
			printf("receiveFrom ERROR: %d\n", serversock.getLastError());
			fflush(stdout);

			break;
		}
		else if (bytesTransferred == 0) {
			// just continue through to next message
			printf("ZERO bytes transferred\n");
			break;
		}
		else {
			// we got a command
			// we'll assume it's for 
			// taking a snapshot and sending it
			bigbuff[bytesTransferred] = 0;
			//printf("Command: %s\n", bigbuff);

			sendCurrentScreen(serversock, clientAddr);
		}
	}

}
