////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Author: Jarret Shook
//
// Module: tester.hpp
//
// Time-period:
//
// Dec 11, 2014: Version 1.0: Created
// Dec 11, 2014: Version 1.0: Last Updated
//
// Notes:
//
// Requirements: c++11
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef __TESTER_HPP__
#define __TESTER_HPP__

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "error.hpp"

#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace ev9 {
    
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
   
class tester
{
   public:  // Type definitions
   
      typedef std::function<void()> test;
   
   public: // Private Inner Class
      
      class test_task
      {
      public:  // Constructor | Destructor
         
         test_task() { _ctor(); }
         ~test_task() { _dtor(); }
         
      public:  // Public Member Functions
         
         void add_to_error_list(std::vector<error*>& error_list) { _add_to_error_list(error_list); }
         void finish() { _finish(); }
         void start() { _start(); }
   
         static void run(test_task* test) { test->_run(); }
         
      private: // Private Member Functions
         
         void  _add_to_error_list(std::vector<error*>& error_list)
         {
            for (ev9::error* err : _m_error_list)
            {
               error_list.push_back(err);
            }
         }
         
         void _ctor()
         {
            _m_thread = nullptr;
         }
         
         void _dtor()
         {
            for (error* err : _m_error_list)
            {
               delete err;
            }
         }
         
         void _finish()
         {
            if (_m_thread != nullptr)
            {
               _m_thread->join();
               
               delete _m_thread;
            }
         }
         
         void _start()
         {
            _m_thread = new std::thread(run, this);
         }
         
         void _run()
         {
            for (std::size_t i = _m_start; i < _m_end; ++i)
            {
               test* current_test = _m_task_list[i];
               
               try
               {
                  (*current_test)();
               }
               
               catch(std::exception& e)
               {
                  _m_error_list.push_back(new error(e.what()));
               }
               
            }
         }
         
      public: // Member Variables
         
         std::vector<test*> _m_task_list;
         std::size_t _m_start;
         std::size_t _m_end;
         
         std::vector<error*> _m_error_list;
         
         std::thread* _m_thread;
         
         
      }; // end of class(test_task)
   
   public:  // Constructor | Destructor
   
      tester(std::size_t threads = 0) { _ctor(threads); }
   
      virtual ~tester() { _dtor(); }
   
   private:  // Constructor
   
      tester(std::size_t threads, void* object) { _ctor(threads, object); }
   
   public:  // Public Member Functions
    
   void collect_results() { _collect_results(); }
   void output_results() { _output_results(); }
   void run(const test& function) { _run(function); }
   void start_tests() { }
    
   private: // Private Member Functions
   
      void _collect_results()
      {
         for (std::size_t index = 0; index < m_test_object->_m_thread_count; ++index)
         {
            test_task& task = m_test_object->_m_tasks[index];
            
            task.finish();
            
            task.add_to_error_list(_m_error_list);
         }
      }
   
      void _ctor(std::size_t threads)
      {
         _m_tasks = nullptr;
         _m_thread_count = 0;
         _m_total_tests = 0;
         
         if (tester::m_test_object == nullptr)
         {
             tester::m_test_object = new tester(threads, nullptr);
         }
      
      }
   
      void _ctor(std::size_t threads, void* object)
      {
         if (threads == 0)
         {
            threads = std::thread::hardware_concurrency();
         }
         
         _m_thread_count = threads;
         _m_total_tests = 0;
         
         _m_tasks = new test_task[threads];
      }
   
   
      void _dtor()
      {
         if (this == m_test_object)
         {
            return;
         }
         
         _run_tests();
         
         delete [] _m_tasks;
         
         for (test* current_test : m_test_object->_m_task_list)
         {
            delete current_test;
         }
         
         delete m_test_object;
      }
   
      void _output_results()
      {
         for (error* err : m_test_object->_m_error_list)
         {
            err->print();
         }
         
         std::size_t errors = _m_error_list.size();
         
         // Print the time processing took
         std::chrono::duration<double> elapsed_seconds = _m_end_time - _m_start_time;
         
         double elapsed_time = elapsed_seconds.count();
         
         std::printf("--- Total Tests: %lu, Passed: %lu, Failed: %lu\nTested with %lu threads ", _m_total_tests, _m_total_tests - errors, errors, _m_thread_count);
         
         if (elapsed_time < 1)
         {
            elapsed_time *= 1000;
            
            std::cout << "in " << elapsed_time << " milliseconds." << std::endl;
         }
         
         else
         {
            std::cout << "in " << elapsed_time << " seconds." << std::endl;
         }
      }
   
      void _run(const test& current_test)
      {
         test* current_test_copy = new test(current_test);
         
         ++m_test_object->_m_total_tests;
         
         m_test_object->_m_task_list.push_back(current_test_copy);
         
      }
   
      static void _run_tests()
      {
         m_test_object->_m_start_time = std::chrono::system_clock::now();
         
         m_test_object->_start_tests();
         m_test_object->_collect_results();
         
         m_test_object->_m_end_time = std::chrono::system_clock::now();
         
         m_test_object->_output_results();
      }
   
      void _start_tests()
      {
         std::size_t offset = (m_test_object->_m_task_list.size() / _m_thread_count);
         std::size_t remainder = (m_test_object->_m_task_list.size() % _m_thread_count);

         std::size_t shift_number = 0;

         for (int i = 0; i < _m_thread_count && i < _m_task_list.size(); ++i)
         {
            m_test_object->_m_tasks[i]._m_task_list = _m_task_list;
            m_test_object->_m_tasks[i]._m_start = i * offset + shift_number;

            std::size_t end = (i * offset + shift_number) + offset;

            if (remainder)
            {
               ++end; // add an extra to account for left overs
               --remainder;
               ++shift_number;
            }
            
            if (end > m_test_object->_m_task_list.size())
            {
               end = m_test_object->_m_task_list.size();
            }
            
            m_test_object->_m_tasks[i]._m_end = end;
            
            m_test_object->_m_tasks[i].start();
            
         }
         
      }
    
   private: // Member Variables
   
      static tester* m_test_object;
   
      std::chrono::time_point<std::chrono::system_clock> _m_start_time;
      std::chrono::time_point<std::chrono::system_clock> _m_end_time;
   
      std::size_t _m_total_tests;
      std::size_t _m_thread_count;
      
      test_task* _m_tasks;
      
      std::vector<error*> _m_error_list;
      std::vector<test*> _m_task_list;
   
}; // end of class(tester)
   
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // end of namespace(ev9)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // __TESTER_HPP__

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////