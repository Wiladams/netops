#pragma once

#include "network.h"

//
// PGM is a reliable packet protocol
// This PGMSocket acts as the convenience 
// base to deal with it.
//
// The PGMSender, and PGMReceiver are what you will actually
// interact with.
//
// Note:  The PGM protocol is only available on windows if 
// MSMQ has been installed.  Without that installation, 
// this socket type will not exist.
// https://stackoverflow.com/questions/19272261/pgm-winsock2-failure-to-create-socket
//
class PGMSocket : public ASocket
{
public:
    //
    // socktype
    //   SOCK_RDM   - if you want message based semantics
    //   SOCK_STREAM - if you want streaming semantics
    //
    PGMSocket(int socktype)
    {
        SOCKET s = WSASocketA(AF_INET, socktype, IPPROTO_PGM, nullptr, 0, 0);
        if (INVALID_SOCKET == s)
            return;

        init(s, false);
    }

    // broadcast - give permission for socket to do broadcasts
//
    virtual bool setBroadcast()
    {
        uint32_t oneInt = 1;
        return setSocketOption(IPPROTO_PGM, SO_BROADCAST, (char*)&oneInt, sizeof(oneInt));
    }

    virtual bool setNoBroadcast()
    {
        uint32_t oneInt = 0;
        return setSocketOption(IPPROTO_PGM, SO_BROADCAST, (char*)&oneInt, sizeof(oneInt));
    }

};

//
// PGMSender is used to send packets out to a 
// broadcast address
//
class PGMSender
{
    PGMSocket sendSocket;
    IPAddress bindAddr;

public:
    PGMSender(const char* multicastname)
        :sendSocket(SOCK_RDM)
    {
        // construct local address
        // this is just so we can bind to an interface
        bindAddr = IPV4Address(INADDR_ANY, 0, AF_INET);

        // Set any options we want before bind
        // Set more sending socket options here
        int res = sendSocket.setBroadcast();

        res = sendSocket.bindTo(bindAddr);



        // now connect to the multicast group
        // multiple channels can be created in the single multicast group
        // each port number represents a distinct channel
        addChannel(multicastname, 0);

        // At this point, we're ready to use send() and receive()
    }

    void addChannel(const char* ipname, uint16_t portnum)
    {
        IPAddress chanAddr = IPV4Address(ipname, portnum, AF_INET);

        sendSocket.connect(chanAddr);
    }

    std::tuple<int, int> send(void* buff, const int buffLen)
    {
        return  sendSocket.send((const char*)buff, buffLen, 0);
    }

};

class PGMReceiver
{
    PGMSocket receivingSocket;
    IPAddress boundAddress;

    PGMReceiver(const char* multicastname, uint16_t channel, int backlog = 10)
        :receivingSocket(SOCK_RDM)
    {
        boundAddress = IPV4Address(multicastname, channel, AF_INET);

        // Set socket options required before binding

        // bind to that local address
        receivingSocket.bindTo(boundAddress);

        // Set socket options required after binding

        // Setup for receiving messages
        receivingSocket.listen(backlog);

        // We're ready to start accepting and therefore
        // receiving data.  Call accept separately to get
        // the socket for receiving, so we don't block in the constructor
    }

    //
    // Recipient is responsible for closing socket
    // so we don't leak resources
    ASocket waitForConnection()
    {
        ASocket s = receivingSocket.accept();
        return s;
    }
};
