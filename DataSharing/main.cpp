#include <iostream>
#include <thread>
#include <random>

#include "DataSharing.hpp"

void hello(){
    std::cout << "Hello most basic thread" << std::endl;
}

void hello_string(const std::string& s){
    std::cout << "Hello from " << s << std::endl;
}

void stack_pusher(AdaptedStack::threadsafe_stack<int> *st, int start, const std::string& name){
    for(int i = start; i < start + 300; i++){
        st->push(i);

        if(i % 10 == 0){
            std::string n = name;
            n = "d";
            std::cout << name << " " << i << std::endl;
        }
    }
}
void stack_reader(AdaptedStack::threadsafe_stack<int> *st, const std::string& name){
    for(int i = 1; i < 300; i++){
        if(i % 10 == 0){
            std::string n = name;
            n = "d";
            int j;
            try{
                st->pop(j);
            } catch (std::exception& e){
                std::cout << e.what() << std::endl;
            }
            std::cout << name << " " << j << std::endl;
        }
    }
}

void cache_interaction(SharedDataProtection::Dns_cache* cache){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution dist(1, 10);
    for(int i = 0; i < 10; i++){
        int x = dist(mt);
        cache->update_or_add_entry(std::to_string(x), SharedDataProtection::Dns_entry(std::to_string(i)));
    }
}

void cache_read(SharedDataProtection::Dns_cache* cache){
    for(int i = 1; i <= 10; i++){
        std::cout << "Cache entry value for " << std::to_string(i) << " : " << cache->find_entry(std::to_string(i)).to_string() << std::endl;
    }
}

int main(){


    auto ycomp_a = DeadlockProblem::Y(22);
    auto ycomp_b = DeadlockProblem::Y(33);
    auto ycomp_c = DeadlockProblem::Y(22);
    std::cout << "a == a: " << (ycomp_a == ycomp_a) << std::endl;
    std::cout << "a == b: " << (ycomp_a == ycomp_b) << std::endl;
    std::cout << "a == c: " << (ycomp_a == ycomp_c) << std::endl;

    auto add_int_thread = std::jthread(MutexExample::add_some_ints);
    auto check_list_thread = std::jthread(MutexExample::check_for_some_ints);

    AdaptedStack::threadsafe_stack<int> shared_stack;
    auto stack_thread = std::jthread(stack_pusher, &shared_stack, 1, "first");
    auto stack_thread2 = std::jthread(stack_pusher, &shared_stack, 301, "second");
    auto stack_thread3 = std::jthread(stack_reader, &shared_stack, "third");
    auto stack_thread4 = std::jthread(stack_reader, &shared_stack, "fourth");

    SharedDataProtection::foo();

    auto cache = SharedDataProtection::Dns_cache();

    auto cache_thread = std::jthread(cache_interaction, &cache);
    auto cache_thread2 = std::jthread(cache_interaction, &cache);
    auto cache_reader = std::jthread(cache_read, &cache);

}