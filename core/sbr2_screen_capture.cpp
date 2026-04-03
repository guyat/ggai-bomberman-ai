#include "core/sbr2_screen_capture.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cwctype>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{

#pragma pack(push, 1)
    struct BMPFileHeader
    {
        std::uint16_t bfType = 0x4D42; // 'BM'
        std::uint32_t bfSize = 0;
        std::uint16_t bfReserved1 = 0;
        std::uint16_t bfReserved2 = 0;
        std::uint32_t bfOffBits = 0;
    };

    struct BMPInfoHeader
    {
        std::uint32_t biSize = 40;
        std::int32_t biWidth = 0;
        std::int32_t biHeight = 0;
        std::uint16_t biPlanes = 1;
        std::uint16_t biBitCount = 32;
        std::uint32_t biCompression = 0; // BI_RGB
        std::uint32_t biSizeImage = 0;
        std::int32_t biXPelsPerMeter = 0;
        std::int32_t biYPelsPerMeter = 0;
        std::uint32_t biClrUsed = 0;
        std::uint32_t biClrImportant = 0;
    };
#pragma pack(pop)

#ifdef _WIN32

    struct FindWindowByTitleData
    {
        std::wstring needle;
        HWND found = nullptr;
    };

    bool contains_substring_case_insensitive(const std::wstring &text,
                                             const std::wstring &needle)
    {
        if (needle.empty())
        {
            return true;
        }

        std::wstring lower_text = text;
        std::wstring lower_needle = needle;

        for (wchar_t &ch : lower_text)
        {
            ch = static_cast<wchar_t>(towlower(ch));
        }
        for (wchar_t &ch : lower_needle)
        {
            ch = static_cast<wchar_t>(towlower(ch));
        }

        return lower_text.find(lower_needle) != std::wstring::npos;
    }

    BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lparam)
    {
        auto *data = reinterpret_cast<FindWindowByTitleData *>(lparam);
        if (!data)
        {
            return TRUE;
        }

        if (!IsWindowVisible(hwnd))
        {
            return TRUE;
        }

        int length = GetWindowTextLengthW(hwnd);
        if (length <= 0)
        {
            return TRUE;
        }

        std::wstring title;
        title.resize(static_cast<size_t>(length));
        GetWindowTextW(hwnd, &title[0], length + 1);

        if (contains_substring_case_insensitive(title, data->needle))
        {
            data->found = hwnd;
            return FALSE;
        }

        return TRUE;
    }

    HWND find_window_by_title_substring(const std::wstring &needle)
    {
        FindWindowByTitleData data;
        data.needle = needle;
        EnumWindows(enum_windows_proc, reinterpret_cast<LPARAM>(&data));
        return data.found;
    }

    bool get_window_client_rect_on_screen(HWND hwnd, RECT &out_rect)
    {
        RECT client_rect{};
        if (!GetClientRect(hwnd, &client_rect))
        {
            return false;
        }

        POINT top_left{client_rect.left, client_rect.top};
        POINT bottom_right{client_rect.right, client_rect.bottom};

        if (!ClientToScreen(hwnd, &top_left))
        {
            return false;
        }
        if (!ClientToScreen(hwnd, &bottom_right))
        {
            return false;
        }

        out_rect.left = top_left.x;
        out_rect.top = top_left.y;
        out_rect.right = bottom_right.x;
        out_rect.bottom = bottom_right.y;

        return (out_rect.right > out_rect.left && out_rect.bottom > out_rect.top);
    }

    bool capture_rect_from_desktop(int src_x, int src_y, int width, int height, SBR2Image &out_image)
    {
        HDC screen_dc = GetDC(nullptr);
        if (!screen_dc)
        {
            return false;
        }

        HDC memory_dc = CreateCompatibleDC(screen_dc);
        if (!memory_dc)
        {
            ReleaseDC(nullptr, screen_dc);
            return false;
        }

        HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, width, height);
        if (!bitmap)
        {
            DeleteDC(memory_dc);
            ReleaseDC(nullptr, screen_dc);
            return false;
        }

        HGDIOBJ old_obj = SelectObject(memory_dc, bitmap);

        BOOL ok = BitBlt(memory_dc, 0, 0, width, height, screen_dc, src_x, src_y, SRCCOPY);

        if (!ok)
        {
            SelectObject(memory_dc, old_obj);
            DeleteObject(bitmap);
            DeleteDC(memory_dc);
            ReleaseDC(nullptr, screen_dc);
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
            return false;
        }

        SelectObject(memory_dc, old_obj);
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        ReleaseDC(nullptr, screen_dc);

        return true;
    }

#endif // _WIN32

} // namespace

bool capture_screen(SBR2Image &out_image)
{
#ifdef _WIN32
    static int counter = 0;

    std::string mode = "desktop";
    int src_x = 0;
    int src_y = 0;

    HWND sbr2_hwnd = find_window_by_title_substring(L"SUPER BOMBERMAN R 2");
    if (sbr2_hwnd)
    {
        RECT rect{};
        if (get_window_client_rect_on_screen(sbr2_hwnd, rect))
        {
            src_x = rect.left;
            src_y = rect.top;
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            if (capture_rect_from_desktop(src_x, src_y, width, height, out_image))
            {
                mode = "sbr2_window_blt";
            }
        }
    }

    if (out_image.width <= 0 || out_image.height <= 0 || out_image.pixels.empty())
    {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        src_x = 0;
        src_y = 0;

        if (!capture_rect_from_desktop(src_x, src_y, width, height, out_image))
        {
            std::cout << "[capture] failed" << std::endl;
            return false;
        }
        mode = "desktop";
    }

    if (counter % 60 == 0)
    {
        std::cout << "[capture] ok "
                  << out_image.width << "x" << out_image.height
                  << " pixels=" << out_image.pixels.size()
                  << " mode=" << mode
                  << " src=(" << src_x << "," << src_y << ")"
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

bool save_image_as_bmp(const SBR2Image &image, const std::string &path)
{
    if (image.width <= 0 || image.height <= 0)
    {
        std::cout << "[bmp] invalid image size" << std::endl;
        return false;
    }

    const std::size_t expected_size =
        static_cast<std::size_t>(image.width) *
        static_cast<std::size_t>(image.height) * 4;

    if (image.pixels.size() != expected_size)
    {
        std::cout << "[bmp] invalid pixel buffer size" << std::endl;
        return false;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    info_header.biWidth = image.width;
    info_header.biHeight = -image.height;
    info_header.biSizeImage = static_cast<std::uint32_t>(expected_size);

    file_header.bfOffBits =
        static_cast<std::uint32_t>(sizeof(BMPFileHeader) + sizeof(BMPInfoHeader));
    file_header.bfSize = file_header.bfOffBits + info_header.biSizeImage;

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
    {
        std::cout << "[bmp] failed to open file: " << path << std::endl;
        return false;
    }

    ofs.write(reinterpret_cast<const char *>(&file_header), sizeof(file_header));
    ofs.write(reinterpret_cast<const char *>(&info_header), sizeof(info_header));
    ofs.write(reinterpret_cast<const char *>(image.pixels.data()),
              static_cast<std::streamsize>(image.pixels.size()));

    if (!ofs)
    {
        std::cout << "[bmp] failed to write file: " << path << std::endl;
        return false;
    }

    std::cout << "[bmp] saved: " << path << std::endl;
    return true;
}