//This example demonstrated how you would connect a message passing tcp connection to one or multiple servers

#include "Tcp_connection.h"
#include "Tcp_connector.h"
#include "Connection_manager.h"
#include <set>
#include <iostream>

//accepts a connection and sends a welcome message
class Connector : public Tcp_connector<> {
  public:
	Connector(boost::asio::io_service& io_service) : Tcp_connector<>(io_service) {}
	~Connector() {
	    for (std::set<Tcp_connection*>::iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
	        delete *it;
	    } 
	}
	
	//You take ownership of the connections received here, so keep
	void accepted(Tcp_connection* connection) {
		m_connections.insert(connection);
		std::cout << "Connected" << std::endl;
		std::string hello_message = "0123456789::0123456789::0123456789::0123456789::0123456789::0123456789::0123456789::0123456789::0123456789::0123456789";
		connection->write(hello_message.c_str(), hello_message.length());
		//hello_message = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
		connection->write(hello_message.c_str(), hello_message.length());
	}
	
	//one of the connctions generated throws an error.
	void error(Tcp_connection* connection, const system::error_code& error_code) {
	    delete connection;
		m_connections.erase(connection);
	}
	
	//the connector throws an error
	void error(const system::error_code& error_code) {
		std::cout << "Connection failed" << std::endl;
	}
	
	void received(Tcp_connection* connection, const char* data, int size) {
		std::cout << std::string(data, size) << std::endl;
	}
	
	std::set<Tcp_connection*> m_connections;
};

int main() {
  try {
    boost::asio::io_service io_service;
    Connector connector(io_service);
    connector.connect("127.0.0.1", 6000);
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
