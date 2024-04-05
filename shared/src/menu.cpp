/**
 * @file src/menu.cpp
 * @brief Menu display module.
 */
#include "menu.hpp"

#include <cctype>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

// Generation command: figlet 'AIRv1.0' -f slant -k -c | boxes -d parchment
static const std::string BANNER = R"(
                    _____________________________________________
                   /\                                            \
                   \_|     ___     ____ ____         ___   ____  |
                     |    /   |   /  _// __ \ _   __<  /  / __ \ |
                     |   / /| |   / / / /_/ /| | / // /  / / / / |
                     |  / ___ | _/ / / _, _/ | |/ // /_ / /_/ /  |
                     | /_/  |_|/___//_/ |_|  |___//_/(_)\____/   |
                     |                                           |
                     |   ________________________________________|_
                      \_/__________________________________________/
)";

static constexpr std::string CLEAR_TTY = "\033[2J\033[1;1H";

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct termios restore_setting;

static void set_tty();
static void reset_tty();
static void exit_sig_handler(int exit_code);

void show_menu(const std::string &heading,
	const std::vector<menu_item> &items,
	bool is_submenu) {
	while (true) {
		// Clear screen
		std::cout << CLEAR_TTY << std::flush;
		set_tty();

		auto digits = (uint32_t)std::log10(items.size() + 1) + 1;

		// Print list
		std::cout << BANNER << "\n";
		std::cout << heading << "\n\n";
		uint32_t index = 0;
		for (index = 0; index < items.size(); index++) {
			printf("[%0*u] %s\n", digits, index + 1, items[index].text.c_str());
		}
		printf(
			"[%0*u] %s\n\n", digits, index + 1, is_submenu ? "Return" : "Quit");
		std::cout << std::flush;

		// Await input
		uint32_t selection;
		do {
			// Clear selection
			std::cout << '\r';
			for (uint32_t i = 0; i < digits; i++) {
				std::cout << ' ';
			}
			std::cout << '\r' << std::flush;

			uint32_t digits_left = digits;
			selection = 0;
			while (digits_left > 0) {
				selection *= 10;
				int input = std::getchar();
				if (input == 'q') {
					std::cout << CLEAR_TTY << std::flush;
					reset_tty();
					return;
				}

				// NOLINTNEXTLINE(readability-implicit-bool-conversion)
				if (isdigit(input)) {
					uint32_t value = input - '0';
					selection += value;
					std::cout << value << std::flush;
					digits_left--;
				}
			}
		} while (selection == 0 || selection > index + 1);

		// Clear screen
		std::cout << CLEAR_TTY << std::flush;
		reset_tty();

		// Handle quit
		uint32_t selection_index = selection - 1;
		if (selection_index == index) {
			break;
		}

		items[selection_index].action();
	}
}

/**
 * @brief Set tty options we want.
 *
 */
void set_tty() {
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

/**
 * @brief Reset tty options.
 *
 */
void reset_tty() {
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
	reset_tty();
	std::exit(exit_code);
}
