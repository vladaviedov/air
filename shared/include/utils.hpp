/**
 * @file include/utils.hpp
 * @brief Useful functions and constants.
 */
#pragma once

#include <string>

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
