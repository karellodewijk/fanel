#ifndef CONNECTION_GENERATOR_H
#define CONNECTION_GENERATOR_H

#include "Tcp_connection.h"

using boost::asio::ip::tcp;

class Connection_manager {
  public:    
    ///overload to receive new connections
	virtual void accepted(Tcp_connection* connection) = 0;
	///overload to receive errors on existing connections
	virtual void error(Tcp_connection* connection, const boost::system::error_code& error) = 0;
	///overload to receive errors from the connection manager 
    virtual void error(const boost::system::error_code& error) {}
	///overload to receive data from an existing connection
	virtual void received(Tcp_connection* connection, const char* data, int size) = 0;
	
    ///use this to write data to a connection
	void write(Tcp_connection* connection, const char* data, int size);
};

#endif //CONNECTION_GENERATOR_H
