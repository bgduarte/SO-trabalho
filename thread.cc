#include "thread.h"
#include "cpu.h"
#include <iostream>

__BEGIN_API

int Thread::_thread_counter = 0;
Thread * Thread::_running = nullptr;

int Thread::switch_context(Thread * prev, Thread * next) {
    if (!prev->context() || !next->context())
        return -1;
    CPU::switch_context(prev->context(), next->context());
    _running = next;
    return 0;
}

void Thread::thread_exit(int exit_code) {
    if (_context)
        delete _context;
    _thread_counter--;
}

int Thread::id() {
    return _id;
}

int Thread::_increment_thread_counter() {
    return _thread_counter++;
}

__END_API