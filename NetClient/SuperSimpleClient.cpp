#include <iostream>
#include <server.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "RenderWindow.hpp"
#include "UIManager.hpp"


struct lobbyChange {
	uint32_t from;
	uint32_t to;
} lb;

class CustomClient : public net::client_interface<PlayerActions>, public UIManager {
public:

	void changePerspective() {
		changeBoardPerspective(boardType::white);
	}

	std::shared_ptr<ChessBoard> ChessGame;
	
	void PingServer() {
		
		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::ServerPing;

		auto timeNow = std::chrono::system_clock::now(); //dangerous

		msg << timeNow;
		Send(msg);
		
	}

	void ChessMatch() {
		ChessBoard cb(true);

		ChessGame = std::make_shared<ChessBoard>(cb);
		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::MatchRequest;

		//uint32_t playerIdToMatchWith = 10001; //temp

		//msg << playerIdToMatchWith;
		Send(msg);
	}

	virtual void requestGame() override {
		ChessMatch();
	}

	void acceptMatch(uint32_t playerIdToAccept = 0) {
		
		//scene = sceneType::game;

		changeBoardPerspective(boardType::black);  //temp

		ChessBoard cb(false);

		ChessGame = std::make_shared<ChessBoard>(cb);

		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::MatchAccept;

		uint32_t playerIdToMatchWith = 10000; //temp

		//msg << playerIdToMatchWith;
		//Send(msg);
		changeLobbyToGame();
	}

	void SendId() {
		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::ServerMessage;

		msg << "Hello my id is: " + this->m_connection->GetID();
		Send(msg);
	}

	virtual void GetCurrentLobby() {
		std::cout << "You are in lobby: " << this->m_connection->GetLobbyId() << '\n';
	}

	virtual void sendUpdatedPiecePos(std::pair<int, int> from, std::pair<int, int> to) override {

		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::MovePiece;

		msg << from.first;
		msg << from.second;
		msg << to.first;
		msg << to.second;

		Send(msg);

	}

	virtual bool validMove(std::pair<int, int> from, std::pair<int, int> to) override {
		bool res;
		if (boardT == boardType::white) {
			res = ChessGame->Move(from.second, from.first, to.second, to.first, true); //flips from row to column major order since chess indexes starting with which column instead of which row
		} else {
			res = ChessGame->Move(7 - from.second, 7 - from.first, 7 - to.second, 7 - to.first, true); //blacks perspective
		}
		ChessGame->printBoard();

		std::cout << res << '\n';

		std::cout << "(" << from.second << '-' << from.first << ")" << std::endl;
		std::cout << "(" << to.second << '-' << to.first << ")" << std::endl;
		return res;
	}

	auto RequestOnlinePlayers() {

		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::GetOnline;

		msg << this->m_connection->GetID();	//constructed so that the server returns list of everyone online except you
		Send(msg);

	}

	void ChangeLobby(uint32_t lobby) {
		net::message<PlayerActions> msg;
		msg.header.id = PlayerActions::LobbyConnect;
		
		lb.from = this->m_connection->GetLobbyId();
		lb.to = lobby;
		msg << lb;
		Send(msg);
	}

	void SetLobby(uint32_t lobby) {
		this->m_connection->SetLobbyId(lobby);
	}

	void messageLobby() {
		
		std::cout << "SendMsg: ";

		std::string lobbyMessageSend;

		std::cin >> lobbyMessageSend;

		const char* myMessage = lobbyMessageSend.c_str();
		size_t size = lobbyMessageSend.size();
		//lm.lobbyId = this->m_connection->GetLobbyId();
		//lm.msg = lobbyMessageSend.c_str();
		
		net::message<PlayerActions> msg;
		for (int i = 0; i < size; i++) {
			msg << (*(myMessage + i));
		}
		msg << '\0';
		msg.header.id = PlayerActions::LobbyMessage;
		Send(msg);
	}

	void HandleServerResponse(net::message<PlayerActions>& msg) {

		createAndPushArbEvent();

		switch (msg.header.id) {
			
			case PlayerActions::ServerPing: {
				auto timeNow = std::chrono::system_clock::now();
				decltype(timeNow) timeThen;

				msg >> timeThen;

				std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				break;
			}
			case PlayerActions::ServerMessage: {

				std::cout << msg.body.data();
				break;
			}
			case PlayerActions::MessageAll: {
				std::cout << msg.body.data();
				break;
			}
			case PlayerActions::LobbyConnectSuccess: {
				std::cout << "Connection to lobby Success!" << '\n';
				uint32_t lobby;
				msg >> lobby;


				this->SetLobby(lobby);
				this->GetCurrentLobby();

				break;
			}
			case PlayerActions::LobbyMessage: {

				std::cout << msg.body.data() << '\n';
				break;
			}
			case PlayerActions::MatchRequest: {
				uint32_t incReqId;
				msg >> incReqId;
				std::cout << "Player: [" << incReqId << "] would like to play a match\n";
				acceptMatch(incReqId);
				break;
			}
			case PlayerActions::MatchInit: {

				acceptMatch();
				break;

			}
			case PlayerActions::GetOnline: {
				
				int i = 0;
				while (msg.size() != 0) {
					uint32_t id;
					msg >> id;
					online.push_back(id);
					i++;
				}
				break;
			}
			//updates a given piece
			case PlayerActions::MovePiece: {	

				isWhitesTurn = !isWhitesTurn;

				int tY;
				int tX;
				int fY;
				int fX;
				msg >> tY;
				msg >> tX;
				msg >> fY;
				msg >> fX;

				std::pair<int, int> from{ fX, fY };
				std::pair<int, int> to{ tX, tY };

				int preChessX = fX / tileDistanceinPixelsHeight;
				int preChessY = fY / tileDistanceInPixels;

				int chessX = tX / tileDistanceinPixelsHeight;
				int chessY = tY / tileDistanceInPixels;
				std::pair<int, int> chessCoord{ chessX, chessY };
				std::cout << chessCoord.first << " " << chessCoord.second << '\n';
	
				if (boardT == boardType::black) {
					ChessGame->Move(7 - preChessY, 7 - preChessX, 7 - chessY, 7 - chessX, true); //take the incoming move and simulate it for the engine behind the scenes
				}
				else {
					ChessGame->Move(preChessY, preChessX, chessY, chessX, true);
				}
				ChessGame->printBoard();
				checkIfOccupiedAndEvict(chessCoord); //inverted pieces cuz for now this is from black's POV

				snapFromTo(from, to);
				break;
			}
		}
	}

	uint32_t id;
};

void StateExec() {

	CustomClient c;

	c.changePerspective();

	std::thread uimanager(&CustomClient::Run, &c);

	c.Connect("127.0.0.1", 60000);


	bool bQuit = false;


	while (!bQuit) {		

		if (c.isConnected()) {

			if (!c.Incoming().empty()) {

				auto msg = c.Incoming().pop_front().msg;

				c.HandleServerResponse(msg);
			}
			
		}
		else {
			std::cout << "Server Down\n";
			bQuit = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}





int main(int argc, char * argv[]) {


	StateExec();
	
	return 0;
}
