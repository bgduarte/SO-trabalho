#ifndef traits_h
#define traits_h

//Não alterar as 3 declarações abaixo

#define __BEGIN_API             namespace SOLUTION {
#define __END_API               }
#define __USING_API            using namespace SOLUTION;

__BEGIN_API

class CPU; //declaração das classes criadas nos trabalhos devem ser colocadas aqui
class Thread;
class Debug;
class System;
class Lists;

template<typename T>
struct Traits {
    static const bool debugged = false;
};

template<> struct Traits<CPU>
{
    static const unsigned int STACK_SIZE = 65536; // 64kB
};

template<> struct Traits<Debug>: public Traits<void>
{
    static const bool error = false;
    static const bool warning = false;
    static const bool info = false;
    static const bool trace = false;
};

template<> struct Traits<System>:public Traits<void> {
    static const bool debugged = true;
};

template<> struct Traits<Thread>:public Traits<void> {
    static const bool debugged = true;
};

template<> struct Traits<Lists>:public Traits<void> {
    static const bool debugged = false;
};


__END_API

#endif
