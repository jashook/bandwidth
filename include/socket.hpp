////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Author: Jarret Shook
//
// Module: socket.hpp
//
// Time-period:
//
// Dec 11, 2014: Version 1.0: Created
// Dec 11, 2014: Version 1.0: Last Updated
//
// Notes:
//
// Requirements: POSIX threads
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdexcept>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace ev9 {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 256

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class socket
{
   public:  // Constructor | Destructor

      socket() { _ctor(); }
      ~socket() { _dtor(); }

   public:  // Public member functions

      void accept() { _accept(); }
      void bind() { _bind(); }
      void listen() { _listen(); }
      void read(std::vector<char>& buffer) { _read(buffer); }
      void write(const char* const message) { _write(message); }
      void write(const std::string& message) { _write(message); }

   private: // Private member functions

      void _accept()
      {
         socklen_t client_length = sizeof(_m_client_address);

         _m_accepted_fd = ::accept(_m_socket_fd, (sockaddr *)&_m_client_address, &client_length);

         // -1 on failure
         if (_m_accepted_fd < 0)
         {
            throw std::runtime_error("Unable to connect to socket.");
         }
      }

      void _bind()
      {
         if (::bind(_m_socket_fd, (const sockaddr*)&_m_server_address, sizeof(_m_server_address) < 0))
         {
            throw std::runtime_error("Unable to bind to port");
         }
      }

      void _ctor()
      {
         // AF_INET: Internet domain of the computer
         // SOCK_STREAM: TCP as opposed to UDP
         // 0: Choose TCP for the transfer protocol
         _m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);

         // -1 if failed
         if (_m_socket_fd < 0)
         {
            throw std::runtime_error("Unable to open socket.");
         }

         memset(&_m_server_address, 0, sizeof(_m_server_address));
         memset(&_m_client_address, 0, sizeof(_m_client_address));

         _m_server_address.sin_family = AF_INET;
         _m_server_address.sin_addr.s_addr = INADDR_ANY;
         _m_server_address.sin_port = _m_port_number;

         _m_accepted_fd = 0;
         _m_amount_read = 0;
      }

      void _dtor()
      {
         if (_m_accepted_fd) close(_m_accepted_fd);
         if (_m_socket_fd) close(_m_socket_fd);
      }

      void _listen()
      {
         // 5 is the max amount of waiting connections
         ::listen(_m_socket_fd, 5);
      }

      void _read(std::vector<char>& buffer)
      {
         do
         {
            memset(_m_buffer, 0, sizeof(char) * BUFFER_SIZE);

            _m_amount_read = ::read(_m_accepted_fd, _m_buffer, BUFFER_SIZE - 1);

            if (_m_amount_read < 0)
            {
               throw std::runtime_error("Error reading from the connection");
            }

            for(int index = 0; index < _m_amount_read; ++_m_amount_read)
            {
               buffer.push_back(_m_buffer[index]);
            }

         } while (_m_amount_read == BUFFER_SIZE - 1);
      }

      void _write(const char* const message)
      {
         int error_code = ::write(_m_accepted_fd, message, strlen(message));

         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }

      void _write(const std::string& message)
      {
         int error_code = ::write(_m_accepted_fd, message.c_str(), message.size());

         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }

   private: // Member Variables

      char _m_buffer[BUFFER_SIZE];

      int _m_accepted_fd;
      int _m_socket_fd;
      std::size_t _m_amount_read;
      std::size_t _m_port_number;

      sockaddr_in _m_server_address;
      sockaddr_in _m_client_address;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // end of namespace ev9

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif /* __SOCKET_HPP__ */

////////////////////////////////////////////////////////////////////////////////
// end of socket.hpp
////////////////////////////////////////////////////////////////////////////////
