/**
 * @file include/pinmap.hpp
 * @brief This file provides macro mappings from physical pin numbers on the
 * Raspberry Pi to numbers recognized by libgpiod.
 */
#pragma once

#include <cstdint>

// Source: https://i.stack.imgur.com/p2nBh.png
// Physical Pin Number -> libgpiod Pin Number
constexpr uint32_t RASPI_3 = 2;
constexpr uint32_t RASPI_5 = 3;
constexpr uint32_t RASPI_7 = 4;
constexpr uint32_t RASPI_8 = 14;
constexpr uint32_t RASPI_10 = 15;
constexpr uint32_t RASPI_11 = 17;
constexpr uint32_t RASPI_12 = 18;
constexpr uint32_t RASPI_13 = 27;
constexpr uint32_t RASPI_15 = 22;
constexpr uint32_t RASPI_16 = 23;
constexpr uint32_t RASPI_18 = 24;
constexpr uint32_t RASPI_19 = 10;
constexpr uint32_t RASPI_21 = 9;
constexpr uint32_t RASPI_22 = 25;
constexpr uint32_t RASPI_23 = 11;
constexpr uint32_t RASPI_24 = 8;
constexpr uint32_t RASPI_26 = 7;
constexpr uint32_t RASPI_29 = 5;
constexpr uint32_t RASPI_31 = 6;
constexpr uint32_t RASPI_32 = 12;
constexpr uint32_t RASPI_33 = 13;
constexpr uint32_t RASPI_35 = 19;
constexpr uint32_t RASPI_36 = 16;
constexpr uint32_t RASPI_37 = 26;
constexpr uint32_t RASPI_38 = 20;
constexpr uint32_t RASPI_40 = 21;
