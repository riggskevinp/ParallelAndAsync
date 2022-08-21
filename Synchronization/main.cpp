#include <iostream>
#include <thread>
#include <random>

#include "Synchronization.hpp"

int main(){
    // Condition variable
    {
        std::cout << "Condition Variable" << std::endl;
        auto prep_thread = std::jthread(ConditionVariables::data_preparation_thread);
        auto proc_thread = std::jthread(ConditionVariables::data_processing_thread);
    }

    // Threadsafe queue with Condition variable
    {
        std::cout << "Threadsafe queue with Condition variable" << std::endl;
        auto prep_thread = std::jthread(ThreadSafe_Queue_ConditionVariables::data_preparation_thread);
        auto proc_thread = std::jthread(ThreadSafe_Queue_ConditionVariables::data_processing_thread);
    }



}