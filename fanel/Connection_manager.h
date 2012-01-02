#ifndef FANEL_CONNECTION_MANAGER_H
#define FANEL_CONNECTION_MANAGER_H

#include <boost/asio.hpp>

class Connection;

class Connection_manager {
  public:
    virtual ~Connection_manager() {}
    ///overload to receive new connections
    virtual void accepted(Connection* connection) = 0;
    ///overload to receive errors on existing connections
    virtual void error(Connection* connection, const boost::system::error_code& error) = 0;
    ///overload to receive errors from the connection manager 
    virtual void error(const boost::system::error_code& error) {}
    ///overload to receive data from an existing connection
    virtual void received(Connection* connection, const char* data, int size) = 0;
    ///overload to receive a notice when a connection is done writing
    virtual void write_done(Connection* connection) {};
};

#endif //FANEL_CONNECTION_MANAGER_H
