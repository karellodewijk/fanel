#ifndef TCP_CONNECTOR_AND_ACCEPTOR_H
#define TCP_CONNECTOR_AND_ACCEPTOR_H

#include "Tcp_connector.h"
#include "Tcp_acceptor.h"

/** \brief Well the declaration says it all
 *
*/

template <class Connection_type = Tcp_connection>
class Tcp_connector_and_acceptor : public Tcp_connector<Connection_type>, public Tcp_acceptor<Connection_type> {
  public:
    Tcp_connector_and_acceptor(boost::asio::io_service& io_service) : Tcp_connector<Connection_type>(io_service), Tcp_acceptor<Connection_type>(io_service) {}
}; 

#endif //CONNECTOR_ACCEPTOR_H

