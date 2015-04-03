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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#if _WIN32

#include <windows.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#else // UNIX

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#endif

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

      socket(std::size_t port) { _ctor(port); }
      socket(const char* const ip, std::size_t port) { _ctor(port, ip); }
      socket(const std::string& host_name, std::size_t port) { _ctor(port, host_name); }
      ~socket() { _dtor(); }

   public:  // Public member functions

      void accept() { _accept(); }
      void bind() { _bind(); }
      void close() { _close(); }
      void connect() { _connect(); }
      void listen() { _listen(); }
      void read(std::vector<char>& buffer) { _read(buffer); }
      void read_back(std::vector<char>& buffer) { _read_back(buffer); }
      void write(const char* const message) { _write(message); }
      void write(const std::string& message) { _write(message); }
      void write_back(const char* const message) { _write_back(message); }
      void write_back(const std::string& message) { _write_back(message); }

   private: // Private member functions

      void _accept()
      {
         #if _WIN32
            int client_length = sizeof(sockaddr_in);
         #else
            socklen_t client_length = sizeof(sockaddr_in);
         #endif

         _m_accepted_fd = ::accept(_m_socket_fd, (sockaddr *)&_m_client_address, &client_length);

         // -1 on failure
         if (_m_accepted_fd < 0)
         {
            std::string err = "Unable to connect to socket - error number: ";
            
            err += errno;
            
            err += " -- ";
            
            #if _WIN32
               char buffer[256];

               err += strerror_s(buffer, 256, errno);
            #else
               err += strerror(errno);
            #endif
            
            throw std::runtime_error(err);
         }
      }

      void _bind()
      {
         auto return_value = ::bind(_m_socket_fd, (const sockaddr*)&_m_server_address, sizeof(_m_server_address));
         
         if (return_value != 0)
         {
            throw std::runtime_error("Unable to bind to port");
         }
      }
   
      void _ctor(std::size_t port, const std::string& host_name)
      {
         hostent* host_ent;
         
         host_ent = ::gethostbyname(host_name.c_str());
         
         _ctor(port, host_ent->h_name);
      }

      void _ctor(std::size_t port, const char* const ip = nullptr)
      {
         #if _WIN32
            WSADATA w;

            int error = WSAStartup(0x0202, &w);

            if (w.wVersion != 0x0202) //Wrong Winsock version?
            {
               WSACleanup();

               throw std::runtime_error("[ev9::socket.hpp::_ctor()::140] - Incorrect WSA Version");
            }
         #endif

         _m_port_number = port;
         
         _m_ip_address = nullptr;
         
         if (ip != nullptr)
         {
            _m_ip_address = new char[std::strlen(ip) + 1];

            #if _WIN32
               strcpy_s(_m_ip_address, std::strlen(ip) + 1, ip);
            #else
               std::strcpy(_m_ip_address, ip);
            #endif
         }

         else
         {
            const char* loopback = "127.0.0.1";

            _m_ip_address = new char[std::strlen(loopback) + 1];

            #if _WIN32
               strcpy_s(_m_ip_address, std::strlen(loopback) + 1, loopback);
            #else
               std::strcpy(_m_ip_address, ip);
            #endif
         }

         // AF_INET: Internet domain of the computer
         // SOCK_STREAM: TCP as opposed to UDP
         // 0: Choose TCP for the transfer protocol
         _m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);

         // -1 if failed
         if (_m_socket_fd < 0)
         {
            throw std::runtime_error("Unable to open socket.");
         }

         memset(&_m_server_address, 0, sizeof(sockaddr_in));
         memset(&_m_client_address, 0, sizeof(sockaddr_in));

         _m_server_address.sin_family = AF_INET;
         _m_server_address.sin_addr.s_addr = INADDR_ANY;
         _m_server_address.sin_port = _m_port_number;

         _m_accepted_fd = 0;
         _m_amount_read = 0;
      }
   
      void _close()
      {
         #if _WIN32
            ::closesocket(_m_socket_fd);
         #else
            ::close(_m_socket_fd);
         #endif
      }
   
      void _connect()
      {
         // IPv4
         _m_server_address.sin_family = AF_INET;
         _m_server_address.sin_port = _m_port_number;
         
         if (_m_ip_address != nullptr)
         {
            #if _WIN32
               int status = _m_server_address.sin_addr.s_addr = ::inet_addr(_m_ip_address);
            #else
               int status = ::inet_pton(AF_INET, _m_ip_address, &_m_server_address.sin_addr);
            #endif

            if (status < 0)
            {
               throw std::runtime_error("Error cannot convert the ip address");
            }
         }
         
         auto return_value = ::connect(_m_socket_fd, (struct sockaddr *) &_m_server_address, sizeof(_m_server_address));
         
         if (return_value < 0)
         {
            std::cout << WSAGetLastError() << std::endl;

            throw std::runtime_error("Error cannot connect to the address");
         }

      }

      void _dtor()
      {
         if (_m_accepted_fd) close();
         if (_m_socket_fd) close();
         if (_m_ip_address) delete [] _m_ip_address;
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
            
            #if _WIN32
               _m_amount_read = (int)::recv(_m_accepted_fd, _m_buffer, BUFFER_SIZE - 1, 0);
            #else
               _m_amount_read = (int)::read(_m_accepted_fd, _m_buffer, BUFFER_SIZE - 1);
            #endif

            if (_m_amount_read < 0)
            {
               throw std::runtime_error("Error reading from the connection");
            }

            for(int index = 0; index < _m_amount_read; ++index)
            {
               buffer.push_back(_m_buffer[index]);
            }

         } while (_m_amount_read == BUFFER_SIZE - 1);
      }
   
      void _read_back(std::vector<char>& buffer)
      {
         do
         {
            memset(_m_buffer, 0, sizeof(char) * BUFFER_SIZE);
            
            #if _WIN32
               _m_amount_read = (int)::recv(_m_socket_fd, _m_buffer, BUFFER_SIZE - 1, 0);
            #else
               _m_amount_read = (int)::read(_m_accepted_fd, _m_buffer, BUFFER_SIZE - 1);
            #endif
            
            if (_m_amount_read < 0)
            {
               throw std::runtime_error("Error reading from the connection");
            }
            
            for(int index = 0; index < _m_amount_read; ++index)
            {
               buffer.push_back(_m_buffer[index]);
            }
            
         } while (_m_amount_read == BUFFER_SIZE - 1);
      }
   
      void _write(const char* const message)
      {
         #if _WIN32
            auto error_code = ::send(_m_socket_fd, message, (std::size_t)strlen(message), 0);
         #else
            auto error_code = ::write(_m_socket_fd, message, (std::size_t)strlen(message));
         #endif
      
         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }

      void _write(const std::string& message)
      {
         #if _WIN32
            auto error_code = ::send(_m_socket_fd, message.c_str(), message.size(), 0);
         #else
            auto error_code = ::write(_m_socket_fd, message.c_str(), message.size());
         #endif

         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }

      void _write_back(const char* const message)
      {
         #if _WIN32
            auto error_code = ::send(_m_accepted_fd, message, (std::size_t)strlen(message), 0);
         #else
            auto error_code = ::write(_m_accepted_fd, message, (std::size_t)strlen(message));
         #endif

         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }
   
      void _write_back(const std::string& message)
      {
         #if _WIN32
            auto error_code = ::send(_m_accepted_fd, message.c_str(), message.size(), 0);
         #else
            auto error_code = ::write(_m_accepted_fd, message.c_str(), message.size());
         #endif
         
         if (error_code < 0)
         {
            throw std::runtime_error("Error writing to the connection");
         }
      }
   
   private: // Member Variables

      char _m_buffer[BUFFER_SIZE];
      char* _m_ip_address;

      #if _WIN32
         SOCKET _m_accepted_fd;
         SOCKET _m_socket_fd;
      #else
         int _m_accepted_fd;
         int _m_socket_fd;
      #endif
         int _m_amount_read;
      std::size_t _m_port_number;

      #if _WIN32
         SOCKADDR_IN _m_server_address;
         SOCKADDR_IN _m_client_address;
      #else
         sockaddr_in _m_server_addres;
         sockaddr_in _m_client_address;
      #endif 
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
