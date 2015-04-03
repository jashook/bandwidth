////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Author: Jarret Shook
//
// Module: socket_test.cpp
//
// Time-period:
//
// 10-December-14: Version 1.0: Created
// 10-December-14: Version 1.0: Last Updated
//
// Notes:
//
// Requirements: POSIX sockets
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "socket.hpp"
#include "test.hpp"

#include <chrono>
#include <thread>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void test_socket_ctor_dtor()
{
   ev9::socket* socket;
   
   try
   {
      socket = new ev9::socket(7006);
      
      socket->bind();
      
      delete socket;
   }
   
   catch (std::exception& e)
   {
      throw std::runtime_error(TEST_INFORMATION + std::string("unable to finish socket constructor.") + e.what());
   }
}

void test_accepting_connection()
{
   ev9::socket* socket;
   
   try
   {
      socket = new ev9::socket(7000);
 
      socket->bind();
      socket->listen();
      socket->accept();
      socket->accept();
      
      delete socket;
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
}

void test_connect()
{
   ev9::socket* socket;
   
   // Sleep to allow the accept thread to finish.
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   
   try
   {
      socket = new ev9::socket(7000);
      
      socket->connect();
      
      delete socket;
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
}

void test_connect_different_ip()
{
   ev9::socket* socket;
   
   // Sleep to allow the accept thread to finish.
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   
   try
   {
      socket = new ev9::socket("127.0.0.1", 7000);
      
      socket->connect();
      
      delete socket;
      
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
   
}

void test_read()
{
   ev9::socket* socket;
   
   try
   {
      socket = new ev9::socket(7001);
      
      socket->bind();
      socket->listen();
      socket->accept();
      
      std::vector<char> input;
      
      socket->read(input);
      
      if (input.at(0) != 'a')
      {
         throw std::runtime_error(TEST_INFORMATION + "input does not equal a");
      }
   }
   
   catch (std::runtime_error& error)
   {
      throw std::runtime_error(error.what());
   }

}

void test_write()
{
   ev9::socket* socket;
   
   // Sleep to allow the accept thread to finish.
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   
   try
   {
      socket = new ev9::socket(7001);
      
      socket->connect();
      
      socket->write("a");
      
      delete socket;
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
}

void read_write_setup()
{
   ev9::socket* socket;
   
   // Sleep to allow the accept thread to finish.
   std::this_thread::sleep_for(std::chrono::milliseconds(10));
   
   try
   {
      socket = new ev9::socket(7002);
      
      socket->bind();
      socket->listen();
      socket->accept();
      
      std::vector<char> input_vector;
      
      socket->read(input_vector);
      
      socket->write_back("back");
      
      delete socket;
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
}

void test_read_write()
{
   ev9::socket* socket;
   
   try
   {
      socket = new ev9::socket(7002);
      
      std::thread connecting_thread(read_write_setup);
      
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      
      socket->connect();
      
      socket->write("a");
      
      std::vector<char> input_vector;
      
      socket->read_back(input_vector);
      
      std::string str;
      
      for (int index = 0; index < 4; ++index)
      {
         str.push_back(input_vector.at(index));
      }
  
      if (str != "back")
      {
         throw 0;
      }
      
      connecting_thread.join();
      
      delete socket;
   }
   
   catch (...)
   {
      throw std::runtime_error(TEST_INFORMATION);
   }
}

int main()
{
   ev9::test socket_test(0);

   socket_test.add_test(test_socket_ctor_dtor);
   socket_test.add_test(test_accepting_connection);
   socket_test.add_test(test_read);
   socket_test.add_test(test_connect);
   socket_test.add_test(test_connect_different_ip);
   socket_test.add_test(test_write);
   socket_test.add_test(test_read_write);
}