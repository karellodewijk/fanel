#include <fanel/Tcp_connection.h>
#include <fanel/Tcp_acceptor.h>
#include <set>
#include <iostream>

//accepts a connection and sends a welcome message
class Acceptor : public Tcp_acceptor<> {
  public:
	Acceptor(boost::asio::io_service& io_service) : Tcp_acceptor<>(io_service) {}
	~Acceptor() {
	    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
	        delete *it;
	    }
	}
	
	//overload of the virtual function Tcp_acceptor::read, called any time a new connection is received
	void accepted(Connection* connection) {
		m_connections.insert(connection);
		std::cout << "Accepted a connection" << std::endl;
		std::string welcome_message = "Welcome to the test server";
		connection->write(welcome_message.data(), welcome_message.length());
	}
	
	//one of the generated connections is in error
	void error(Connection* connection, const system::error_code& error_code) {
	    std::cout << "A connection failed: " << error_code.message() << std::endl;
	    delete connection;
		m_connections.erase(connection);
	}
	
	//the acceptor throws an error, this does not mean it will stop accepting connections, just that accepting one particular connection failed
	void error(const system::error_code& error_code) {
		std::cout << "Accept failed: " << error_code.message() << std::endl;
	}
	
	void received(Connection* connection, const char* data, int size) {
		std::cout << std::string(data, size) << " size: " << size << std::endl;
	}
	
	std::set<Connection*> m_connections;
};

int main() {
  {
    boost::asio::io_service io_service;
    Acceptor acceptor(io_service);
    acceptor.accept(6000);
    io_service.run();
  }
  return 0;
}
