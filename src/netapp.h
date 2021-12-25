#pragma once

#pragma once

/*
	The primary benefit of the apphost.h/appmain.cpp combination is to turn
	windows specifics into simpler concepts.

	Typical Windows messages, such as mouse and keyboard, and turned into
	topics, that other applications can subscribe to.

	A 'main()' is provided, so the application can implement a 'setup()'
	instead, and not have to worry about whether it is a console or Window
	target.

	All other aspects of the application are up to the application environment,
	but at least all the Win32 specific stuff is wrapped up.
*/

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <windows.h>

#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mmsystem.h>

#include <memory>
#include <deque>
#include <functional>
#include <stdio.h>
#include <string>


// EXPORT allows us to declare a function as exported
// this will make it possible to then lookup that 
// function in the .dll using GetProcAddress
// So, if that's needed, just put EXPORT at the front
// of a declaration.
#define APP_EXPORT		__declspec(dllexport)


#define APP_INLINE  static _forceinline
#define APP_EXTERN  extern


#include "pubsub.h"
#include "network.h"

/*
	The interface the implementing application sees is 'C'.  This
	makes binding much easier for scripting languages, and allows
	usage across different languages, without having to worry about
	the C++ name mangling that would occur otherwise.
*/

#ifdef __cplusplus
extern "C" {
#endif
	typedef void (*VOIDROUTINE)();
	typedef void (*PFNDOUBLE1)(const double param);

	// Miscellaneous globals
	APP_EXPORT extern int gargc;
	APP_EXPORT extern char** gargv;



	// The various 'onxxx' routines are meant to be implemented by
	// application environment code.  If they are implemented
	// the ndt runtime will load them in and call them at appropriate times
	// if they are not implemented, they simply won't be called.
	APP_EXPORT void onLoad();	// upon loading application
	APP_EXPORT void onUnload();

	APP_EXPORT void onRun();	// called for the user's app


#ifdef __cplusplus
}
#endif

// Make Topic publishers available
using SignalEventTopic = Topic<intptr_t>;


// Doing C++ pub/sub



APP_EXPORT void subscribe(SignalEventTopic::Subscriber s);

