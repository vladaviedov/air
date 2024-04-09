/**
 * @file include/utils.hpp
 * @brief Useful functions and constants.
 */
#pragma once

#include <cstdint>
#include <string>

static constexpr uint32_t FREQ_DEMO = 433900;
static constexpr uint32_t FREQ_CALIBRATION = 434900;
static constexpr uint32_t FREQ_LIVE = 435900;

static constexpr std::string CLEAR_TTY = "\033[2J\033[1;1H";

/**
 * @brief Prompt user to press any key.
 *
 */
void prompt_enter();

/**
 * @brief Set terminal into "raw"ish mode.
 * @note Turns off ECHO as well.
 * @note Sets its own signal handlers.
 */
void raw_tty();

/**
 * @brief Restore tty back to normal mode.
 * @note Resets signal handlers.
 */
void restore_tty();

/**
 * @brief Print program and license information.
 *
 */
void print_about();

/**
 * @brief Generate current timestamps in ms.
 *
 * @return Timestamp number.
 */
int32_t generate_ms();
