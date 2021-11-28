#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"
#include <unordered_map>

struct lobbyMessage {
	const char * msg;
	lobbyMessage(std::string& str) : msg(str.c_str()) {};
	lobbyMessage() {}
};

namespace net {
	template <typename T>
	class server_interface {
	protected: 
	protected:
		tsqueue<owned_message<T>> m_qMessagesIn;

		std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

		//This will store the unordered map of lobbies so that given an ID, 
		//it will return a specific lobby containing a variable amount of players
		std::unordered_map<uint32_t, std::deque<std::shared_ptr<connection<T>>>> lobbies;

		asio::io_context m_asioContext;
		std::thread m_threadContext;

		asio::ip::tcp::acceptor m_asioAcceptor;

		std::vector<std::thread> m_clientThreadHandlers;

		std::deque<uint32_t> pendingMatches; //every id in this list is waiting for a match

		uint32_t nIDCounter = 10000;
	public:

		

		server_interface(uint16_t port)
			: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

			lobbies.insert(std::make_pair(0, m_deqConnections));
		}

		virtual ~server_interface() {
			
			Stop();
		}

		bool Start() {
			try {
				WaitForClientConnection();

				m_threadContext = std::thread([this]() {m_asioContext.run();});

				for (auto& t : m_clientThreadHandlers) {
					t = std::thread([this]() { m_asioContext.run();});

				}

			}
			catch (std::exception& e) {
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}

			std::cout << "[SERVER] Started!\n";
			return true;
		}

		void Stop() {
			m_asioContext.stop();

			if (m_threadContext.joinable()) m_threadContext.join();

			for (auto& thread : m_clientThreadHandlers) {
				if (thread.joinable()) thread.join();
			}

			std::cout << "[SERVER] stopped!\n";
		}


		//ASYNC
		void WaitForClientConnection() {
			m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
				if (!ec) {
					std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
					

					std::shared_ptr<connection<T>> newconn =
						std::make_shared<connection<T>>(connection<T>::owner::server,
							m_asioContext, std::move(socket), m_qMessagesIn);

					if (OnClientConnect(newconn)) {

						
						m_deqConnections.push_back(std::move(newconn));

						m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

						//lobbies.at(0).push_back(std::move(newconn));

						//lobbies.insert_or_assign(0, newconn);
						std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";

						SendOnline();
					}
					else {
						std::cout << "[-----] Connection Denied\n";
					}	 
					
				}
				else {
					std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
				}

				WaitForClientConnection();
			});
		}

		void SendOnline() {
			std::list<uint32_t> connectionIds;
			for (auto& con : m_deqConnections) {
				connectionIds.push_back(con->GetID());
			}
			net::message<PlayerActions> myMsg;
			myMsg.header.id = PlayerActions::GetOnline;

			//myMsg << connectionIds.size();
			
			for (auto& ids : connectionIds) {
				myMsg << ids;
			}
			for(auto& con : m_deqConnections)
				con->Send(myMsg);
		}

		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg) {
			if (client && client->isConnected()) {
				client->Send(msg);
			}
			else {
				OnClientDisconnect(client);
				client->reset();
				m_deqConnections.erase(
					std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end()
				);
			}
		}

		

		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr) {
			
			bool bInvalidClientExists = false;
			
			for (auto& client : m_deqConnections) {
				if (client && client->isConnected()) {
					if (client != pIgnoreClient) {
						client->Send(msg);
						
					}
				}
				else {
					OnClientDisconnect(client);
					client->Disconnect();
					bInvalidClientExists = true;
				}
			}

			if (bInvalidClientExists) {
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
				
			}
		}

		

		void Update(size_t nMaxMessages = -1, bool bWait = false) {

			if (bWait) m_qMessagesIn.wait();

			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
				auto msg = m_qMessagesIn.pop_front();
				
				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

		std::shared_ptr<connection<T>>& getClient(uint32_t uid) {
			for (auto& client : m_deqConnections) {
				if (client->GetID() == uid)
					return client;
			}
		}

		//Forcibly disconnects a client based on unique userId
		void KickClient(uint32_t uid) {
			getClient(uid)->Disconnect();
		}

		//Forcibly disconnects a client based on a unique reference to a specific client
		void KickClient(std::shared_ptr<connection<T>> &client) {
			client->Disconnect();
		}

	protected:

		virtual bool OnClientConnect(std::shared_ptr<connection<T>> &client) {
			
			
			return false;
		}

		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> &client) {

		}

		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg) {
			
		}

		virtual void OnClientLobbyConnect(std::shared_ptr<connection<T>> &client, uint32_t lobbyId) {

		}
		/*
		* Adds a player to a given lobby and removes them from the previous lobby as well as
		* removes the previous lobby from the hashmap if it is empty
		*/
		void addPlayerToLobby(std::shared_ptr<connection<T>> client, uint32_t lobbyId, uint32_t oldLobby = 0) {
			
			
			if (!lobbies.contains(lobbyId)) { //if the client is entering a brand new lobby
				std::deque<std::shared_ptr<connection<T>>> temp;
				temp.push_back(client);
				lobbies.insert(std::make_pair(lobbyId, temp));
			}
			else {
				lobbies.find(lobbyId)->second.push_back(client);//add them to existing lobby
			}
			if (lobbies.contains(oldLobby)) {
				auto& lobby = lobbies.find(oldLobby)->second; //a reference to the deque which contains the list of players in that lobby
				int i = 0;
				for (auto& person : lobby) {
					if (person == client) {	//remove the player from the previous lobby they used to reside
						lobby.erase(lobby.begin() + i);
					}
					i++;
				}
				if (lobby.empty()) {
					lobbies.erase(lobbies.find(oldLobby));
				}

				//after erasing client from old lobby
				//update the new lobbies deque to include new client
				auto newLobby = lobbies.find(lobbyId)->second;
				newLobby.push_back(client);
			}
			
			
		}

		virtual void OnClientLobbyDisconnect() {

		}

	public:
		virtual void OnClientValidated(std::shared_ptr<connection<T>> client) {

		}
	};
}

