
#include <iostream>

#define DEBUG_CONNECT_PI

//#undef DEBUG_CONNECT_PI

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


std::vector<char> vBuffer(1 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket) {
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length) {
			if (!ec) {
				std::cout << "\n\nRead" << length << " bytes\n\n";

				for (int i = 0; i < length; i++)
					std::cout << vBuffer[i];

				GrabSomeData(socket);
			}
		});
}

int main() 
{
	//used to catch and interrogate errors
	asio::error_code ec;

	//the non-platform specific context in which connections take place
	asio::io_context context;

	//constructs the io_context what to do when there is no active work to be done, i.e., nothing
	//but also guarantees that the .run() call does not exit prematurely
	asio::io_context::work idleWork(context);

	std::thread thrContext = std::thread([&]() {context.run();});

	#ifdef DEBUG_CONNECT_PI
	//the thing we are trying to talk to/with
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("10.0.0.244", ec), 8080);
	#else
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);
	//a socket in which our connection will be routed through
	#endif
	asio::ip::tcp::socket socket(context);

	//connect to the endpoint
	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Connected!" << '\n';
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << '\n';
	}

	if (socket.is_open()) {
		GrabSomeData(socket);

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();
	}

	return 0;
}