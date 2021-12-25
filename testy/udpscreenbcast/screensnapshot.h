#pragma once
// ScreenSnapshot
//
// Take a snapshot of a portion of the screen and hold
// it in a DIBSection

//
// When constructed, a single snapshot is taken.
// every time you want a new snapshot, just call 'next()'
// This is great for doing a live screen capture
//
//    ScreenSnapshot ss(x,y, width, height);
//
//    References:
//    https://www.codeproject.com/articles/5051/various-methods-for-capturing-the-screen
//    https://stackoverflow.com/questions/5069104/fastest-method-of-screen-capturing-on-windows
//  https://github.com/bmharper/WindowsDesktopDuplicationSample
//

#include <windows.h>
#include <cstdint>

class ScreenSnapshot
{
    HDC fScreenDC;  // Device Context for the screen
    BITMAPINFO fBMInfo{ 0 };
    HBITMAP fDIBHandle = nullptr;
    //HGDIOBJ fOriginDIBHandle = nullptr;
    HDC     fBitmapDC = nullptr;
    void* fData = nullptr;       // A pointer to the data
    size_t fDataSize = 0;       // How much data is allocated
    size_t fBytesPerRow = 0;        // Row stride

    // A couple of constants
    static const int fChannels = 4;
    static const int bitsPerPixel = 32;
    static const int alignment = 4;

    // which location on the screen are we capturing
    int fOriginX;
    int fOriginY;
    int fWidth;
    int fHeight;

public:
    ScreenSnapshot(int x, int y, int awidth, int aheight)
        : fOriginX(x), fOriginY(y),
        fWidth(awidth), fHeight(aheight)
    {
        // create a device context for the display
        //fScreenDC = CreateDCA("DISPLAY", nullptr, nullptr, nullptr);
        fScreenDC = GetDC(nullptr);

        fBytesPerRow = awidth * fChannels;

        fBMInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        fBMInfo.bmiHeader.biWidth = awidth;
        fBMInfo.bmiHeader.biHeight = -(LONG)aheight;	// top-down DIB Section
        fBMInfo.bmiHeader.biPlanes = 1;
        fBMInfo.bmiHeader.biBitCount = bitsPerPixel;
        fBMInfo.bmiHeader.biSizeImage = fBytesPerRow * aheight;
        fBMInfo.bmiHeader.biClrImportant = 0;
        fBMInfo.bmiHeader.biClrUsed = 0;
        fBMInfo.bmiHeader.biCompression = BI_RGB;
        fDataSize = fBMInfo.bmiHeader.biSizeImage;

        // We'll create a DIBSection so we have an actual backing
        // storage for the context to draw into
        // BUGBUG - check for nullptr and fail if found
        fDIBHandle = ::CreateDIBSection(nullptr, &fBMInfo, DIB_RGB_COLORS, &fData, nullptr, 0);


        // Create a GDI Device Context
        fBitmapDC = ::CreateCompatibleDC(nullptr);

        // select the DIBSection into the memory context so we can 
        // peform operations with it
        //fOriginDIBHandle = ::SelectObject(fBitmapDC, fDIBHandle);

        // take at least one snapshot
        next();
    }

    inline int originX() { return fOriginX; }
    inline int originY() { return fOriginY; }
    inline int width() { return fWidth; }
    inline int height() { return fHeight; }
    inline int channels() { return fChannels; }

    inline BITMAPINFO& getBitmapInfo() { return fBMInfo; }
    inline HDC getDC() { return fBitmapDC; }
    inline void* getData() { return fData; }
    inline void* getPixelPointer(const int x, const int y) { return &((uint32_t*)fData)[(y * fWidth) + x]; }
    inline size_t bytesPerRow() const { return fBytesPerRow; }

    // take a snapshot of current screen
    bool next()
    {
        auto res = BitBlt(getDC(), 0, 0, width(), height(), fScreenDC, originX(), originY(), SRCCOPY | CAPTUREBLT);
        return res != 0;
    }
};
