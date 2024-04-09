/**
 * @file src/controller.hpp
 * @brief handles control logic of cars
*/
#include "controller.hpp"
#include <driver/pinmap.hpp>
#include <driver/device.hpp>

controller::controller(uint8_t intersect_size, tdma::scheme div) : rf_module(std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0)) {
    blocked_intersects.reserve(intersect_size);
    workers.reserve(intersect_size);
    for (uint32_t i = 0; i < intersect_size; i++) {
        auto tdma_ptr = std::make_shared<tdma>(rf_module, i, div);
        message_worker worker(tdma_ptr);
        auto executor = [&](uint8_t curr_pos, uint8_t requested_pos, std::string car_id, message_worker worker) {
            receive_request_callback(curr_pos, requested_pos, car_id, worker);
        };
        worker.await_request(executor);
        workers.push_back(std::move(worker)); //check if push back causes problems
        blocked_intersects.push_back(false);
    }
}

void controller::receive_request_callback(uint8_t current_pos, uint8_t requested_pos, std::string car_id, message_worker worker) {

    car car = {
        .car_id = car_id,
        .state = CHECKIN,
        .current_pos = current_pos,
        .request_pos = requested_pos,
    };  

    lock.lock();
    cars.insert(cars.begin() + worker.get_timeslot(), car);
    lock.unlock();
}

void controller::process_requests() {
    
    for (auto &curr : cars) {
        uint8_t curr_pos = curr.current_pos;
        uint8_t request_pos = curr.request_pos;
        if (curr.state == CHECKIN) {
            if (blocked_intersects[request_pos]) {
                workers[curr_pos].send_standby();
                curr.state = STANDBY;
            }
            else {
                workers[curr_pos].send_go_requested();
                move_car(curr);
            }
        }
        if (curr.state == STANDBY) {
            if (!blocked_intersects[request_pos]) {
                move_car(curr);
            }
        }
    }
}

void controller::move_car(car &car) {
    car.state = MOVING;
    for (uint8_t i = car.current_pos + 1; i <= car.request_pos; i++) {
        blocked_intersects[i] = true;
    }
    auto executor = [&](bool cleared, message_worker worker) {
        clear_callback(cleared, worker);
    };
    workers[car.current_pos].await_clear(executor);
}

void controller::clear_callback(bool cleared, message_worker message_worker) {
    car curr_car = cars[message_worker.get_timeslot()];
    if (cleared) {
        for (uint8_t i = curr_car.current_pos + 1; i <= curr_car.request_pos; i++) {
            blocked_intersects[i] = false;
        }
        cars.erase(cars.begin() + message_worker.get_timeslot());
    }
}







