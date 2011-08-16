#ifndef CONNECTION_H
#define CONNECTION_H

#include <ctime>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef THREADSAFE
#include <boost/thread/shared_mutex.hpp>
#endif

#include "copyable_unique_ptr.h"

#define HEADER_SIZE 4

class Connection_manager;

using namespace boost;

#define MAX_MESSAGE_SIZE_EXCEEDED 1
class custom_network_error_category : public boost::system::error_category { 
public:
  const char *name() const { return "custom_network_error"; } 
  std::string message(int ev) const { 
    switch (ev) {
        case 0: return "Succes";
        case MAX_MESSAGE_SIZE_EXCEEDED: return "Maximum message size exceeded";
    }
  } 
};

/** \brief Tcp connection with an asynchronous message passing infrastructure build on top.
 * 
 * This socket can pass messages over tcp. Connection is handled by the Tcp_connector
 * or tcp acceptor. Once connected you can send messages by using the write. And complete
 * messages will call the received which you can override or by default calls the same
 * function on the Connection_manager. Errors will call the error function which you 
 * can override but again calls by default a function with the same name on Connection_manager.
 * You must call start after the socket is connected to start reading.
 *
 * Messages are passed over the network preceded by their size as a 4 byte unsigned integer 
 * encoded in network byte order (big endian), htonl/ntohl are used for platform independent 
 * encoding and decoding of this integer.
 */

class Tcp_connection {

  struct Buffer {
      char* data;
      size_t size;
  };

  public:
    Tcp_connection(asio::io_service& io_service, Connection_manager* manager);
    virtual ~Tcp_connection();
  
    asio::ip::tcp::socket& socket();
  
    //start reading 
    void start();
    //write data
    void write(const char* data, size_t size);
    //overload this function to receive all data read
    virtual void received(const char* data, size_t size);
    //overload this functions to receive errors, like disconnect
    virtual void error(const system::error_code& error_code);
  
  private:

    void start_write();
    void handle_write(const system::error_code& error);
    
    #ifdef DELIMITER
    void start_read();
    void handle_read(const system::error_code& error, std::size_t bytes_transferred );
    size_t read_progress;
    size_t read_buffer_size;
    size_t delimiter_progress;
    char* message_start;
    #else    
    void start_read_header();
    void handle_read_header(const system::error_code& error);
    void start_read_body(unsigned msg_len);
    void handle_read_body(size_t size, const system::error_code& error);
    #endif

    asio::ip::tcp::socket socket_;
    std::deque<Buffer> write_queue;
    
    
    Connection_manager* m_connection_manager;

    #ifdef THREADSAFE
    boost::mutex write_queue_mutex;
    #endif

    char* m_readbuf;
};

#endif //CONNECTION_H
