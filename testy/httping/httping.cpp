/*
    In this example, we want to connect to various
    servers on port 80, and see what kind of response they
    return when we ask for the '/' resource.

    We connect to all the IP addresses reported by the server
    and not just the first one on the list.
*/

#include "netapp.h"
#include "tcpclient.h"
#include "netstream.h"
#include "httpmessage.h"

#include <cstdio>
#include <cstring>
#include <iostream>



const char* sites[] = {
    "Microsoft.com",
    "facebook.com",
    "google.com",
    "amazon.com",
    "netflix.com",
    "msn.com",
    "adafruit.com",
    "news.ycombinator.com"
};

/*
    Connect to all the host IP addresses we get
    and send a simple HTTP GET to see what comes back
*/
bool pingHttp(const char* hostname)
{
    static const int recvSize = 1024 * 64; // 64k 
    char response[recvSize + 1];

    char request[512];
    sprintf_s(request, "GET / HTTP/1.1\r\n"
        "Host: %s\r\n"
        "\r\n", hostname);

    const char* portname = "80";

    IPHost host(hostname, portname);

    if (!host.isValid()) {
        printf("Could not find host: %s\n", hostname);
        return false;
    }

    // Go through each of the host addresses
    // trying to connect to each one in turn
    printf("\n== HOST BEGIN (%s)==\n", hostname);
    for (int idx = 0; idx < host.numberOfAddresses(); idx++)
    {
        // Create a socket for the given address
        auto addr = host.getAddress(idx);
        TcpClient s(addr);
        NetStream ns(s.socket());

        // convert address to string form so we can print it out
        char addressBuff[256] = { 0 };
        int addressBuffLen = 255;
        int strLen = addr.toString(addressBuff, addressBuffLen);
        if (strLen > 0) {
            addressBuff[strLen] = 0;
        }

        if (!s.isValid()) {
            printf("Invalid TcpClient: %d\n", s.getLastError());

            // just move onto the next one
            continue;
        }

        int retCode = s.send(request, strlen(request), 0);

        if (retCode == SOCKET_ERROR) {
            printf("SOCKET SEND ERROR: %d\n", s.getLastError());
            continue;
        }

        printf("\n== RESPONSE BEGIN [%s]==\n", addressBuff);

        // Reading the HTTP response
        // Read the first line
        HttpMessage hmsg;

        auto [error, size] = ns.readOneLine(response, recvSize);
        
        if (error)
            continue;

        printf("%s\n", response);

        // Read the headers
        hmsg.readHeaders(ns);

        // print out headers
        auto it = hmsg.headers().begin();
        while (it != hmsg.headers().end())
        {
            std::cout << it->first << " : " << it->second << std::endl;
            it++;
        }
 
        printf("\n== RESPONSE END ==\n");

        // close the socket
        s.close();
    }
    printf("\n== HOST END (%s)==\n", hostname);

    return true;
}



void onRun()
{
    int nSites = sizeof(sites) / sizeof(char*);

    printf("SCAN BEGIN[%d]\n", nSites);

    for (int idx = 0; idx < nSites; idx++)
    {
        bool success = pingHttp(sites[idx]);
        if (!success) {
            printf("FAILED: %s\n", sites[idx]);
            continue;
        }
    }
    printf("\n== SCAN END ==\n");
}

