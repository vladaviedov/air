/**
 * @file src/menu.cpp
 * @brief Menu display module.
 */
#include "menu.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

#include "utils.hpp"

// Generation command: figlet 'AIRv1.0' -f slant -k -c | boxes -d ansi-rounded
static const std::string BANNER =
	R"(                   ╭───────────────────────────────────────────╮
                   │     ___     ____ ____         ___   ____  │
                   │    /   |   /  _// __ \ _   __<  /  / __ \ │
                   │   / /| |   / / / /_/ /| | / // /  / / / / │
                   │  / ___ | _/ / / _, _/ | |/ // /_ / /_/ /  │
                   │ /_/  |_|/___//_/ |_|  |___//_/(_)\____/   │
                   │                                           │
                   ╰───────────────────────────────────────────╯
	)";

void show_menu(const std::string &heading,
	const std::vector<menu_item> &items,
	bool is_submenu) {
	while (true) {
		// Clear screen
		std::cout << CLEAR_TTY << std::flush;
		raw_tty();

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
					restore_tty();
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
		restore_tty();

		// Handle quit
		uint32_t selection_index = selection - 1;
		if (selection_index == index) {
			break;
		}

		items[selection_index].action();
	}
}
