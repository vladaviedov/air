#pragma once

#include <cstdint>
#include <stdlib.h>
#include <string>
#include <shared/tdma.hpp>

constexpr std::string MSG_HEADER = "AIRv1.0";
constexpr std::string CHECK = "CHK";
constexpr std::string ACKNOWLEDGE = "ACK";
constexpr std::string UNSUPPORTED = "UN";
constexpr std::string STANDBY = "SBY";
constexpr std::string CLEAR = "CLR";
constexpr std::string FINAL = "FIN";

class message_worker
{
public:
    message_worker(const std::shared_ptr<tdma> &tdma_handler_in);
    
    void await_checkin();
    std::pair<std::string, std::string> await_request();
    void await_clear();

    std::string format_checkin();
    std::string format_unsupported();
    std::string format_command(std::string command);
    
private:
    std::shared_ptr<tdma> tdma_handler;
};


