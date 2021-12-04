#pragma once


#include <map>
#include <string>
#include <string_view>
#include <cstring>
#include <algorithm>

#include "netstream.h"
#include "strutils.h"

class HttpMessage
{
	std::map<std::string, std::string> fHeaders;
	char lineBuffer[4096];
	int lineBufferSize = 4096;

protected:

public:
	std::map<std::string, std::string> & headers() { return fHeaders; }

	void appendHeader(std::string name, std::string value)
	{
		auto header = fHeaders.find(name);
		if (fHeaders.find(name) == fHeaders.end())
			return;

		fHeaders[name] = fHeaders[name] + value;
	}

	//
	// addHeader
	// append a ',' to an existing header
	//
	void addHeader(std::string name, std::string value)
	{
		fHeaders[name] = value;
	}

	std::string getHeader(std::string name)
	{
		auto value = fHeaders.find(name);
		if (value == fHeaders.end())
			return std::string();
		
		return value->second;
	}

	//
	// readHeaders
	// Read the http headers from a message
	// consume the expected blank line after the headers
	// as well
	void readHeaders(NetStream& ns)
	{
		std::string prevname;

		while (true) {
			auto [error, size] = ns.readOneLine(lineBuffer, lineBufferSize);

			if (error != 0) {
				break;
			}

			if (size == 0)
				break;

			// parse line to separate name from value
			auto ret = strchr(lineBuffer, ':');
			if (nullptr == ret)
			{
				// no colon, so append to previous header
				if (prevname != std::string())
					appendHeader(prevname, lineBuffer);
			}
			else
			{
				// We have a name separating a value with a colon
				int len = ret - lineBuffer;
				std::string name(lineBuffer, len);
				ret++;
				std::string value(ret);
				
				// turn the name field into lowercase while
				// trimming it at the same time
				strutils::trim(name);
				strutils::tolower(name);

				prevname = name;

				// trim leading and trailing whitespace from value
				strutils::trim(value);

				addHeader(name, value);
			}
		}
	}
};