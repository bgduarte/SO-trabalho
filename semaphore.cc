#include "semaphore.h"
#include "cpu.h"
#include "thread.h"
#include "traits.h"
#include "debug.h"

__BEGIN_API

Semaphore::~Semaphore() {}

void Semaphore::p() {
    db<Semaphore>(TRC) << "Semaphore p(): Thread id="<< Thread::running()->id() << " count:" << _count << "\n";
    if (_count == 0)
        sleep();
    else
        fdec(_count);
}

void Semaphore::v() {
    db<Semaphore>(TRC) << "Semaphore v(): Thread id="<< Thread::running()->id() << " count:" << _count << "\n";
    if (_count == 0 && !_threads.empty())
        wakeup();
    else
        finc(_count);
}

int Semaphore::finc(volatile int & number) {
    db<Semaphore>(TRC) << "Semaphore finc():\n";
    return CPU::finc(number);
}
int Semaphore::fdec(volatile int & number) {
    db<Semaphore>(TRC) << "Semaphore fdec():\n";
    return CPU::fdec(number);
}

void Semaphore::sleep() {
    db<Semaphore>(TRC) << "Semaphore sleep(): Thread id="<< Thread::running()->id() << " \n";
    _threads.insert(Thread::running()->link());
    Thread::running()->sleep();
}
void Semaphore::wakeup() {
    Thread* waking_thread = _threads.remove()->object();
    db<Semaphore>(TRC) << "Semaphore wakeup(): Thread id="<< waking_thread->id() << " \n";
    waking_thread->wakeup();
}
void Semaphore::wakeup_all() {
    db<Semaphore>(TRC) << "Semaphore wakeup_all()\n";
    while (!_threads.empty()) {
        _threads.remove()->object()->wakeup();
    }
} 

__END_API