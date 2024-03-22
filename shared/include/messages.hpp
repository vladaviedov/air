#pragma once

#include <cstdint>
#include <string>

struct msg_t {
	std::string caller_id;
	std::string receiver_id;
	std::string body;
};

/*
const std::string requests[6] = {
    "CHK", "RQ1", "RQ2", "RQ3", "RQ4", "CLR"
};

const std::string positions[4] = {
    "PS1", "PS2", "PS3", "PS4"
};

const std::string positions[4] = {
    "PS1", "PS2", "PS3", "PS4"
};

const std::string directives[6] = {
    "GRQ", "SBY", "RG1", "RG2", "RG3", "RG4"
};

const std::string helpers[6] = {
    "ACK", "NCK"
};
*/

std::string format_message(const msg_t &msg);
msg_t parse_message(const std::string &str_msg);
