#pragma once
#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"


namespace net {

	template <typename T>
	class server_interface;

	template <typename T>
	class connection : public std::enable_shared_from_this<connection<T>> {
	public:
		enum class owner {
			server,
			client	
		};

		connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn) 
			: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn), read(asioContext), write(asioContext)
		{
			m_nOwnerType = parent;
			
			if (parent == owner::server) {
				handshakeIn = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				handshakeOut = handshakeIn;
				handshakeCheck = scramble(handshakeIn);
			}
			else {
				handshakeIn = 0;
				handshakeOut = 0;
			}

		};

		void ConnectToClient(net::server_interface<T>* server, uint32_t uid = 0) {
			if (m_nOwnerType == owner::server) {
				if (m_socket.is_open()) {
					id = uid;

					WriteValidation();

					ReadValidation(server);
				}
			}
		}

		uint64_t scramble(uint64_t securityMessage) {

			return securityMessage >> 2;
		}

		constexpr uint32_t GetID() const noexcept { return id; }
		constexpr uint32_t GetLobbyId() const noexcept { return lobbyId; }

		void SetLobbyId(uint32_t id) { lobbyId = id; }

		virtual ~connection() {}

	public:
		bool ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
			if (m_nOwnerType == owner::client) {
				asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
					if (!ec) {
						ReadValidation();
					}
				});
			 }
			return true;
		}
		bool Disconnect() {
			if (isConnected())
				asio::post(m_asioContext, [this]() {m_socket.close();});
			return true;
		};
		bool isConnected() const {
			return m_socket.is_open();
		};

	private:

		void ReadValidation(net::server_interface<T>* server = nullptr) {
			asio::async_read(m_socket, asio::buffer(&handshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length) {
					if (!ec) {
						if (m_nOwnerType == owner::server) {

							if (handshakeIn == handshakeCheck) {
								std::cout << "Client Validated\n";
								server->OnClientValidated(this->shared_from_this());

								ReadHeader();
							}
							else {
								std::cout << "Client failed validation";
									m_socket.close();
							}
							
						}
						else {
							handshakeOut = scramble(handshakeIn);

							WriteValidation();
						}
					}
					else {

					}
				});
		}

		void WriteValidation() {
			asio::async_write(m_socket, asio::buffer(&handshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length) {
					if (!ec) {
						if (m_nOwnerType == owner::client)
							ReadHeader();
					}
					else {
						std::cout << "Error Writing Validation\n";
						m_socket.close();
					}
				});
		}

		void ReadHeader() {
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
				asio::bind_executor(read, [this](std::error_code ec, std::size_t length) {
					if (!ec) {
						if (m_msgTemporaryIn.header.size > 0) {
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else {
							AddToIncomingMessageQueue();
						}
					}
					else {
						std::cerr << ec.message();
						std::cout << "[" << id << "] Read Header Fail.\n";
						m_socket.close();
					}
				}));
		}

		void ReadBody() {
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				asio::bind_executor(read, [this](std::error_code ec, std::size_t length) {
					
					if (!ec) {
						AddToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << id << "] Read Body Fail.\n";
						m_socket.close();
					}
				}
			));
		}

		void WriteHeader() {
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
				asio::bind_executor(write,[this](std::error_code ec, std::size_t length) {
					if (!ec) {
						
						if (m_qMessagesOut.front().body.size() > 0) {
							WriteBody();
						}
						else {
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty()) {
								WriteHeader();
							}
						}
					}
					else {
						std::cout << "[" << id << "] Write Header Fail.\n";
						m_socket.close();
					}
				}
				));
		}

		void WriteBody() {
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				asio::bind_executor(write, [this](std::error_code ec, std::size_t length) {

					if (!ec) {
					
						m_qMessagesOut.pop_front();

						if (!m_qMessagesOut.empty()) {
							WriteHeader();
						}
						
					}
					else {
						std::cout << "[" << id << "] Write Body Fail.\n";
						m_socket.close();
					}
				}
			));
		}

		void AddToIncomingMessageQueue() {
			if (m_nOwnerType == owner::server)
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

			ReadHeader();
		}

	public:
		bool Send(const message<T>& msg) {
			asio::post(m_asioContext, 
			[this, msg]() {
				bool bWritingMessage = !m_qMessagesOut.empty();
				m_qMessagesOut.push_back(msg);
				if (!bWritingMessage) {
					WriteHeader();
				}
				
			});
			return true;
		};

		

	protected:
		owner m_nOwnerType = owner::server;
		uint32_t id = 0;
		uint32_t lobbyId = 0;

		asio::ip::tcp::socket m_socket;

		asio::io_context& m_asioContext;

		asio::io_context::strand read;
		asio::io_context::strand write;


		tsqueue<message<T>> m_qMessagesOut;

		tsqueue<owned_message<T>>& m_qMessagesIn;
		message<T> m_msgTemporaryIn;

		uint64_t handshakeIn;
		uint64_t handshakeOut;

		uint64_t handshakeCheck;
	
	};


}  
