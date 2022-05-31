#include "thread.h"
#include "cpu.h"
#include "traits.h"
#include "debug.h"
#include <iostream>
#include <ctime> 
#include <chrono>

#include <list>

__BEGIN_API

int Thread::_thread_counter = 0;
Ordered_List<Thread> Thread::_ready = Ready_Queue();
Thread Thread::_main = Thread();
Thread Thread::_dispatcher = Thread();
Thread* Thread::_running = nullptr;
CPU::Context Thread::_main_context = CPU::Context();


int Thread::switch_context(Thread * prev, Thread * next) {
    db<Thread>(TRC) << "Thread::switch_context(prev=" << prev->id() << ",next=" << next->id() << ")" << "\n";
    if (!prev->context() || !next->context())
        return -1;
    
    _running = next;
    next->_state = RUNNING;
    CPU::switch_context(prev->context(), next->context());
    return 0;
}

void Thread::thread_exit(int exit_code) {
    db<Thread>(TRC) << "Thread::thread_exit(exit_code=" << exit_code << ", id=" << _id << ")\n";
    _state = FINISHING;

    yield();
}

int Thread::id() {
    return _id;
}

void Thread::init(void (*main)(void *)) {
    db<Thread>(TRC) << "Thread::init(main=" << main << ")" << "\n";

    Thread main_thread = Thread(main, (void *) "main"); 
    Thread::_main = main_thread;
    Thread::_main_context = *Thread::_main._context;
    Thread::_ready.remove(&main_thread._link);

    Thread dispatcher_thread = Thread(dispatcher);
    Thread::_dispatcher = dispatcher_thread;
    Thread::_ready.remove(&dispatcher_thread._link);

    Thread::_running = &Thread::_main;
    Thread::_main_context.load();
}

void Thread::dispatcher() {
    db<Thread>(TRC) << "Thread::dispatcher()" << "\n";
    while(!Thread::_ready.empty()) {
        Thread* next = Thread::_ready.remove()->object();
        db<Thread>(TRC) << "Dispatcher: running=" << Thread::_running->_id << " next="<< next->_id <<"\n";
        Thread::print_list();
        Thread::_dispatcher._state = READY;

        switch_context(&Thread::_dispatcher, next);
        if(next->_state == FINISHING) {
            Thread::_ready.remove(&next->_link);
        }
    }
    Thread::_dispatcher.thread_exit(0);
    switch_context(&Thread::_dispatcher, &Thread::_main);
}

void Thread::yield() {
    Thread* prev = _running;
    db<Thread>(TRC) << "Thread id="<< prev->_id << " yield()" << "\n";

    if(prev->_id != Thread::_main._id) {
        int now = std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        prev->_link.rank(now);
        Thread::_ready.insert(&prev->_link);
    }
    switch_context(prev, &_dispatcher);
}

Thread::~Thread() {
    db<Thread>(TRC) << "~Thread() id="<< this->_id << "\n";
    Thread::_thread_counter--;
    
    if(Thread::_thread_counter == 0) {
        db<Thread>(TRC) << "~Thread() id="<< this->_id << " is main thread" << "\n";
    } else {
        if (_context)
            delete _context;
    }
}

void Thread::print_list() {
    db<Thread>(TRC) << "List Size=" << Thread::_ready.size() << "\n";
    for (int i = 0; i < Thread::_ready.size(); i++) {
        db<Thread>(TRC) << "list id=" << Thread::_ready.get(i) << "\n"; //->object()->_id
    }
}

__END_API