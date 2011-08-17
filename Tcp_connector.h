#ifndef TCP_CONNECTOR
#define TCP_CONNECTOR

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <set>
#include "copyable_unique_ptr.h"
#include "Connection_manager.h"

using boost::asio::ip::tcp;

/** \brief This is a class to one or more servers and ports asynchronously
 *
 * After initialisation, call connect(server, port) to connect. You can connect
 * to multiple servers or multiple ports on the same server or both.
 */

template <class Conection_type = Tcp_connection>
class Tcp_connector : public virtual Connection_manager {

  public:
    Tcp_connector(boost::asio::io_service& io_service);
    virtual ~Tcp_connector();
    
    void connect(const std::string server, int port);
  
  private:
    void handle_resolve(copyable_unique_ptr<tcp::resolver> resolver, const boost::system::error_code& error_code, tcp::resolver::iterator endpoint_iterator);
    void start_connect(tcp::resolver::iterator endpoint_iterator);  
    void handle_connect(copyable_unique_ptr<Conection_type> connection, const boost::system::error_code& error_code, tcp::resolver::iterator next_endpoint_iterator);

    boost::asio::io_service& io_service;
};

//implementation

template <class Conection_type>
Tcp_connector<Conection_type>::Tcp_connector(boost::asio::io_service& io_service_) 
   : io_service(io_service_) {}

template <class Conection_type>
Tcp_connector<Conection_type>::~Tcp_connector() {}
  
template <class Conection_type>
void Tcp_connector<Conection_type>::connect(const std::string server, int port) {
  tcp::resolver::query query(server, boost::lexical_cast< std::string>(port));
  
  tcp::resolver* pResolver = new tcp::resolver(io_service);
  copyable_unique_ptr<tcp::resolver> resolver(pResolver);
  
  pResolver->async_resolve(query,
    boost::bind(&Tcp_connector::handle_resolve, this, resolver,
        boost::asio::placeholders::error, 
        boost::asio::placeholders::iterator));         
}

template <class Conection_type>
void Tcp_connector<Conection_type>::handle_resolve(copyable_unique_ptr<tcp::resolver> resolver, const boost::system::error_code& error_code, tcp::resolver::iterator endpoint_iterator) {
    tcp::resolver::iterator end;
    if (!error_code && endpoint_iterator != end) {
        start_connect(endpoint_iterator);
    } else {
        error(error_code);
    }
}

template <class Conection_type>
void Tcp_connector<Conection_type>::start_connect(tcp::resolver::iterator endpoint_iterator) {
  Conection_type* pConnection = new Conection_type(io_service, this);
  copyable_unique_ptr<Conection_type> new_connection(pConnection);
  tcp::resolver::endpoint_type endPoint = *endpoint_iterator;
  pConnection->socket().async_connect(endPoint,
      boost::bind(&Tcp_connector::handle_connect, this, new_connection,
          boost::asio::placeholders::error, ++endpoint_iterator));
}
  
template <class Conection_type>
void Tcp_connector<Conection_type>::handle_connect(copyable_unique_ptr<Conection_type> connection, const boost::system::error_code& error_code, tcp::resolver::iterator next_endpoint_iterator) {
  if (!error_code) {
      Conection_type* pConnection = connection.release();
      pConnection->start();
      accepted(pConnection);
  } else {
      tcp::resolver::iterator end;
      if (next_endpoint_iterator != end) {
          start_connect(next_endpoint_iterator); //we try again with the next endpoint
      } else {
          error(error_code);   
      }
  }
}

#endif
