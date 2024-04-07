/**
 * @file src/controller.hpp
 * @brief handles control logic of cars
*/
#pragma once

#include <memory>
#include <cstdlib>      
#include <string>
#include <map>
#include "messageworker.hpp"

class controller
{
public:
    /**
     * @brief constuctor for message worker
     * @param[in] ms_in
    */
    controller(const std::shared_ptr<message_worker> ms_in);
    /**
     * @brief processes car requests
    */
    void process_requests();

    /**
     * @brief awaits car to arrive at intersection and send request
    */
    void await_car();
private:
    std::shared_ptr<message_worker> ms;
    std::map<uint8_t, uint8_t> cars;
};