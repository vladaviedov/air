#pragma once

#include <stdlib.h>
#include <string>

struct car_t
{
    std::string id;
    std::string message;
    std::string pos;
    std::string time_stamp;
    bool in_int;
};

uint8_t find_car(car_t* cars, std::string car_id);
bool search_message(car_t* cars,std::string msg);
void update_position(car_t &car, std::string pos_msg);
