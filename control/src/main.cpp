#include <iostream>

#include <shared/messages.hpp>

int main() {
	
	std::cout << "--------Parsing Message----------" << std::endl;
	
	msg_t parsed_msg = parse_message("AIRv1.0 122343 SM\n COOL \nEM 53");
		
	std::cout << std::endl;
	std::cout << "r-id: " << parsed_msg.receiver_id << std::endl;
	std::cout << "body: " << parsed_msg.body << std::endl;
    std::cout << "c-id: " << parsed_msg.caller_id << std::endl;
	
	std::string error_msg = "AIRv1.0 LOL BYE EM 2";
	try {
		parse_message(error_msg);
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
