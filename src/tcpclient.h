#pragma once

#include "network.h"


class TcpClient {
private:
    ASocket fSocket;
    IPAddress fAddress;
    bool    fIsValid=false;
    int fLastError=0;

public:

    // Create a TcpClient connected to a specific host
    static TcpClient create(const char* hostname, const char* portname, bool hostNumeric = false)
    {
        // Get address to host
        IPHost host(hostname, portname);

        if (!host.isValid())
        {
            printf("could not find host: %s\n", hostname);
            return TcpClient();
        }

        // Try to creat a socket based on the first address
        // from the host
        IPAddress addr = host.getAddress(0);
        return TcpClient(addr);
    }


    TcpClient()
        : fSocket()
        , fAddress()
        , fIsValid(false)
        , fLastError(0)
    {}


    TcpClient(IPAddress &address)
        :fSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, false)
        , fAddress(address)
        , fLastError(0)
        , fIsValid(false)
    {
        if (!fSocket.isValid()) {
            fLastError = fSocket.getLastError();
            return ;
        }

        // Try to connect, if failure
        // return immediately
        if (!connect()) {
            return;
        }
        
        int sendSize = fSocket.getSendBufferSize();
        int receiveSize = fSocket.getReceiveBufferSize();

        printf("TcpClient: send Size: %d\treceiveSize: %d\n", sendSize, receiveSize);

        fIsValid = true;
    }

    bool isValid() const {return fIsValid;}
    int getLastError() const {return fLastError;}
    ASocket& socket() { return fSocket; }

    bool close()
    {
        if (!fSocket.forceClose()) {
            fLastError = fSocket.getLastError();
            return false;
        } 
        return true;
    }

    bool connect()
    {
        int retCode = ::connect(fSocket.fSocket, &fAddress.fAddress, fAddress.fAddressLength);

        if (retCode != 0) {
            fLastError = WSAGetLastError();

            return false;
        }

        return true;
    }

    // Send the entirety of the buffer specified
    // deal with mundane errors.
    // Depending on buffer sizes, async, and blocking
    // you might be sending less than the buffer 
    // size each time, so you have to keep sending til
    // all the bytes have been sent, unless there's an 
    // error we can't recover from.
    // return number of bytes actually sent
    int send(const char *buff, const int buffLen, int flags = 0)
    {

        int bytesTransferred = 0;

        while (bytesTransferred < buffLen) {
            // do this send in a loop, because
            // it may not all get sent in one call
            auto [error, bytesTrans] = fSocket.send(buff, buffLen, flags);

            if (error != 0) {
                // depends on what kind of error we encounter
                // as to whether we should continue on
                // or just break out of the loop
                fLastError = error;
                break;
            }
            else {
                // increment number of bytes sent
                bytesTransferred += bytesTrans;
            }
        }

        return bytesTransferred;
    }

    //
    // Reading is a bit tricky.  You want to read enough,
    // but not necessarily stall waiting for stuff to arrive.
    // So, it really depends on how the underlying socket
    // is configured, whether we're non-blocking, async, buffer
    // size.  
    // And it also depends on the application.  If you're doing 
    // http parsing, you might just need enough to read a single
    // line, athough there is more available to be read.
    int receive(char *buff, int buffLen, int flags=0)
    {
        int bytesTransferred = 0;

        //while (bytesTransferred < buffLen) {
            auto [error, bytesRead] = fSocket.recv(buff, buffLen, flags);

            if (error != 0) {
                // depends on what kind of error we encounter
                // as to whether we should continue on
                // or just break out of the loop
                fLastError = error;
                //break;
            }
            else {
                // increment number of bytes sent
                bytesTransferred += bytesRead;
            }

        //}

        return bytesTransferred;
    }
};

