/**
 * @file src/main.cpp
 * @brief Car entry point.
 */
#include <vector>

#include <shared/menu.hpp>

#include "demos.hpp"

static const std::vector<menu_item> car_menu = {
	{.text = "Demos", .action = &demo_submenu}};

int main() {
	show_menu("Car Actions", car_menu, false);
	return 0;
}
