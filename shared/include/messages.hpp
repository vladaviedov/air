#pragma once

#include <cstdint>
#include <string>

struct msg_t {
    std::string caller_id;
    std::string receiver_id;
    std::string body;
};


std::string format_message(const msg_t &msg);
msg_t parse_message(std::string str_msg);
