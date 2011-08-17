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
//        std::string hello_message = "012345678901234567890123456789012345678901234567890123456780123456789012345678901234567890123456789";
//        hello_message += hello_message;
//        hello_message += hello_message;
//        hello_message += hello_message;
//        hello_message += hello_message;
//        hello_message += hello_message;
        std::string hello_message = "hfsdjklfhqsdfjksdhfkjlqsdhfjklsdqfhsdjkflhsdjkflsdhfjkldhfjkqsdhfsdjkqfhsdjklqfhqsdjklfhsdjkfhsdfkjlqsdhfjkldqshfsdjklfhsdjkflhqsdjkflhs";
        std::string hello_message2 = "";

        connection->write(hello_message.c_str(), hello_message.length());
        connection->write(hello_message2.c_str(), hello_message2.length());
        connection->write(hello_message.c_str(), hello_message.length());
        connection->write(hello_message2.c_str(), hello_message2.length());
    }
    
    void error(Tcp_connection* connection, const system::error_code& error_code) {
        delete connection;
        m_connections.erase(connection);
    }
    
    void error(const system::error_code& error_code) {
        std::cout << "Received error: "<< error_code.message() << std::endl;
    }
    
    
    void received(Tcp_connection* connection, const char* data, int size) {
        //std::cout << "Received message of size "<< size << std::endl;
        std::cout << "Received message: " << std::string(data, size) << std::endl;
    }
    
    std::set<Tcp_connection*> m_connections;
};

int main() {

  //try {
    boost::asio::io_service io_service;
    Connector_and_acceptor connector_and_acceptor(io_service);
    //open 2 accept channels and 2 connections on every channel.
    //This will open 8 connections, 4 server, 4 client
    //and thus 8 welcome messages
    connector_and_acceptor.accept(6000);
    connector_and_acceptor.accept(6001);
    connector_and_acceptor.accept(6002);
    connector_and_acceptor.connect("127.0.0.1", 6001);
    connector_and_acceptor.connect("127.0.0.1", 6001);
    connector_and_acceptor.connect("127.0.0.1", 6002);
    connector_and_acceptor.connect("127.0.0.1", 6002);
    //io_service.run(); would run indefinatly as the accept call will always remain active. This loop basically means
    //run all events for more or less 1 second.
    for (int i = 0; i < 1000; i++) {
        io_service.poll();
        usleep(1000); //sleep 1 ms, sorry windows users, I think
    }
  //} catch (std::exception& e) {
  //  std::cerr << e.what() << std::endl;
  //}
  return 0;
}
