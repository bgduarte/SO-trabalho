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

__END_API
