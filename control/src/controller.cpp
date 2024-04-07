/**
 * @file src/controller.hpp
 * @brief handles control logic of cars
*/

#include "controller.hpp"

controller::controller(const std::shared_ptr<message_worker> ms_in): ms(ms_in) {}

void controller::await_car() { /*TO-DO: add return value*/
    if (ms->await_checkin()) {
        std::pair<uint8_t, uint8_t> request =  ms->await_request();
        uint8_t current_pos = request.first; 
        uint8_t request_pos = request.second; 

        cars[current_pos] = request_pos; 
    }
}

void controller::process_requests() {
    for (auto car = cars.begin(); car != cars.end(); car++) {
        uint8_t current_pos = car->first; 
        uint8_t request_pos = car->second;
        //implement logic
    }
}



