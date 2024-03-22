#include "intersection.hpp"
#include <stdlib.h>
#include <string>
#include <stdexcept>

uint8_t find_car(car_t* cars, std::string car_id) {
    for (int i=0; i < sizeof(cars) / sizeof(car_t); i++) {
        if (cars[i].id == car_id)
            return i; 
    }
    throw std::invalid_argument(car_id + " was not found");
}

bool search_message(car_t* cars,std::string msg) {
    for (int i=0; i < sizeof(cars) / sizeof(car_t); i++) {
        if (cars[i].message == msg)
            return true; 
    }
    return false;
}

void update_position(car_t &car, std::string pos_msg) {
    if (pos_msg == "PS1") {
		car.pos = 1;
	} else if (pos_msg == "PS2") {
		car.pos = 2;
	} else if (pos_msg == "PS3") {
		car.pos = 3;
	} else if (pos_msg == "PS4") {
		car.pos = 4;
	}
}