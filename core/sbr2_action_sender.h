#pragma once

#include "core/sbr2_ai_brain.h"
#include "core/sbr2_virtual_pad.h"

class SBR2ActionSender
{
public:
    explicit SBR2ActionSender(SBR2VirtualPad &pad);

    // action を仮想入力に変換して送る
    void send_action(SBR2Action action);

    //斜め入力用補助関数
    void send_buttons(unsigned short buttons);

private:
    SBR2VirtualPad &pad_;
};