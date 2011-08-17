#include "Tcp_connection.h"
#include "Connection_manager.h"
#include <ctime>
#include <deque>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>

#ifdef NETSTRING
#include <boost/lexical_cast.hpp>
#endif

//This is used as a sanity check for messages, largely to 
//prevent DOS attacks that work by sending very large  
//messages in order to starve the memory of the server
//larger than this are considered to be in error. 
//For length prefixed based protocols it will result in an
//error immediatly, when a prefix is received that exceeds
//this limit. Delimiter based protocol will read only wait
//this amount of bytes for a delimiter.

//Note the default 1GB here is excessive. As the protocols here
//are atleast copy once, every single connection can consume
//twice this amount in memory but it is difficult to guess
//what magnitude will be acceptable for your application.
//it is recommended you change this value by providing a 
//MAX_MESSAGE_SIZE compile time flag. 
#ifndef MAX_MESSAGE_SIZE
    #define MAX_MESSAGE_SIZE 1073741824 //1GB 
#endif

//only used by delimiter based protocols. It is the initial size of the read buffer.
//you can set this at compile time and it is wise to set it to a value that fits
//most messages. The code will not break if messages exceed this size but it will
//have to reallocate a larger buffer when it happens. 
#ifndef DEFAULT_BUFFER_SIZE
    #define DEFAULT_BUFFER_SIZE 1000
#endif

custom_network_error_category cat;

using namespace boost;
using namespace std;

  Tcp_connection::Tcp_connection(asio::io_service& io_service, Connection_manager* connection_manager_)
    :socket_(io_service) 
    ,m_connection_manager(connection_manager_)
    ,m_readbuf(0)
    #ifdef DELIMITER
        ,read_progress(0)
    #endif
  {}

  asio::ip::tcp::socket& Tcp_connection::socket() {
    return socket_;
  }

  void Tcp_connection::write(const char* data, size_t size) {
    Buffer buffer;
    #if defined(DELIMITER)
        buffer.size = size + sizeof(DELIMITER) - 1;
        buffer.data = (char*)malloc(buffer.size);
          memcpy(buffer.data, data, size); //copying data
          memcpy(buffer.data + size, DELIMITER, sizeof(DELIMITER)-1); //copying delimiter
    #elif defined(NETSTRING)
        //write data with length prefix (DEFAULT)
        std::string sizeStr(lexical_cast<std::string>(size));
        sizeStr += ":";
        buffer.size = size + sizeStr.length() + 1;
        buffer.data = (char*)malloc(buffer.size);
        memcpy(buffer.data, (char*)sizeStr.data(), sizeStr.length()); //copying size
        memcpy(buffer.data + sizeStr.length(), data, size); //copying data
        buffer.data[buffer.size-1] = ',';   
    #else
        //write data with length prefix (DEFAULT)
        buffer.size = size + HEADER_SIZE;
        buffer.data = (char*)malloc(buffer.size);
        uint32_t network_size = htonl(size); //to network byte order
        memcpy(buffer.data, (char*)&network_size, HEADER_SIZE); //copying size
        memcpy(buffer.data + HEADER_SIZE, data, size); //copying data
    #endif


    bool was_empty;
    {
        #ifdef THREADSAFE
            boost::lock_guard<boost::mutex> lock(write_queue_mutex);
        #endif
        was_empty = write_queue.empty();
        write_queue.push_back(buffer);
    }
    
    //at this point we have asserted the buffer was empty, so there is not a 
    //single active write. We no longer need the lock because we are alone
    //and no other thread will ever come to the conclusion that the write queue
    //is empty and start writing because it won't be empty until we are finished
    
    if (was_empty)
        asio::async_write(socket_, asio::buffer(buffer.data, buffer.size),
            bind(&Tcp_connection::handle_write, this,
                asio::placeholders::error));    
  }
  
  void Tcp_connection::received(const char* data, size_t size) {
    m_connection_manager->received(this, data, size);
  }  
  
  void Tcp_connection::error(const system::error_code& error_code) {
    m_connection_manager->error(this, error_code);
  }
  
  void Tcp_connection::start()
  {
    #ifdef DELIMITER
        start_read();
    #else
          start_read_header();
      #endif
  } 

  Tcp_connection::~Tcp_connection() {
    //if (socket_.is_open())
    //    socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
      for (std::deque<Buffer>::iterator it = write_queue.begin(); it != write_queue.end(); ++it) {
          free(it->data);
      }
      write_queue.clear();
    free(m_readbuf);
  }

  void Tcp_connection::handle_write(const system::error_code& error_code) {
    if (!error_code) {
        Buffer old_buffer;
        Buffer new_buffer;
        bool is_empty;
        {
            #ifdef THREADSAFE
                boost::lock_guard<boost::mutex> lock(write_queue_mutex);
            #endif
            if (!write_queue.empty()) {
                old_buffer = write_queue.front();
                write_queue.pop_front();
            }
            is_empty = write_queue.empty();
            if (!is_empty)
                new_buffer = write_queue.front();
        }
        
        //again we don't need the lock any more. Remember there is only one
        //thread writing at any given time. Either the queue was empty
        //and we take no further action so start_write can safely call an
        //async_write. Or the queue is not empty and start_write will still
        //keep all threads out.
        
          if (!is_empty)
            asio::async_write(socket_, asio::buffer(new_buffer.data, new_buffer.size),
                bind(&Tcp_connection::handle_write, this,
                    asio::placeholders::error));
                    
        free(old_buffer.data); //basically it doesn't matter when we free the buffer, so we let everything performance critical take priority
                          //not that I think freeing is that expensive but getting memory management out of the critical sections is 
                          //always a good idea.
    } else {
          error(error_code);
    }
  }
  
  //all the read code is already threadsafe on account that there is always
  //exactly one active reader. The only time a async_read call can start is at the
  //activation of the connection or if the last one finished.
  
  #if defined(DELIMITER) //code to read a messages seperated by a delimiter
      void Tcp_connection::start_read() {
        m_readbuf = (char*)malloc(DEFAULT_BUFFER_SIZE);
        read_buffer_size = DEFAULT_BUFFER_SIZE;
        delimiter_progress = 0;
        read_progress = 0;
        message_start = m_readbuf;
        socket_.async_read_some(asio::mutable_buffers_1(m_readbuf, DEFAULT_BUFFER_SIZE),
            bind(&Tcp_connection::handle_read, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
      } 
      
      void Tcp_connection::handle_read(const system::error_code& error, std::size_t bytes_transferred) {
          //this code is looks more complicated than it needs to be, this is because it implements
          //a semi-rotating buffer. semi because in order to deliver messages that occupy consecutive
          //memory, instead of wrapping around at the end, we move the current message to the front
          //when we are near the end.
          //
          //The algorithm will just walk forward in the readbuffer updating a pointer message_start 
          //and a counter read_progress to keep track of the current message we are parsing in the 
          //larger m_readbuf.
          //
          //The result is that we do not have to move or allocate memory unless one of the following conditions are met
          //
          //    1) There is less than 5% remaining in the buffer when a new message starts -> We move what data is remaining to the front (see A) 
          //       RATIONALE: there is not much space for new data so it is likely a move will be nescessary, not moving right now might result in a async_read_some
          //                  call with a small number of maximum bytes, which will hurt performance. Besides moving 5% of the buffer is cheap.
          //    2) The buffer is full and the current message occupy's more than 80% -> we enlarge it without moving because it is highly likely it won't fit (see B)
          //       RATIONALE: The buffer is full or even if it is not, moving everything to the front (which is expensive because it is a lot data), it will still 
          //                  be nearly full. So we enlarge the buffer with realloc. If we are lucky the buffer will not have to be moved. if not max 20% of the buffer
          //                  size in junk data will be copied needlessly.
          //    3) The buffer is full and the current message occupies less than 80% -> we move the message to the front of the buffer (see C)
          //       RATIONALE: We move data to the front of the buffer in the hopes the complete message will still fit that way after all. If not atleast we
          //                  will have a clean realloc in the next call without copying junk data.
          //
          //These boundaries are the result of guesses, not of performance analysis to decide upon the optimal value, which will be highly application dependent anyway 
          //
          size_t delimiter_size = sizeof(DELIMITER)-1;
          size_t read_until = read_progress + bytes_transferred;
          while (read_progress < read_until) {
              if (message_start[read_progress] == DELIMITER[delimiter_progress])
                  delimiter_progress += 1;
              else
                  delimiter_progress = 0;            
              if (delimiter_progress == delimiter_size) {
                  received(message_start, read_progress + 1 - delimiter_size);
                  delimiter_progress = 0;
                  message_start += read_progress + 1;
                  read_until -= read_progress + 1;
                  read_progress = 0;
                  size_t buffer_remaining = read_buffer_size - (message_start - m_readbuf);
                  if (buffer_remaining < ceil(read_buffer_size*0.05)) { //A
                      memmove(m_readbuf, message_start, read_until);
                      read_buffer_size = std::max<int>(DEFAULT_BUFFER_SIZE, read_until);
                      m_readbuf = (char*)realloc(m_readbuf, read_buffer_size); //possibly shrink
                      message_start = m_readbuf;
                  }
                  continue;
              }
              read_progress++;
          }
          
          if (((message_start - m_readbuf) + read_progress) == read_buffer_size) {
               if (read_progress > read_buffer_size*0.8) { //B, it's just too small
                   size_t message_start_offset = m_readbuf - message_start;
                   read_buffer_size *= 2;
                   m_readbuf = (char*)realloc(m_readbuf, read_buffer_size);
                   message_start = m_readbuf + message_start_offset;
               } else { //C, it's just full of junk
                   memmove(m_readbuf, message_start, read_progress);
                   message_start = m_readbuf;
               }
          }
          
          if (read_progress > MAX_MESSAGE_SIZE) {
              error(boost::system::error_code(MAX_MESSAGE_SIZE_EXCEEDED, cat));
              return;
          }
          
          socket_.async_read_some(asio::mutable_buffers_1(message_start + read_progress, read_buffer_size - (message_start - m_readbuf) - read_progress),
              bind(&Tcp_connection::handle_read, this,
                  asio::placeholders::error,
                  asio::placeholders::bytes_transferred));
                            
      }
  #elif defined(NETSTRING)
      const size_t max_header_size = lexical_cast<std::string>(MAX_MESSAGE_SIZE).length()+1;
  
      void Tcp_connection::start_read_header() {
        m_readbuf = (char*)malloc(max_header_size);
        header_progress = 0;
        length_string = "";
        socket_.async_read_some(asio::mutable_buffers_1(m_readbuf, max_header_size),
            bind(&Tcp_connection::handle_read_header, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
      }
      
      void Tcp_connection::handle_read_header(const system::error_code& error_code, std::size_t bytes_transferred) {
          // Netstrings prove suprisingly difficult to read asynchronously. 
          // The big problem is that our standard method of reading
          // just enough bytes to see how large it this doesn't work.
          // So we use async_read_some with max_header_size as maximum
          // But now we are in a situation where we might not have read enough
          // (async_read_some gives no guarantees). Or we might have read too many
          // a buffer of max_header_size can contain one or several complete 
          // small netstrings or parts thereof.
          
          //basically I see two options, read one byte at the time while reading the
          //header, this will allow for clean code.
          //or well use this
          
          if (!error_code) {     
              size_t read_until = header_progress + bytes_transferred;
              while (header_progress < read_until) {
                  if (m_readbuf[header_progress] == ':')
                      break;
                  else
                      length_string += m_readbuf[header_progress];
                  header_progress++;
              }
              
              bool valid_header = (m_readbuf[header_progress] == ':');
              size_t superfluous_data = read_until - header_progress - 1;
                         
              if (!valid_header) {
                  if (header_progress < max_header_size) { //we might not have enough data
                      socket_.async_read_some(asio::mutable_buffers_1(m_readbuf + header_progress, max_header_size - header_progress),
                          bind(&Tcp_connection::handle_read_header, this,
                              asio::placeholders::error,
                              asio::placeholders::bytes_transferred));                 
                  } else {
                      free(m_readbuf);
                      m_readbuf = 0;     
                      error(boost::system::error_code(NETSTRING_MALFORMED_HEADER, cat));
                  }
              } else {
                  try {
                      size_t length = lexical_cast<int>(length_string);
                      
                      if (length > MAX_MESSAGE_SIZE) {
                          error(boost::system::error_code(MAX_MESSAGE_SIZE_EXCEEDED, cat));
                          free(m_readbuf);
                          m_readbuf = 0;
                          return;
                      }
                          
                      //we align the beginning of the data with the beginning of the readbuffer TODO: this can avoided
                      memmove(m_readbuf, m_readbuf + header_progress + 1, superfluous_data); 
                      int still_to_read = length + 1 - superfluous_data;
                      
                      if (still_to_read <= 0) { //the whole message has allready has arrived
                          if (m_readbuf[length] == ',')
                              received(m_readbuf, length);
                          memmove(m_readbuf, m_readbuf + length + 1, -still_to_read);
                          header_progress = 0;
                          length_string = "";
                          handle_read_header(system::error_code(), -still_to_read); //call this again recursively for the next netstring
                          return;
                      }  
                      
                      m_readbuf = (char*)realloc(m_readbuf, length+2);
                      
                      asio::async_read(socket_, asio::mutable_buffers_1(m_readbuf + superfluous_data + 1, still_to_read),
                          bind(&Tcp_connection::handle_read_body, this, length + 1,
                              asio::placeholders::error));
                              
                  } catch (boost::bad_lexical_cast) {
                      free(m_readbuf);
                      m_readbuf = 0;     
                      error(boost::system::error_code(NETSTRING_MALFORMED_HEADER, cat));
                  }
              }
          } else {
               free(m_readbuf);
               m_readbuf = 0;     
               error(error_code);
          }
      }
      
      void Tcp_connection::handle_read_body(size_t len, const system::error_code& error_code) {
          if (!error_code) {
              if (m_readbuf[len] == ',') {
                  received(m_readbuf, len);
                  free(m_readbuf);
                  m_readbuf = 0;
                  start_read_header();
              } else {
                  free(m_readbuf);
                  m_readbuf = 0;
                  error(boost::system::error_code(NETSTRING_DELIMITER_NOT_FOUND, cat));
              }
          } else {
                free(m_readbuf);
                m_readbuf = 0;
                error(error_code);
          }
      }    
  #else //code to read a messages prefixed by the size as a binary 4 byte unsigned integer in network byte order (big endian) or netstring
      void Tcp_connection::start_read_header() {
        m_readbuf = (char*)malloc(HEADER_SIZE);
        asio::async_read(socket_, asio::mutable_buffers_1(m_readbuf, HEADER_SIZE),
            bind(&Tcp_connection::handle_read_header, this,
                asio::placeholders::error));
      }
      
      void Tcp_connection::handle_read_header(const system::error_code& error_code) {
          if (!error_code) {
              uint32_t msg_len = ntohl(*(uint32_t*)(&m_readbuf[0]));
              if (msg_len > MAX_MESSAGE_SIZE) {
                  error(boost::system::error_code(MAX_MESSAGE_SIZE_EXCEEDED, cat));
                  return;
              }
              // m_readbuf already contains the header in its first HEADER_SIZE
              // bytes. Expand it to fit in the body as well, and start async
              // read into the body.
              free(m_readbuf);
              m_readbuf = (char*)malloc(msg_len);
              asio::async_read(socket_, asio::mutable_buffers_1(m_readbuf, msg_len),
                  bind(&Tcp_connection::handle_read_body, this, msg_len,
                      asio::placeholders::error));
          } else {
                free(m_readbuf);
                m_readbuf = 0;
                error(error_code);
          }
      }
      
      void Tcp_connection::handle_read_body(size_t len, const system::error_code& error_code) {
          if (!error_code) {
              received(m_readbuf, len);
              free(m_readbuf);
              m_readbuf = 0;
              start_read_header();
          } else {
              free(m_readbuf);
              m_readbuf = 0;
              error(error_code);
          }
      }
  #endif
  


  

