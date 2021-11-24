
/*
    netapp
    The primary function of this file is to provide a ready made base
    for creating network based applications on the Windows operating system.

    This file provides the 'main()' as well as performing the required
    startup of the Windows networking stack.

    user application should implement 'onRun()' if they want anything
    to happen.
*/

#include <cstdio>
#include <array>
#include <iostream>
#include <memory>

#include "netapp.h"

// Some function signatures
// WinMSGObserver - Function signature for Win32 message handler
typedef LRESULT (*WinMSGObserver)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Application routines
// appmain looks for this routine in the compiled application
static VOIDROUTINE gOnloadHandler = nullptr;
static VOIDROUTINE gOnUnloadHandler = nullptr;
static VOIDROUTINE gOnRunHandler = nullptr;



// Topics applications can subscribe to
SignalEventTopic gSignalEventTopic;

// Miscellaneous globals
int gargc;
char **gargv;

/*
    Subscription routines
*/
// General signal subscription
void subscribe(SignalEventTopic::Subscriber s)
{
    gSignalEventTopic.subscribe(s);
}


//
// Look for the dynamic routines that will be used
// to setup client applications.
// Most notable is 'onLoad()' and 'onUnload'
//
void registerHandlers()
{
    // we're going to look within our own module
    // to find handler functions.  This is because the user's application should
    // be compiled with the application, so the exported functions should
    // be attainable using 'GetProcAddress()'

    HMODULE hInst = ::GetModuleHandleA(NULL);

    // Get the general app routines
    // onLoad()
    gOnloadHandler = (VOIDROUTINE)::GetProcAddress(hInst, "onLoad");
    gOnUnloadHandler = (VOIDROUTINE)::GetProcAddress(hInst, "onUnload");

    gOnRunHandler = (VOIDROUTINE)::GetProcAddress(hInst, "onRun");
}



// do any initialization that needs to occur 
// in the very beginning
bool prolog()
{
    // startup networking sub-system
    uint16_t version = MAKEWORD(2, 2);
    WSADATA lpWSAData;
    int res = ::WSAStartup(version, &lpWSAData);

    // Make sure we have all the event handlers connected
    registerHandlers();

    return true;
}

// Allow cleanup before exit
void epilog()
{
    if (gOnUnloadHandler != nullptr) {
        gOnUnloadHandler();
    }
}


/*
    The 'main()' function is in here to ensure that compiling
    this file will result in an executable program.

    Why on Earth are there two 'main' routines in here?
    the first one 'main()' will be called when the code is compiled
    as a CONSOLE subsystem.

    the second one 'WinMain' will be called when the code is compiled
    as a WINDOWS subsystem.  
    
    By having both, we kill two birds with one stone.
*/
int mainRun()
{
    if (!prolog()) {
        printf("error in prolog\n");
        return -1;
    }

    // call the application's 'onLoad()' if it exists
    if (gOnloadHandler != nullptr) {
        gOnloadHandler();
    }

    // If the application has implemented 'onRun()'
    // then call that
    if (gOnRunHandler) {
        gOnRunHandler();
    }

    // If we've gotten here, then the user's 'onRun()' has
    // finished, so we need to do some cleanup
    epilog();

    return 0;
}

int main(int argc, char **argv)
{
    gargc = argc;
    gargv = argv;
    
    return mainRun();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdLine, int nShowCmd)
{
    // BUGBUG, need to parse those command line arguments
    return mainRun();
}