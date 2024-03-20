#include "messages.hpp"
#include <iostream>
#include <stdexcept>

std::string format_message(const msg_t &msg) {
    std::string str_msg("AIRv1.0 " + msg.receiver_id + " SM\n" + msg.body + "\nEM " + msg.caller_id);
    return str_msg;
}

msg_t parse_message(const std::string &str_msg) {

    const size_t start_msg = str_msg.find("SM");
    const size_t end_msg = str_msg.find("EM");
    const std::string msg_header = "AIRv1.0";
    const size_t header_length =  msg_header.length();
    
    //checking validity of message
    if (str_msg.substr(0,header_length) != msg_header) {
        throw std::invalid_argument("No AIRv1.0 header found");
    } 

    if (start_msg == std::string::npos) {
        throw std::invalid_argument("No SM found");
    }
    if (end_msg == std::string::npos) {
        throw std::invalid_argument("No EM found");
    }

    //parsing message
    msg_t msg = {
        .caller_id = str_msg.substr(header_length + 1, start_msg - header_length - 1),
        .receiver_id = str_msg.substr(end_msg + 3),
        .body = str_msg.substr(start_msg + 4, end_msg - start_msg - 6),
    };
   
    return msg;
}