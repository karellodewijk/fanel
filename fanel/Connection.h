#ifndef FANEL_CONNECTION_H
#define FANEL_CONNECTION_H

#include <cstddef>
#include <boost/asio.hpp>
#include "Connection_manager.h"

using namespace boost;

class Connection_manager;

class Connection {
  public:
    Connection(Connection_manager& manager);
    virtual ~Connection() {}
    //start reading 
    virtual void start() = 0;
    //write data
    virtual void write(const char* data, size_t size) = 0;
    
  protected:
    #ifdef USE_CONNECTION_RECEIVE_OVERIDE
    //override this function to receive all data read
    virtual
    #endif
    void received(const char* data, size_t size);
    #ifdef USE_CONNECTION_ERROR_OVERIDE
    //override this functions to receive errors, like disconnect
    virtual
    #endif
    void error(const system::error_code& error_code);
    Connection_manager& connection_manager;
};

//implementation

Connection::Connection(Connection_manager& manager) : connection_manager(manager) {}

void Connection::received(const char* data, size_t size) {
     connection_manager.received(this, data, size);
}

void Connection::error(const system::error_code& error_code) {
     connection_manager.error(this, error_code);
}


#endif //FANEL_BASE_CONNECTION_H
