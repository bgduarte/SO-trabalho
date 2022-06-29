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
Ordered_List<Thread> Thread::_waiting = Ordered_List<Thread>();
Thread Thread::_main = Thread();
Thread Thread::_dispatcher = Thread();
Thread* Thread::_running = nullptr;
CPU::Context Thread::_main_context = CPU::Context();


int Thread::switch_context(Thread * prev, Thread * next) {
    db<Thread>(TRC) << "Thread::switch_context(prev=" << prev->id() << ",next=" << next->id() << ")" << "\n";
    if (prev != next) {
        _running = next;
        next->_state = RUNNING;
        CPU::switch_context(prev->context(), next->context());
    }
    return 0;
}

void Thread::thread_exit(int exit_code) {
    db<Thread>(TRC) << "Thread::thread_exit(exit_code=" << exit_code << ", id=" << _id << ")\n";
    _exit_code = exit_code;
    _state = FINISHING;
    if (_joiner_thread)
        _joiner_thread->resume();
    yield();
}

int Thread::id() {
    return _id;
}

void Thread::init(void (*main)(void *)) {
    db<Thread>(TRC) << "Thread::init(main=" << main << ")" << "\n";

    new (&_main) Thread(main, (void *) "main"); 
    new (&_dispatcher) Thread((void (*) (void *)) &Thread::dispatcher, (void *) NULL);
    new (&_main_context) CPU::Context();
    _ready.insert(&_dispatcher._link);
    _running = &_main;
    _main._context->load();
}

void Thread::dispatcher() {
    db<Thread>(TRC) << "Thread::dispatcher()" << "\n";
    while(!_ready.empty() && _ready.head()->object() != &_dispatcher) {
        Thread* next = Thread::_ready.remove()->object();
        db<Thread>(TRC) << "Dispatcher: running=" << Thread::_running->_id << " next="<< next->_id <<"\n";
        _dispatcher._state = READY;
        _ready.insert(&_dispatcher._link);
    
        switch_context(&Thread::_dispatcher, next);

        if (next->_state == FINISHING)
            _ready.remove(next);
    }
    _dispatcher._state = FINISHING;
    _ready.remove(&_dispatcher);
    switch_context(&_dispatcher, &_main);
}

void Thread::yield() {
    Thread* prev = _running;
    Thread* next = _ready.remove()->object();
    db<Thread>(TRC) << "Thread id="<< prev->_id << " yield()" << "\n";

    if (prev != &_main && prev != &_dispatcher && prev->_state != FINISHING && prev->_state != WAITING) {
        prev->_state = READY;
        int now = std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        prev->_link.rank(now);
    }
    if (prev != &_main && prev->_state != WAITING)
        _ready.insert(&prev->_link);

    switch_context(prev, next);
}

Thread::~Thread() {
    db<Thread>(TRC) << "~Thread() id="<< this->_id << "\n";
    Thread::_thread_counter--;
    _ready.remove(this);
    delete _context;
}

int Thread::join() {
    db<Thread>(TRC) << "Thread " << _running->_id << " called join() on Thread "<< this->_id << "\n";
    if (_state != FINISHING) {
        _joiner_thread = _running;
        _running->suspend();
    }
    return _exit_code;
}

void Thread::suspend() {
    db<Thread>(TRC) << "Thread() id="<< this->_id << " suspend()\n";
    _state = WAITING;
    _waiting.insert(&_link);
    yield();
}

void Thread::resume() {
    db<Thread>(TRC) << "Thread() id="<< this->_id << " resume()\n";
    _waiting.remove(&_link);
    _state = READY;
    _ready.insert(&_link);
    yield();
}

__END_API