#include <iostream>

#include "core/sbr2_virtual_pad.h"

int main()
{
    SBR2VirtualPad pad;

    std::cout << "stub mode: "
              << (pad.is_stub_mode() ? "true" : "false")
              << std::endl;

    if (!pad.connect(1))
    {
        std::cout << "connect failed: " << pad.last_error() << std::endl;
        return 1;
    }

    std::cout << "connect ok" << std::endl;

    if (!pad.send_neutral())
    {
        std::cout << "send_neutral failed: " << pad.last_error() << std::endl;
        return 1;
    }
    std::cout << "send_neutral ok" << std::endl;

    if (!pad.send_right())
    {
        std::cout << "send_right failed: " << pad.last_error() << std::endl;
        return 1;
    }
    std::cout << "send_right ok" << std::endl;

    if (!pad.send_bomb())
    {
        std::cout << "send_bomb failed: " << pad.last_error() << std::endl;
        return 1;
    }
    std::cout << "send_bomb ok" << std::endl;

    if (!pad.release_all())
    {
        std::cout << "release_all failed: " << pad.last_error() << std::endl;
        return 1;
    }
    std::cout << "release_all ok" << std::endl;

    pad.disconnect();
    std::cout << "disconnect done" << std::endl;

    return 0;
}