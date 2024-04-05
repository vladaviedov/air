/**
 * @file src/menu.hpp
 * @brief
 */
#pragma once

#include <functional>
#include <string>
#include <vector>

struct menu_item {
	std::string text;
	std::function<void()> action;
};

/**
 * @brief Display menu.
 *
 * @param[in] heading - Menu heading.
 * @param[in] items - Menu items.
 * @param[in] is_submenu - Changes 'Quit' to 'Return'.
 */
void show_menu(const std::string &heading,
	const std::vector<menu_item> &items,
	bool is_submenu);
