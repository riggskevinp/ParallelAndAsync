#include <iostream>
#include <thread>

#include "DataSharing.hpp"
#include "../Hello/basics.hpp"

void hello(){
    std::cout << "Hello most basic thread" << std::endl;
}

void hello_string(const std::string& s){
    std::cout << "Hello from " << s << std::endl;
}

void stack_pusher(AdaptedStack::threadsafe_stack<int> *st, int start, std::string name){
    for(int i = start; i < start + 300; i++){
        st->push(i);
        if(i % 10 == 0){
            std::cout << name << " " << i << std::endl;
        }
    }
}
void stack_reader(AdaptedStack::threadsafe_stack<int> *st, std::string name){
    for(int i = 1; i < 300; i++){
        if(i % 10 == 0){
            int j;
            st->pop(j);
            std::cout << name << " " << j << std::endl;
        }
    }
}

int main(){

    auto add_int_thread = thread_ownership::joining_thread(MutexExample::add_some_ints);
    auto check_list_thread = thread_ownership::joining_thread(MutexExample::check_for_some_ints);

    AdaptedStack::threadsafe_stack<int> shared_stack;
    auto stack_thread = thread_ownership::joining_thread(stack_pusher, &shared_stack, 1, "first");
    auto stack_thread2 = thread_ownership::joining_thread(stack_pusher, &shared_stack, 301, "second");
    auto stack_thread3 = thread_ownership::joining_thread(stack_reader, &shared_stack, "third");
    auto stack_thread4 = thread_ownership::joining_thread(stack_reader, &shared_stack, "fourth");

}