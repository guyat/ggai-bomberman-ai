#include "core/sbr2_screen_capture.h"

#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

bool capture_screen(SBR2Image &out_image)
{
#ifdef _WIN32
    HDC screen_dc = GetDC(nullptr);
    if (!screen_dc)
    {
        std::cout << "[capture] GetDC failed" << std::endl;
        return false;
    }

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HDC memory_dc = CreateCompatibleDC(screen_dc);
    if (!memory_dc)
    {
        ReleaseDC(nullptr, screen_dc);
        std::cout << "[capture] CreateCompatibleDC failed" << std::endl;
        return false;
    }

    HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, width, height);
    if (!bitmap)
    {
        DeleteDC(memory_dc);
        ReleaseDC(nullptr, screen_dc);
        std::cout << "[capture] CreateCompatibleBitmap failed" << std::endl;
        return false;
    }

    HGDIOBJ old_obj = SelectObject(memory_dc, bitmap);

    if (!BitBlt(memory_dc, 0, 0, width, height, screen_dc, 0, 0, SRCCOPY))
    {
        SelectObject(memory_dc, old_obj);
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        ReleaseDC(nullptr, screen_dc);
        std::cout << "[capture] BitBlt failed" << std::endl;
        return false;
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    out_image.width = width;
    out_image.height = height;
    out_image.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    if (!GetDIBits(
            memory_dc,
            bitmap,
            0,
            static_cast<UINT>(height),
            out_image.pixels.data(),
            &bmi,
            DIB_RGB_COLORS))
    {
        out_image.width = 0;
        out_image.height = 0;
        out_image.pixels.clear();

        SelectObject(memory_dc, old_obj);
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        ReleaseDC(nullptr, screen_dc);
        std::cout << "[capture] GetDIBits failed" << std::endl;
        return false;
    }

    SelectObject(memory_dc, old_obj);
    DeleteObject(bitmap);
    DeleteDC(memory_dc);
    ReleaseDC(nullptr, screen_dc);

    static int counter = 0;
    if (counter % 60 == 0)
    {
        std::cout << "[capture] ok "
                  << out_image.width << "x" << out_image.height
                  << " pixels=" << out_image.pixels.size()
                  << std::endl;
    }
    counter++;

    return true;
#else
    out_image.width = 0;
    out_image.height = 0;
    out_image.pixels.clear();
    std::cout << "[capture] Windows only" << std::endl;
    return false;
#endif
}