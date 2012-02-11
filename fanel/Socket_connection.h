//#ifndef FANEL_SOCKET_CONNECTION_H
//#define FANEL_SOCKET_CONNECTION_H

//Do not include this file directly, bad things will happen. It is included by Ssl_connection.h with USE_SSL flags enabled and included by Tcp_connection without those flags enabled

#include "Connection.h"
#include "copyable_unique_ptr.h"

#include <deque>
#include <boost/asio.hpp>
#include <memory>

#ifdef THREADSAFE
    #include <boost/thread/shared_mutex.hpp>
#endif

#ifdef USE_SSL
    #include <boost/asio/ssl.hpp>
#endif

using namespace boost;

#define MAX_MESSAGE_SIZE_EXCEEDED 1
#ifdef NETSTRING
    #define NETSTRING_MALFORMED_HEADER 2
    #define NETSTRING_DELIMITER_NOT_FOUND 3
#endif
class custom_network_error_category : public boost::system::error_category {
public:
  const char *name() const { return "custom_network_error"; }
  std::string message(int ev) const { 
    switch (ev) {
        case 0: return "Succes";
        case MAX_MESSAGE_SIZE_EXCEEDED: return "Maximum message size exceeded";
        #ifdef NETSTRING
            case NETSTRING_MALFORMED_HEADER: return "Malformed netstringheader";
            case NETSTRING_DELIMITER_NOT_FOUND: return "Netstring Delimiter not found";
        #endif
        default: return "Inkown error";
    }
  }
};

/** \brief Socket_connection with an asynchronous message passing infrastructure build on top.
 * 
 * This socket can pass messages over tcp. Socket_connection is handled by the XXX_connector
 * or XXX_acceptor classes. Once connected you can send messages by using the write. And complete
 * messages will call the received which you can override or by default calls the same
 * function on the Socket_connection_manager. Errors will call the error function which you 
 * can override but again calls by default a function with the same name on Socket_connection_manager.
 * You must call start after the socket is connected to start reading.
 *
 * Messages are passed over the network preceded by their size as a 4 byte unsigned integer 
 * encoded in network byte order (big endian), htonl/ntohl are used for platform independent 
 * encoding and decoding of this integer.
 */

template<class SocketType>
class Socket_connection : public Connection {
  static custom_network_error_category cat;

  struct Buffer {
      char* data;
      size_t size;
  };

  public:
    typedef SocketType connection_type;
  
    #ifdef USE_SSL
        Socket_connection(asio::io_service& io_service, asio::ssl::context& context, Connection_manager& manager);
    #else
        Socket_connection(asio::io_service& io_service, Connection_manager& manager);
    #endif
    
    virtual ~Socket_connection();
  
    SocketType& socket();
  
    //start reading 
    void start();
    //write data
    void write(const char* data, size_t size);

  private:

    void start_write();
    void handle_write(const system::error_code& error, std::weak_ptr<bool> alive);
    
    #ifdef DELIMITER
        void start_read();
        void handle_read(const system::error_code& error, std::size_t bytes_transferred, std::weak_ptr<bool> alive);
        size_t read_progress;
        size_t read_buffer_size;
        size_t delimiter_progress;
        char* message_start;
    #elif NETSTRING
        void start_read_header();
        void handle_read_header(const system::error_code& error, std::size_t bytes_transferred, std::weak_ptr<bool> alive);
        std::string length_string;
        size_t header_progress;
        void handle_read_body(size_t size, const system::error_code& error, std::weak_ptr<bool> alive);
    #elif STREAMING
        void start_read();
        void handle_read(const system::error_code& error, std::size_t bytes_transferred, std::weak_ptr<bool> alive);
    #else //default size_prefix
        void start_read_header();
        void handle_read_header(const system::error_code& error, std::weak_ptr<bool> alive);
        void handle_read_body(size_t size, const system::error_code& error, std::weak_ptr<bool> alive);
    #endif

    SocketType socket_;
    char* m_readbuf;
    std::deque<Buffer> write_queue;
	std::shared_ptr<bool> still_alive;

    #ifdef THREADSAFE
    boost::mutex write_queue_mutex;
    #endif

};

//#endif //FANEL_SOCKET_CONNECTION_H
