#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <queue>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


//defines actions that a player can commence
//shared between server and client
enum class PlayerActions : uint32_t {
	MessageIndividual,
	MessageAll,
	LobbyConnect,
	LobbyConnectSuccess,
	LobbyMessage,

	MatchRequest,
	MatchQuit,
	MatchAccept,
	MatchInit,

	FriendRequest,
	ServerAccept,
	ServerDeny,
	ServerPing,

	ServerMessage,
	ServerQuestion,

	GetOnline,

	MovePiece

};
