/**
 * @file src/utils.cpp
 * @brief Useful functions and constants.
 */
#include "utils.hpp"

#include <csignal>
#include <cstdio>
#include <iostream>
#include <termios.h>

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

/**
 * @brief Handle exit interrupts.
 *
 * @param[in] exit_code - Exit code.
 */
void exit_sig_handler(int exit_code) {
	restore_tty();
	std::exit(exit_code);
}
