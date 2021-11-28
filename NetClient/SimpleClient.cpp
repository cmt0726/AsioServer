#include <server.h>
#include <iostream>

enum struct CustomMsgTypes : uint32_t {
	FireBullet,
	MovePlayer
};

class CustomClient : public net::client_interface<CustomMsgTypes> {

};


