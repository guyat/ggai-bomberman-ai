#ifndef SBR2_VIRTUAL_PAD_H_
#define SBR2_VIRTUAL_PAD_H_

#include <string>

class SBR2VirtualPad
{
public:
    SBR2VirtualPad();
    ~SBR2VirtualPad();

    bool connect(int pad_index);
    void disconnect();

    bool is_connected() const;

    bool send_neutral();
    bool send_right();
    bool send_bomb();
    bool release_all();

    bool is_stub_mode() const;

    const std::string &last_error() const;

private:
    bool send_buttons(unsigned short buttons);
    void clear_error();
    void set_error(const std::string &message);

    bool connected_;
    int pad_index_;
    void *device_handle_;
    std::string last_error_;
};

#endif // SBR2_VIRTUAL_PAD_H_