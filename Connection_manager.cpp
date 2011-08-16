#include "Connection_manager.h"

//implementation

void Connection_manager::write(Tcp_connection* connection, const char* data, int size) {
	    connection->write(data, size);
}
