#include <server.h>
#include <iostream>

/*
	TODO:
		After adding a player to a lobby
		remove their reference in the previous lobby

		If a client forcibly closes connection,
		remove them from all references
*/


struct lobbyChange {
	uint32_t from;
	uint32_t to;
};

class CustomServer : public net::server_interface<PlayerActions>
{
public:
	CustomServer(uint16_t nPort) : net::server_interface<PlayerActions>(nPort) {
		
	}

	void printLobbies() {
		auto& m_lobbies = this->lobbies;

		std::cout << "Lobbies currently occupied: \n";

		for (auto& lobby : m_lobbies) {
			std::cout << "  " << lobby.first << ": \n";
			for(auto& player : lobby.second)
				std::cout << "      " << player->GetID() << ", " << '\n';
		}
	}

	/*
			Messages all clients within a given lobby. Expects a struct from msg describing the lobby and the message
		*/
	void MessageAllClientsLobby(uint32_t lobbyId, const net::message<PlayerActions>& inMsg, std::shared_ptr <net::connection<PlayerActions>> pIgnoreClient = nullptr) {



		auto& lobbyClientList = lobbies.find(lobbyId)->second;


		for (auto& client : lobbyClientList) {
			if (client != pIgnoreClient)
				client->Send(inMsg);
		}

	}

protected:
	virtual void OnClientLobbyConnect(std::shared_ptr<net::connection<PlayerActions>> client, uint32_t lobbyId, uint32_t oldLobby) {
		addPlayerToLobby(client, lobbyId, oldLobby);
	}

	virtual bool OnClientConnect(std::shared_ptr<net::connection<PlayerActions>> &client) {
		
		



		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<net::connection<PlayerActions>>& client) {

	}

	virtual void OnMessage(std::shared_ptr<net::connection<PlayerActions>> client, net::message<PlayerActions>& msg)  {
		
		switch (msg.header.id) {
		case PlayerActions::ServerPing: {
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			client->Send(msg);
			break;
		}
		case PlayerActions::ServerMessage: {
			std::cout << "The user is trying to tell us something lol\n";

			net::message<PlayerActions> ourMsg;

			ourMsg << "Haha you suck\n";
			ourMsg.header.id = PlayerActions::ServerMessage;
			
			client->Send(ourMsg);
			break;
		}
		case PlayerActions::LobbyConnect: {


			lobbyChange lb;
			msg >> lb;
			std::cout << "Client: " << client->GetID() << " wants to connect to lobby: " << lb.to << '\n';
			OnClientLobbyConnect(client, lb.to, lb.from);

			net::message<PlayerActions> ourMsg;
			ourMsg.header.id = PlayerActions::LobbyConnectSuccess;
			ourMsg << lb.to;
			client->SetLobbyId(lb.to);
			client->Send(ourMsg);
			break;
		}
		case PlayerActions::LobbyMessage: {


			
			MessageAllClientsLobby(client->GetLobbyId(), msg, client);
			break;
		}
		case PlayerActions::MatchRequest: {
			//uint32_t playerReqId;
			//msg >> playerReqId;
			std::cout << "Client: [" << client->GetID() << "] wants to play " << "\n";
			net::message<PlayerActions> myMsg;
			myMsg.header.id = PlayerActions::MatchInit;

			if (pendingMatches.empty()) {
				pendingMatches.push_back(client->GetID());
			}
			else {
				auto id = pendingMatches.front();
				if (id == client->GetID()) //the client that is requesting a game is already waiting for a game
					break;
				pendingMatches.pop_front();

				myMsg << client->GetID();

				auto& clientToSend = getClient(id);
				clientToSend->Send(myMsg);	//tell both players, the one who requested and the one who's waiting, that they're going to enter a game
				client->Send(myMsg);
			}

			break;
		}
		case PlayerActions::MatchAccept: {
			uint32_t lobbyId = 1234; //temp
			net::message<PlayerActions> myMsg;
			myMsg.header.id = PlayerActions::LobbyConnectSuccess;
			myMsg << lobbyId;
			uint32_t clientWhoSentReqId;
			msg >> clientWhoSentReqId;
			auto& clientWhoSentReq = getClient(clientWhoSentReqId);

			OnClientLobbyConnect(client, lobbyId, client->GetLobbyId());
			OnClientLobbyConnect(clientWhoSentReq, lobbyId, clientWhoSentReq->GetLobbyId());

			client->SetLobbyId(lobbyId);
			clientWhoSentReq->SetLobbyId(lobbyId);

			

			clientWhoSentReq->Send(myMsg);
			client->Send(myMsg);
			break;
		}
		case PlayerActions::GetOnline: {
			std::list<uint32_t> connectionIds;
			for (auto& con : m_deqConnections) {
				connectionIds.push_back(con->GetID());
			}
			net::message<PlayerActions> myMsg;
			myMsg.header.id = PlayerActions::GetOnline;

			for (auto& ids : connectionIds) {
				myMsg << ids;
			}
			client->Send(myMsg);
			break;
		}
		case PlayerActions::MovePiece: {
			//tells all watching players the new updated piece movement
			MessageAllClientsLobby(client->GetLobbyId(), msg, client);
			break;

		}
		break;

		}
	}
};

int main() {
	CustomServer server(60000);
	server.Start();	 
	
	bool key[5] = { false, false, false, false, false };
	bool old_key[5] = { false, false, false, false, false };


	while (1) {
		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
			key[3] = GetAsyncKeyState('4') & 0x8000;
			key[4] = GetAsyncKeyState('5') & 0x8000;
		}

		if (key[0] && !old_key[0]) server.printLobbies();

		for (int i = 0; i < 5; i++) old_key[i] = key[i];
		server.Update(-1, false);
	}
	

	
	return 0;
}
