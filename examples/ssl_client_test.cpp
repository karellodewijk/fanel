//This example demonstrated how you would connect a message passing tcp connection to one or multiple servers

#include <fanel/Ssl_connection.h>
#include <fanel/Ssl_connector.h>
#include <set>
#include <iostream>

//accepts a connection and sends a welcome message
class Connector : public Ssl_connector<> {
  public:
	Connector(boost::asio::io_service& io_service) : Ssl_connector<>(io_service) {}
	~Connector() {
	    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
	        delete *it;
	    } 
	}
	
	//You take ownership of the connections received here, so keep
	void accepted(Connection* connection) {
		m_connections.insert(connection);
		std::cout << "Connected" << std::endl;
		std::string hello_message = "0123456789";
		connection->write(hello_message.c_str(), hello_message.length());
		//hello_message = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
		connection->write(hello_message.c_str(), hello_message.length());
	}
	
	//one of the connctions generated throws an error.
	void error(Connection* connection, const system::error_code& error_code) {
	    delete connection;
		m_connections.erase(connection);
	}
	
	//the connector throws an error
	void error(const system::error_code& error_code) {
		std::cout << "Connection failed" << std::endl;
	}
	
	void received(Connection* connection, const char* data, int size) {
		std::cout << std::string(data, size) << " size: " << size << std::endl;
	}
	
	std::set<Connection*> m_connections;
};

int main() {
  try {
    boost::asio::io_service io_service;
    Connector connector(io_service);
    connector.ssl_connect("localhost", 6000, "keys/certificate.pem");
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
