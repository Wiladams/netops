#pragma once

#include "../datachunk.h"

// A network packet consists of a payload, and a header.  The implementation
// is based on the DataChunk object.  This additional layer of abstraction
// allows a subsclass to implement packets that represent a specific protocol, such
// as RTP, without having to subclass the DataChunk object directly.
// Additionally, the Packetizer object deals with packets in the abstract,
// so, this class serves as the base class for all streams that are to be packetized.
//
class PacketBase
{
protected:
	DataChunk buffer;

public:
	// 1452 is best size for a non-fragmentable UDP packet
	// assuming an MTU of 1500
	PacketBase()
		:PacketBase(1452)
	{

	}

	PacketBase(int packetSize) 
		:buffer(packetSize)
	{

	}

	virtual int HeaderSize() { return 0; }

	DataChunk& Buffer() { return buffer; }

	virtual void reset() {
		//buffer.reset(0, HeaderSize);
		onReset();
	}

protected:
	virtual void onReset() {}
	virtual void validateBuffer(DataChunk& buffer) {}

};