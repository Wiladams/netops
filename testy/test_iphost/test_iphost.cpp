//
// test_iphost
//  Test using the IPHost and IPAddress structures
// Just get host information about a few hosts, and print
// the various addresses we find
//
#include "netapp.h"
#include <vector>

void onRun()
{
	std::vector<IPHost> hosts{
		IPHost("localhost", "80"),
		IPHost("adafruit.com", "80"),
		IPHost("facebook.com", "80"),
		IPHost("google.com", "80"),
		IPHost("increment.com", "80"),
		IPHost("kraken.com", "80"),
		IPHost("microsoft.com", "80"),
		IPHost("pnas.org", "80"),
		IPHost("remix.run","80"),
		IPHost("ucsd.edu","80"),
		IPHost("unsafeperform.io", "80"),
		IPHost("ycombinator.com", "80"),
		IPHost("ycombinator.com", "443"),
		IPHost("youtube.com", "80"),
	};



	for (int i = 0; i < hosts.size(); i++)
	{
		auto host = hosts[i];
		printf("Host Name: %s\n", host.getName());
		printf("  Aliases: %d\n", host.numberOfAddresses());
		for (int i = 0; i < host.numberOfAddresses(); i++)
		{
			char buff[512];
			int buffLen = 512;

			IPAddress anAddr = host.getAddress(i);
			int res = anAddr.toString(buff, buffLen);
			buff[res - 1] = '\0';

			printf("         : %s\n", buff);
		}
	}

}