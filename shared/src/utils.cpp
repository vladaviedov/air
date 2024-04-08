/**
 * @file src/utils.cpp
 * @brief Useful functions and constants.
 */
#include "utils.hpp"

#include <csignal>
#include <cstdio>
#include <iostream>
#include <termios.h>

static const std::string ABOUT = R"(AIR Car & Control Software
Copyright (C) 2024 Vladyslav Aviedov, Caio DaSilva, Scott Abramson

This program is part of a project for GE1502 Cornerstone of Engineering 2 at
Northeastern University (Spring 2024). Project group members:
- Aidan Hanson
- Caio DaSilva
- Scott Abramson
- Vladyslav Aviedov

Source code: https://github.com/vladaviedov/air
License: GNU General Public License v2.0

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by the Free
Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
)";

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct termios restore_setting;

static void exit_sig_handler(int exit_code);

void prompt_enter() {
	std::cout << "\nPress enter to continue." << std::flush;
	std::getchar();
}

void raw_tty() {
	tcgetattr(STDOUT_FILENO, &restore_setting);
	struct termios new_setting = restore_setting;

	new_setting.c_lflag &= ~(ICANON | ECHO);
	new_setting.c_cc[VTIME] = 0;
	new_setting.c_cc[VMIN] = 1;

	tcsetattr(STDOUT_FILENO, TCSANOW, &new_setting);

	std::signal(SIGINT, &exit_sig_handler);
	std::signal(SIGTERM, &exit_sig_handler);
	std::signal(SIGQUIT, &exit_sig_handler);
}

void restore_tty() {
	tcsetattr(STDOUT_FILENO, TCSANOW, &restore_setting);

	std::signal(SIGINT, SIG_DFL);
	std::signal(SIGTERM, SIG_DFL);
	std::signal(SIGQUIT, SIG_DFL);
}

void print_about() {
	std::cout << ABOUT;
	prompt_enter();
}

/**
 * @brief Handle exit interrupts.
 *
 * @param[in] exit_code - Exit code.
 */
void exit_sig_handler(int exit_code) {
	restore_tty();
	std::exit(exit_code);
}
