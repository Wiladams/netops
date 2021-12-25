#include "netapp.h"
#include <memory>

using SocketTopic = Topic<ASocket>;


SocketTopic acceptClientTopic;

//void handleAcceptClient(acceptClientTopic::)
void onLoad()
{
	// Do whatever we want before the 
	// app runs
}

ASocket createServer()
{
	ASocket srvsock(SOCK_STREAM, AF_INET,  IPPROTO_TCP, false);

	//srvsock.setNoDelay();
	srvsock.setReuseAddress();


	// create an address that we can bind to
	sockaddr_in addrin;
	addrin.sin_family = AF_INET;
	addrin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrin.sin_port = htons(5050);
	IPAddress addr((sockaddr *)&addrin, sizeof(addrin));

	srvsock.bindTo(addr);

	// turn the socket into a 'server' by telling it to listen
	srvsock.listen(100);

	// Now we want to start listening
	return srvsock;
}

// Run a continuous loop, connecting and accepting clients
void onRun()
{
	ASocket srv = createServer();

	while (true)
	{
		// accept a connection
		ASocket aClient = srv.accept();

		// emit a signal there is a new connection
	}
}