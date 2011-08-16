#include "Tcp_connector_and_acceptor.h"
#include <set>
#include <iostream>

//accepts a connection and sends a welcome message
class Connector_and_acceptor : public Tcp_connector_and_acceptor<> {
  public:
	Connector_and_acceptor(boost::asio::io_service& io_service) : Tcp_connector_and_acceptor<>(io_service) {}
	~Connector_and_acceptor() {
	    for (std::set<Tcp_connection*>::iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
	        delete *it;
	    } 
	}
	
	//overload of the virtual function Tcp_acceptor::read, called any time a new connection is received
	void accepted(Tcp_connection* connection) {
		m_connections.insert(connection);
		std::cout << "Connected" << std::endl;
		std::string hello_message = "Welcome";
		connection->write(hello_message.c_str(), hello_message.length());
	}
	
	void error(Tcp_connection* connection, const system::error_code& error_code) {
	    delete connection;
		m_connections.erase(connection);
	}
	
	void error(const system::error_code& error_code) {
		std::cout << "Received error: "<< error_code.message() << std::endl;
	}
	
	
	void received(Tcp_connection* connection, const char* data, int size) {
	    std::cout << "Received message of size "<< size << std::endl;
		//std::cout << "Received message: "<< std::string(data, size) << std::endl;
	}
	
	std::set<Tcp_connection*> m_connections;
};

int main() {

  //try {
  {
    boost::asio::io_service io_service;
    Connector_and_acceptor connector_and_acceptor(io_service);

    //in theory the behaviour of this code is dependant on what
    //initializes first, the acceptors or the connectors. 
    connector_and_acceptor.accept(6000);
    connector_and_acceptor.accept(6001);
    connector_and_acceptor.accept(6001);
    connector_and_acceptor.accept(6002);
    connector_and_acceptor.connect("localhost", 6001);
    connector_and_acceptor.stop_accept(6002);
    for (int i = 0; i < 1000; i++) {
        io_service.poll();
        usleep(1000); //sleep 1 ms, sorry windows users, I think
    }
    connector_and_acceptor.accept(6003);
    connector_and_acceptor.connect("localhost", 6003);
    for (int i = 0; i < 1000; i++) {
        io_service.poll();
        usleep(1000); //sleep 1 ms, sorry windows users, I think
    }
  }
  return 0;
}
