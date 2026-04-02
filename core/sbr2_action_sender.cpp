#include "core/sbr2_action_sender.h"

SBR2ActionSender::SBR2ActionSender(SBR2VirtualPad &pad)
    : pad_(pad) {}

void SBR2ActionSender::send_action(SBR2Action action)
{
    switch (action)
    {
    case SBR2Action::WAIT:
        pad_.send_neutral();
        break;

    case SBR2Action::RIGHT:
        pad_.send_right();
        break;

    case SBR2Action::LEFT:
        pad_.send_left();
        break;

    case SBR2Action::UP:
        pad_.send_up();
        break;

    case SBR2Action::DOWN:
        pad_.send_down();
        break;

    case SBR2Action::PLACE_BOMB:
        pad_.send_bomb();
        break;

    default:
        // 未対応はとりあえずニュートラル
        pad_.send_neutral();
        break;
    }
}

void SBR2ActionSender::send_buttons(unsigned short buttons)
{
    bool up = buttons & (1u << 0);
    bool down = buttons & (1u << 1);
    bool left = buttons & (1u << 2);
    bool right = buttons & (1u << 3);

    if (up && left)
    {
        pad_.send_up_left();
    }
    else if (up && right)
    {
        pad_.send_up_right();
    }
    else if (down && left)
    {
        pad_.send_down_left();
    }
    else if (down && right)
    {
        pad_.send_down_right();
    }
    else if (up)
    {
        pad_.send_up();
    }
    else if (down)
    {
        pad_.send_down();
    }
    else if (left)
    {
        pad_.send_left();
    }
    else if (right)
    {
        pad_.send_right();
    }
    else
    {
        pad_.send_neutral();
    }
}