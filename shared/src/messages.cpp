#include "messages.hpp"

std::string format_message(const msg_t &msg) {
    std::string str_msg("AIRv1.0 " + msg.receiver_id + " SM\n" + msg.body + "\nEM " + msg.caller_id);
    return str_msg;
}

msg_t parse_message(std::string str_msg) {
    const msg_t msg = {
        .caller_id = str_msg.substr(8,9),
        .receiver_id = str_msg.substr(str_msg.find("EM") + 2, str_msg.length()),
        .body = str_msg.substr(str_msg.find("SM") + 2, str_msg.find(" EM"))
    };
    return msg;
}



