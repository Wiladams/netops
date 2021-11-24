#pragma once

// Make sure the compiler picks up on the library we need
#pragma comment(lib, "ws2_32.lib")

#define WINSOCK_DEPRECATED_NO_WARNINGS 1

/*
References:
https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedInternet3a.html
*/
#include <cstring>
#include <cstdio>
#include <vector>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h>


struct IPAddress {
private:
    IPAddress() = delete;   // don't allow default constructor

public:
    //struct sockaddr_in* sockaddr_ipv4;
    //LPSOCKADDR sockaddr_ip;

    struct sockaddr fAddress;
    int fAddressLength;

    // Copy constructor
    IPAddress(const IPAddress& other)
        :fAddressLength(0)
    {
        ZeroMemory(&fAddress, sizeof(fAddress));

        memcpy(&fAddress, &other.fAddress, sizeof(fAddress));
        fAddressLength = other.fAddressLength;
    }

    // Construct from traditional sockaddr and length
    IPAddress(const struct sockaddr *addr, const int addrLen)
        :fAddressLength(0)
    {
        ZeroMemory(&fAddress, sizeof(fAddress));

        if (addrLen > sizeof(fAddress))
            return; 

        memcpy(&fAddress, addr, addrLen);
        fAddressLength = addrLen;
    }

    ~IPAddress()
    {
        //if (fAddress != nullptr) {
        //    free(fAddress);
        //}
    }

    IPAddress& operator=(const IPAddress& rhs)
    {
        memcpy(&fAddress, &rhs.fAddress, rhs.fAddressLength);
        fAddressLength = rhs.fAddressLength;

        return *this;
    }

    // typecast to struct sockaddr
    operator struct sockaddr& () { return fAddress; }

    int toString(char *addressBuff, int addressBuffLen)
    {
        DWORD consumedLength = addressBuffLen;
        int res = WSAAddressToStringA(&fAddress, fAddressLength, nullptr,addressBuff, &consumedLength);
        
        return consumedLength;
    }

};

class IPHost {
    bool fIsValid = false;
    char fHostName[1024];
    std::vector<IPAddress> fAddresses;
    std::vector<char *> fAliases;

    bool init(const char* hostname, const char* portname, int socktype, int family, int flags)
    {
        int err;
        addrinfo hints;
        struct addrinfo* ppResult;

        // need to zero out some members before using
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = family;
        hints.ai_socktype = socktype;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = flags; // AI_CANONNAME;    // hostname is a canonical name



        err = getaddrinfo(hostname, portname, &hints, &ppResult);

        if (err != 0) {
            printf("IPHost.create(), getaddrinfo ERROR: %d\n", WSAGetLastError());
            return false;
        }


        // Add initial name and address
        this->setName(ppResult->ai_canonname);
        this->addAddress(ppResult->ai_addr, ppResult->ai_addrlen);

        // go through the rest of the entries
        // and add their information as well
        struct addrinfo* sentinel = ppResult->ai_next;
        while (sentinel != nullptr) {
            if (sentinel->ai_canonname != nullptr)
            {
                //printf("Alias: %s\n", sentinel->ai_canonname);
                this->addAlias(sentinel->ai_canonname);
            }

            this->addAddress(sentinel->ai_addr, sentinel->ai_addrlen);
            sentinel = sentinel->ai_next;
        }

        // free the addrinfos structure
        freeaddrinfo(ppResult);
    }

public:
    IPHost(const char* hostname, const char* portname, int socktype = SOCK_STREAM, int family = AF_INET, int flags = AI_CANONNAME)
    {
        init(hostname, portname, socktype, family, flags);
    }

    void setName(const char *name)
    {
        if (name == nullptr) {
            memset(fHostName, 0, sizeof(fHostName));
        }

        strncpy_s(fHostName, sizeof(fHostName), name, sizeof(fHostName));
    }
    const char * getName() const {return fHostName;}

    // addAddress
    bool addAddress(const struct sockaddr *addr, const int addrlen)
    {
        IPAddress anAddress(addr, addrlen);
        fAddresses.push_back(anAddress);
        
        return true;
    }

    int numberOfAddresses() {return fAddresses.size();}

    IPAddress & getAddress(const int idx = 0)
    {
        return fAddresses[idx];
    }

    // addAlias
    bool addAlias(const char *alias)
    {
        return false;
    }
};

//
// ASocket
// Basic representation of a networking socket.
// Sockets can support many protocols.  Mostly they are associated
// the TCP or UDP, but those are not the only protocols.  The intention
// of this class is to represent the raw socket capabilities, without
// getting bogged down in the behavior of any given protocol.
//
// Example:  We don't put TCP specific options in here, such as the NODELAY
// because that's not appropriate for all sockets.  Istead, protocol specific
// socket classes should be implemented that have more specific stuff.
//
// The core option setting calls are here in the base class, as that is convenient, 
// but specific protocol specifics should go in a sub-class.

//
class ASocket {
protected:
    SOCKET fSocket;

    // Retrieve a pointer to an extension function
    // These functions are implementation specific
    // so there's no other way to gain access to them without
    // going through ws2_32
    static void* getExtensionFunctionPointer(GUID &funcguid)
    {
        ASocket s;
        int cbInBuffer = sizeof(GUID);
        void* lpvOutBuffer = nullptr;
        int cbOutBuffer=sizeof(lpvOutBuffer);
        DWORD lpcbBytesReturned = 0;
        LPWSAOVERLAPPED lpOverlapped = nullptr;
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine  = nullptr;

        int res = WSAIoctl(s.fSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            (void *)&funcguid, cbInBuffer, 
            lpvOutBuffer, cbOutBuffer,
            &lpcbBytesReturned,
            lpOverlapped,
            lpCompletionRoutine);

        if (res != 0)
            return nullptr;

        return lpvOutBuffer;
    }

    // Extended Accept
    static int AcceptEx(ASocket listenSocket, ASocket acceptedSocket)
    {
        static LPFN_ACCEPTEX CAcceptEx = nullptr;

        if (nullptr == CAcceptEx)
        {
            GUID g = WSAID_ACCEPTEX;
            CAcceptEx = (LPFN_ACCEPTEX)getExtensionFunctionPointer(g);
        }

        // Call extension function
        //lpOutputBuffer
        DWORD    lpBytesReceived = 0;
        void *lpOutputBuffer= nullptr;
        DWORD dwReceiveDataLength=0;
        DWORD dwLocalAddressLength=0;
        DWORD dwRemoteAddressLength=0;
        DWORD lpdwBytesReceived;
        LPOVERLAPPED lpOverlapped=nullptr;

        int res = CAcceptEx(listenSocket.fSocket, acceptedSocket.fSocket,
            lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength,
            dwRemoteAddressLength, &lpdwBytesReceived,
            lpOverlapped);

        return res;
    }
    
    static int ConnectEx(ASocket s, IPAddress address)
    {
        LPFN_CONNECTEX CConnectEx = nullptr;

        if (CConnectEx == nullptr) {
            GUID g = WSAID_CONNECTEX;
            CConnectEx = (LPFN_CONNECTEX)getExtensionFunctionPointer(g);
        }

        if (nullptr == CConnectEx)
            return -1;

        void* lpSendBuffer = nullptr;
        DWORD dwSendDataLength = 0;
        DWORD lpdwBytesSent = 0;
        LPOVERLAPPED lpOverlapped = nullptr;

        CConnectEx(s.fSocket, &address.fAddress, address.fAddressLength,
            lpSendBuffer, dwSendDataLength,
            &lpdwBytesSent, lpOverlapped);
    }

    static int DisconnectEx(ASocket s)
    {
        static LPFN_DISCONNECTEX CDisconnectEx = nullptr;

        if (nullptr == DisconnectEx)
        {
            GUID g = WSAID_DISCONNECTEX;
            CDisconnectEx = (LPFN_DISCONNECTEX)getExtensionFunctionPointer(g);
        }

        LPOVERLAPPED lpOverlapped = nullptr;
        DWORD dwFlags = 0;
        DWORD dwReserved = 0;
        int res = CDisconnectEx(s.fSocket, lpOverlapped, dwFlags, dwReserved);

        return res;
    }

private:
    bool fIsValid;
    int fLastError;
    bool fAutoClose;

public:
    // Construct with an existing native socket
    ASocket(SOCKET s, const bool autoclose)
        : fSocket(s),
        fLastError(0), 
        fAutoClose(autoclose)
    {
        fIsValid = (s != INVALID_SOCKET);
    }
    
    // Default constructor will initially be invalid
    ASocket()
        : ASocket(INVALID_SOCKET, false)
    {
    }

    // Construct a particular kind of socket
    ASocket(int family, int socktype, int protocol)
        : fIsValid(false)
    {
        fSocket = WSASocketA(family, socktype, protocol, nullptr, 0, 0);
        init(fSocket);
    }

    // There should be a flag to autoclose
    // otherwise, if you create one of these on the 
    // stack, or copy it, it will close
    virtual ~ASocket() {
        if (fAutoClose) {
            forceClose();
        }
    }

    void init(SOCKET sock, bool autoclose=false)
    {
        fIsValid = false;
        fSocket = sock;

        if (fSocket == INVALID_SOCKET) {
            fLastError = WSAGetLastError();
            return;
        }

        fIsValid = true;
    }

    bool isValid() const {return fIsValid;}
    //int getLastError() const {return fLastError;}

    // Socket parameters
    int ioctl(long cmd, unsigned long * argp)
    {
        int res = ioctlsocket(fSocket, cmd, argp);
        if (res == 0)
            return 0;

        res = WSAGetLastError();
        return res;
    }

    bool getSocketOption(int level, int optname, char *optval, int &optlen)
    {
        int res = getsockopt(fSocket, level, optname, optval, &optlen);
        
        if (res == 0)
            return true;

        fLastError = WSAGetLastError();
        return false;
    }

    bool setSocketOption(int level, int optname, const char* optval, int optlen)
    {
        int res = setsockopt(fSocket, level, optname, optval, optlen);
        if (res == 0)
            return true;

        fLastError = WSAGetLastError();
        return false;
    }

    // Convenient commands used with ioctl

    //
    // Async implies the usage of iocompletion ports
    bool setAsync()
    {
        unsigned long param = 1;
        return (0 == ioctl(FIOASYNC, &param));
    }

    bool setNoAsync()
    {
        unsigned long param = 0;
        return (0 == ioctl(FIOASYNC, &param));
    }

    // non-blocking just means you'll have to come
    // back later to see if there's any data
    int getLastError()
    {
        int optValue;
        int optSize = sizeof(int);
        int size = sizeof(int);

        bool success = getSocketOption(SOL_SOCKET, SO_ERROR, (char *)&optValue, optSize);

        if (success)
            return optValue;

        return 0;
    }

    bool setBlocking()
    {
        unsigned long param = 0;
        return (0 == ioctl(FIONBIO, &param));
    }

    bool setNoBlocking()
    {
        unsigned long param = 1;
        return (0 == ioctl(FIONBIO, &param));
    }

    // Convenient Commands used with socket options
    bool setExclusiveAddress()
    {
        int oneInt = 1;
        return setSocketOption(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&oneInt, sizeof(oneInt));
    }

    bool setNoExclusiveAddress()
    {
        int oneInt = 0;
        return setSocketOption(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&oneInt, sizeof(oneInt));
    }

    bool setReuseAddress()
    {
        int oneInt = 1;
        return setSocketOption(SOL_SOCKET, SO_REUSEADDR, (char*)&oneInt, sizeof(oneInt));
    }

    bool setNoReuseAddress()
    {
        int oneInt = 0;
        return setSocketOption(SOL_SOCKET, SO_REUSEADDR, (char*)&oneInt, sizeof(oneInt));
    }



    // Standard Interface
    ASocket accept()
    {
        //struct sockaddr clientAddr;
        //int clientAddrLen=0;
        //int res = ::accept(fSocket,&clientAddr,&clientAddrLen);
        int res = ::accept(fSocket,nullptr,nullptr);

        if (res == INVALID_SOCKET) {
            fLastError = WSAGetLastError();
            return ASocket();
        }

        return ASocket(res, false);
    }

    int bindTo(const sockaddr *addr, const int addrLen)
    {
        return ::bind(fSocket, addr, addrLen);
    }

    // Closing a socket should include a shutdown
    // so the socket isn't lingering
    bool forceClose() {
        int result = ::closesocket(fSocket);
        if (result != 0) {
            fLastError = WSAGetLastError();
            return false;
        }

        return true;
    }

    //
    // setup the socket to accept connections
    //
    bool listen(int backlog = 5)
    {
        int result = ::listen(fSocket, backlog);
        if (result != 0) {
            fLastError = WSAGetLastError();
            return false;
        }

        return true;
    }

    //
    // shutdown - disable sends or receives on a socket
    // you might do this before closing a socket
    // how == SD_SEND, SD_RECEIVE, SD_BOTH
    //
    bool shutdown(int how = SD_SEND) {
        int res = ::shutdown(fSocket, how);
        if (res != 0) {
            fLastError = WSAGetLastError();
            return false;
        }

        return true;
    }






    // Send a chunk of memory
    // return the number of octets sent
    int send(const char* buff, const int bufflen, const int f = 0)
    {
        int len = bufflen;
        int flags = f;

        return ::send(fSocket, buff, len, flags);
    }

    // receive a chunk of memmory
    // return number of octets received
    int recv(char* buff, int len, int flags = 0)
    {
        return ::recv(fSocket, buff, len, flags);
    }


    // Send to a specific address
    // The address was specified when we 
    // created the socket
    int sendTo(const struct sockaddr *addrTo, int addrToLen, const char *buff, const int bufflen)
    {
        return ::sendto(fSocket, buff, bufflen, 0, addrTo, addrToLen);
    }

    int receiveFrom(struct sockaddr *addrFrom, int *addrFromLen, char *buff, int bufflen)
    {
        return ::recvfrom(fSocket, buff, bufflen, 0, addrFrom, addrFromLen);
    }




};


//
// WinExSocket
// A socket that uses Windows Extensions
//
class WinExSocket : public ASocket
{
private:


        // cleanest possible way to close down a socket
        // either client or server side
    bool closeDown()
    {
        //disconnect();
        return forceClose();
    }
};

class TcpSocket : public ASocket
{
public :
    // Nagle's algorithm, uses some delay to ensure we don't send a ton
    // of tiny packets.  Packets are accumulated until some threshold, then
    // sent.
    bool setDelay()
    {
        int oneInt = 0;
        return setSocketOption(IPPROTO_TCP, TCP_NODELAY, (char*)&oneInt, sizeof(oneInt));
    }

    // Turn off delayed send
    bool setNoDelay()
    {
        int oneInt = 1;
        return setSocketOption(IPPROTO_TCP, TCP_NODELAY, (char*)&oneInt, sizeof(oneInt));
    }
};