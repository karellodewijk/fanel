#ifndef TCP_ACCEPTOR_H
#define TCP_ACCEPTOR_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <map>
#include <memory>
#include <iostream>

#ifdef THREADSAFE
#include <boost/thread/shared_mutex.hpp>
#endif

#include "Tcp_connection.h"
#include "Connection_manager.h"
#include "copyable_unique_ptr.h"

using boost::asio::ip::tcp;

/** \brief This is a class to create listeners on one or more ports asynchronously.
 *
 * After initialisation, call accept(port number) to receive connections
 * over a port. It is allowed to call this function more than once with
 * different port numbers. It will then create listeners and accept connections
 * on all ports. 
 *
 * The template parameter is the connection type the class
 * should create and should either be Tcp_connection or descend from it.
 *
 */

template <class Conection_type = Tcp_connection>
class Tcp_acceptor : public virtual Connection_manager {

  public:
    Tcp_acceptor(boost::asio::io_service& io_service);
    virtual ~Tcp_acceptor();
  
    void accept(int port);
    void stop_accept(int port);
    
private:
    void start_accept(std::pair<int, tcp::acceptor*>& acceptor);
    void start_acceptors(std::pair<int, tcp::acceptor*>& acceptor, tcp::resolver::iterator endpoint_iterator);
    void handle_accept(std::pair<int, tcp::acceptor*> acceptor, copyable_unique_ptr<Conection_type> connection, const boost::system::error_code& error);
    void handle_resolve(copyable_unique_ptr<tcp::resolver> resolver, std::pair<int, tcp::acceptor*> acceptor, const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
    inline bool acceptor_still_valid(std::pair<int, tcp::acceptor*>& acceptor);
  
    //port, acceptor map
    std::multimap<int, tcp::acceptor*> acceptors;
    #ifdef THREADSAFE
        boost::shared_mutex acceptors_mutex;
    #endif
    
    boost::asio::io_service& io_service;
};

//implementation

template <class Conection_type>
Tcp_acceptor<Conection_type>::Tcp_acceptor(boost::asio::io_service& io_service_)
  : io_service(io_service_)
{}

template <class Conection_type>
Tcp_acceptor<Conection_type>::~Tcp_acceptor() {
    #ifdef THREADSAFE
        boost::unique_lock<boost::shared_mutex> lock(acceptors_mutex);
    #endif
    for (std::multimap<int, tcp::acceptor*>::iterator it = acceptors.begin(); it != acceptors.end(); ++it)
        delete it->second;
    acceptors.clear();
}

template <class Conection_type>
bool Tcp_acceptor<Conection_type>::acceptor_still_valid(std::pair<int, tcp::acceptor*>& acceptor) {
    //this code is only valid if listeners on a port regardless of protocol can only be removed
    //together. If you wish to allow closing just ipv4/ipv6 endpoints exclusively, then not only 
    //the port must be checked but also the acceptor pointer. 
    return (acceptors.find(acceptor.first) != acceptors.end());

}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::accept(int port) {
    tcp::resolver::query query(boost::lexical_cast< std::string>(port));

    std::pair<int, tcp::acceptor* > acceptor_pair = std::make_pair(port, new tcp::acceptor(io_service));
    {
        #ifdef THREADSAFE
        boost::unique_lock<boost::shared_mutex> lock(acceptors_mutex);
        #endif
        acceptors.insert(acceptor_pair);
    }
    
    tcp::resolver* pResolver = new tcp::resolver(io_service);
    copyable_unique_ptr<tcp::resolver> resolver(pResolver);
    
    pResolver->async_resolve(query,
        boost::bind(&Tcp_acceptor::handle_resolve, this, resolver,  acceptor_pair, 
            boost::asio::placeholders::error, 
            boost::asio::placeholders::iterator)); 
}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::stop_accept(int port) {
    #ifdef THREADSAFE
        boost::upgrade_lock<boost::shared_mutex> lock(acceptors_mutex);
    #endif
    std::pair<std::map<int, tcp::acceptor*>::iterator, std::map<int, tcp::acceptor*>::iterator> it_range = acceptors.equal_range(port);
    for (std::map<int, tcp::acceptor*>::iterator it = it_range.first; it != it_range.second;) {
        tcp::acceptor* acceptor = it->second;
        {
            #ifdef THREADSAFE
               boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            #endif
            acceptors.erase(it++);
            delete acceptor;
        }
    }
}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::handle_resolve(copyable_unique_ptr<tcp::resolver> resolver, std::pair<int, tcp::acceptor*> acceptor, const boost::system::error_code& error_code, tcp::resolver::iterator endpoint_iterator) {
    tcp::resolver::iterator end;
    if (!error_code) {
        start_acceptors(acceptor, endpoint_iterator);
    } else {
        error(error_code);
    }
}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::start_acceptors(std::pair<int, tcp::acceptor*>& acceptor, tcp::resolver::iterator endpoint_iterator) {
    tcp::resolver::iterator end;
    if (endpoint_iterator != end) {
        bool ipv6_only = false;
        boost::asio::ip::v6_only v6_only;

        {
            #ifdef THREADSAFE
                boost::shared_lock<boost::shared_mutex> lock(acceptors_mutex);
            #endif
            if (acceptor_still_valid(acceptor)) { //funny world the asynchronous world, the acceptor might have been invalidated (stop_accept) before it is bound to a port
                //the acceptor will remain valid until we release the lock
                boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
                boost::system::error_code open_error;
                acceptor.second->open(endpoint.protocol(), open_error);
                if (open_error) {
                    error(open_error);
                    return;
                }
                boost::system::error_code option_error;
                acceptor.second->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), option_error);
                if (option_error) {
                    error(option_error);
                    return;
                }                             
                boost::system::error_code bind_error;
                acceptor.second->bind(*endpoint_iterator, bind_error);
                if (bind_error) {
                    error(bind_error);
                    return;
                }
                boost::system::error_code listen_error;
                acceptor.second->listen(boost::asio::socket_base::max_connections, listen_error);
                if (listen_error) {
                    error(listen_error);
                    return;
                }
                start_accept(acceptor);
                if (endpoint_iterator->endpoint().address().is_v6()) {
                    acceptor.second->get_option(v6_only);
                    bool ipv6_only = v6_only.value();
                }
            }
        }
        
        //This part is basically to create an ipv4 listener if we have an ipv6 listener which only supports ipv6. Most modern
        //Os's can actually accept ipv4 conections with an ipv6 listener, will not run this code.
        if (ipv6_only) { //our connection is a ipv6 only, so open another resolver on the next endpoint probably ipv4 (dual stack approach)
            tcp::resolver::iterator next_endpoint_iterator = ++endpoint_iterator;
            tcp::resolver::iterator end;
            if (next_endpoint_iterator != end) { //we have another endpoint, probably ipv4
                std::pair<int, tcp::acceptor* > acceptor_pair = std::make_pair(acceptor.first, new tcp::acceptor(io_service));
                {
                    #ifdef THREADSAFE
                        boost::unique_lock<boost::shared_mutex> lock(acceptors_mutex);
                    #endif
                    acceptors.insert(acceptor_pair);
                }
                start_acceptors(acceptor_pair, next_endpoint_iterator);
            } else {} //we have no other endpoint. I guess this is the future and ipv4 compatability is no longer available
        }
    }
}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::start_accept(std::pair<int, tcp::acceptor*>& acceptor) {
    Conection_type* pConnection = new Conection_type(io_service, this);
    copyable_unique_ptr<Conection_type> connection(pConnection);
    acceptor.second->async_accept(pConnection->socket(),
        boost::bind(&Tcp_acceptor::handle_accept, this, acceptor, connection,
            boost::asio::placeholders::error));    
}

template <class Conection_type>
void Tcp_acceptor<Conection_type>::handle_accept(std::pair<int, tcp::acceptor*> acceptor, copyable_unique_ptr<Conection_type> connection, const boost::system::error_code& error_code) {
    if (!error_code) {
        Conection_type* pConnection = connection.release();
          pConnection->start();
        accepted(pConnection);        
    } else {
        error(error_code);
    }
    
    {
        #ifdef THREADSAFE
            boost::shared_lock<boost::shared_mutex> lock(acceptors_mutex);
        #endif
        if (acceptor_still_valid(acceptor)) {
            start_accept(acceptor);
        }
    }
}

#endif //TCP_ACCEPTOR
