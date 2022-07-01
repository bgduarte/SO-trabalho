#include "cpu.h"
#include <iostream>

__BEGIN_API


void CPU::Context::save()
{
    getcontext(&_context);
}

void CPU::Context::load()
{
    setcontext(&_context);
}

CPU::Context::~Context()
{
    if (_stack)
        delete _stack;
}

void CPU::switch_context(Context *from, Context *to)
{
    from->save();
    to->load();
}


int CPU::finc(volatile int & number) {
    __asm__ __volatile__ ("lock xadd %1, %0;"
        : "+m" (number)
        : "r" (ONE));
}
int CPU::fdec(volatile int & number) {
    __asm__ __volatile__ ("lock subl %1, %0;"
        : "+m" (number)
        : "r" (ONE));
}

__END_API
