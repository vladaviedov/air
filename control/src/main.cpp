#include <iostream>

#include <shared/messages.hpp>

int main() {
	const msg_t msg =  {
        .caller_id = "1",
        .receiver_id = "2",
        .body = "BYE",
    };
	std::cout << format_message(msg);

	const msg_t msg2 = parse_message(format_message(msg));
	std::cout << std::endl;
	std::cout << msg.receiver_id << std::endl;
	std::cout << msg.body << std::endl;
	std::cout << msg.caller_id << std::endl;
	
	return 0;
}
