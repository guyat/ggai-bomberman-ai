#include "core/sbr2_virtual_pad.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <setupapi.h>
#include <objbase.h>
#endif

namespace
{

#ifdef _WIN32

    constexpr wchar_t kScpBusClassGuidString[] =
        L"{F679F562-3164-42CE-A4DB-E7DDBE723909}";

    constexpr DWORD kIoctlPlugIn = 0x2A4000;
    constexpr DWORD kIoctlUnplug = 0x2A4004;
    constexpr DWORD kIoctlReport = 0x2A400C;

    constexpr unsigned short kButtonNone = 0;
    constexpr unsigned short kButtonRight = 1u << 3;
    constexpr unsigned short kButtonLeft = 1u << 2;
    constexpr unsigned short kButtonUp = 1u << 0;
    constexpr unsigned short kButtonDown = 1u << 1;

    constexpr unsigned short kButtonUpLeft = kButtonUp | kButtonLeft;
    constexpr unsigned short kButtonUpRight = kButtonUp | kButtonRight;
    constexpr unsigned short kButtonDownLeft = kButtonDown | kButtonLeft;
    constexpr unsigned short kButtonDownRight = kButtonDown | kButtonRight;

    constexpr unsigned short kButtonA = 1u << 12;

    std::string get_last_error_message(const char *prefix)
    {
        DWORD error_code = GetLastError();

        LPSTR buffer = nullptr;
        DWORD size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&buffer),
            0,
            nullptr);

        std::ostringstream oss;
        oss << prefix << " (GetLastError=" << error_code << ")";

        if (size > 0 && buffer != nullptr)
        {
            oss << ": " << buffer;
        }

        if (buffer != nullptr)
        {
            LocalFree(buffer);
        }

        return oss.str();
    }

    void write_le32(std::uint8_t *dst, std::uint32_t value)
    {
        dst[0] = static_cast<std::uint8_t>(value & 0xFFu);
        dst[1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
        dst[2] = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
        dst[3] = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
    }

    std::array<std::uint8_t, 20> build_controller_report(unsigned short buttons)
    {
        std::array<std::uint8_t, 20> report{};
        report[0] = 0x00; // input report
        report[1] = 0x14; // 20 bytes
        report[2] = static_cast<std::uint8_t>(buttons & 0xFFu);
        report[3] = static_cast<std::uint8_t>((buttons >> 8) & 0xFFu);
        return report;
    }

    std::array<std::uint8_t, 28> build_full_report(
        int pad_index,
        const std::array<std::uint8_t, 20> &controller_report)
    {
        std::array<std::uint8_t, 28> full{};
        full[0] = 0x1C;
        write_le32(&full[4], static_cast<std::uint32_t>(pad_index));
        std::memcpy(&full[8], controller_report.data(), controller_report.size());
        return full;
    }

    bool find_scp_bus_device_path(std::wstring *out_path, std::string *out_error)
    {
        if (out_path == nullptr)
        {
            if (out_error != nullptr)
            {
                *out_error = "out_path is null";
            }
            return false;
        }

        GUID interface_guid{};
        HRESULT hr = CLSIDFromString(kScpBusClassGuidString, &interface_guid);
        if (FAILED(hr))
        {
            if (out_error != nullptr)
            {
                *out_error = "CLSIDFromString failed";
            }
            return false;
        }

        HDEVINFO device_info_set = SetupDiGetClassDevsW(
            &interface_guid,
            nullptr,
            nullptr,
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (device_info_set == INVALID_HANDLE_VALUE)
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message("SetupDiGetClassDevsW failed");
            }
            return false;
        }

        bool success = false;

        SP_DEVICE_INTERFACE_DATA interface_data{};
        interface_data.cbSize = sizeof(interface_data);

        if (!SetupDiEnumDeviceInterfaces(
                device_info_set,
                nullptr,
                &interface_guid,
                0,
                &interface_data))
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message(
                    "SetupDiEnumDeviceInterfaces failed");
            }
            SetupDiDestroyDeviceInfoList(device_info_set);
            return false;
        }

        DWORD required_size = 0;
        SetupDiGetDeviceInterfaceDetailW(
            device_info_set,
            &interface_data,
            nullptr,
            0,
            &required_size,
            nullptr);

        if (required_size == 0)
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message(
                    "SetupDiGetDeviceInterfaceDetailW size query failed");
            }
            SetupDiDestroyDeviceInfoList(device_info_set);
            return false;
        }

        std::vector<std::uint8_t> buffer(required_size, 0);
        auto *detail_data =
            reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W *>(buffer.data());
        detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        if (!SetupDiGetDeviceInterfaceDetailW(
                device_info_set,
                &interface_data,
                detail_data,
                required_size,
                nullptr,
                nullptr))
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message(
                    "SetupDiGetDeviceInterfaceDetailW failed");
            }
            SetupDiDestroyDeviceInfoList(device_info_set);
            return false;
        }

        *out_path = detail_data->DevicePath;
        success = true;

        SetupDiDestroyDeviceInfoList(device_info_set);
        return success;
    }

    HANDLE open_scp_bus_handle(
        const std::wstring &device_path,
        std::string *out_error)
    {
        HANDLE handle = CreateFileW(
            device_path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (handle == INVALID_HANDLE_VALUE)
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message("CreateFileW failed");
            }
            return INVALID_HANDLE_VALUE;
        }

        return handle;
    }

    bool send_simple_ioctl(
        HANDLE handle,
        DWORD ioctl_code,
        int pad_index,
        std::string *out_error)
    {
        std::array<std::uint8_t, 16> buffer{};
        buffer[0] = 0x10;
        write_le32(&buffer[4], static_cast<std::uint32_t>(pad_index));

        DWORD bytes_returned = 0;
        BOOL ok = DeviceIoControl(
            handle,
            ioctl_code,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            nullptr,
            0,
            &bytes_returned,
            nullptr);

        if (!ok)
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message("DeviceIoControl failed");
            }
            return false;
        }

        return true;
    }

    bool send_report_ioctl(
        HANDLE handle,
        int pad_index,
        unsigned short buttons,
        std::string *out_error)
    {
        const auto controller_report = build_controller_report(buttons);
        const auto full_report = build_full_report(pad_index, controller_report);

        DWORD bytes_returned = 0;
        BOOL ok = DeviceIoControl(
            handle,
            kIoctlReport,
            const_cast<std::uint8_t *>(full_report.data()),
            static_cast<DWORD>(full_report.size()),
            nullptr,
            0,
            &bytes_returned,
            nullptr);

        if (!ok)
        {
            if (out_error != nullptr)
            {
                *out_error = get_last_error_message(
                    "DeviceIoControl(report) failed");
            }
            return false;
        }

        if (bytes_returned == 0)
        {
            if (out_error != nullptr)
            {
                *out_error = "DeviceIoControl(report) returned 0 bytes";
            }
            return false;
        }

        return true;
    }

#endif // _WIN32

} // namespace

SBR2VirtualPad::SBR2VirtualPad()
    : connected_(false), pad_index_(-1), device_handle_(nullptr)
{
}

SBR2VirtualPad::~SBR2VirtualPad()
{
    disconnect();
}

bool SBR2VirtualPad::connect(int pad_index)
{
    clear_error();

#ifndef _WIN32
    set_error("SBR2VirtualPad is Windows-only");
    return false;
#else
    if (pad_index <= 0)
    {
        set_error("pad_index must be >= 1");
        return false;
    }

    disconnect();

    std::wstring device_path;
    std::string error;

    if (!find_scp_bus_device_path(&device_path, &error))
    {
        set_error(error);
        return false;
    }

    HANDLE handle = open_scp_bus_handle(device_path, &error);
    if (handle == INVALID_HANDLE_VALUE)
    {
        set_error(error);
        return false;
    }

    if (!send_simple_ioctl(handle, kIoctlPlugIn, pad_index, &error))
    {
        CloseHandle(handle);
        set_error(error);
        return false;
    }

    device_handle_ = handle;
    pad_index_ = pad_index;
    connected_ = true;
    return true;
#endif
}

void SBR2VirtualPad::disconnect()
{
#ifdef _WIN32
    if (device_handle_ != nullptr)
    {
        HANDLE handle = static_cast<HANDLE>(device_handle_);

        if (connected_ && pad_index_ > 0)
        {
            std::string ignore_error;
            send_simple_ioctl(handle, kIoctlUnplug, pad_index_, &ignore_error);
        }

        CloseHandle(handle);
    }
#endif

    device_handle_ = nullptr;
    connected_ = false;
    pad_index_ = -1;
}

bool SBR2VirtualPad::is_connected() const
{
    return connected_;
}

bool SBR2VirtualPad::send_neutral()
{
    return send_buttons(kButtonNone);
}

bool SBR2VirtualPad::send_right()
{
    return send_buttons(kButtonRight);
}

bool SBR2VirtualPad::send_left()
{
    return send_buttons(kButtonLeft);
}

bool SBR2VirtualPad::send_up()
{
    return send_buttons(kButtonUp);
}

bool SBR2VirtualPad::send_down()
{
    return send_buttons(kButtonDown);
}

bool SBR2VirtualPad::send_up_left()
{
    return send_buttons(kButtonUpLeft);
}

bool SBR2VirtualPad::send_up_right()
{
    return send_buttons(kButtonUpRight);
}

bool SBR2VirtualPad::send_down_left()
{
    return send_buttons(kButtonDownLeft);
}

bool SBR2VirtualPad::send_down_right()
{
    return send_buttons(kButtonDownRight);
}

bool SBR2VirtualPad::send_bomb()
{
    return send_buttons(kButtonA);
}

bool SBR2VirtualPad::release_all()
{
    return send_buttons(kButtonNone);
}

bool SBR2VirtualPad::is_stub_mode() const
{
    return false;
}

const std::string &SBR2VirtualPad::last_error() const
{
    return last_error_;
}

bool SBR2VirtualPad::send_buttons(unsigned short buttons)
{
    clear_error();

#ifndef _WIN32
    set_error("SBR2VirtualPad is Windows-only");
    return false;
#else
    if (!connected_)
    {
        set_error("virtual pad is not connected");
        return false;
    }

    if (device_handle_ == nullptr)
    {
        set_error("device handle is null");
        return false;
    }

    std::string error;
    if (!send_report_ioctl(
            static_cast<HANDLE>(device_handle_),
            pad_index_,
            buttons,
            &error))
    {
        set_error(error);
        return false;
    }

    return true;
#endif
}

void SBR2VirtualPad::clear_error()
{
    last_error_.clear();
}

void SBR2VirtualPad::set_error(const std::string &message)
{
    last_error_ = message;
}