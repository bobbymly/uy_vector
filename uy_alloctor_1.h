//第一级空间配置器


#include <iostream>
#include <cstdlib>
#include <new>
using namespace std;

template<int inst>
class uy_alloctor_1
{
private:
    static void* oom_malloc(size_t size);
    static void* oom_realloc(void* p,size_t size);
    static void (*oom_handler)();


public:

    static void* allocate(size_t size)
    {
        void* p = malloc(size);
        if(p == nullptr)
        {
            p = oom_malloc(size);
        }
        return p;
    }

    static void deallocate(void* p)
    {
        free(p);
    }

    static void* reallocate(void* p,size_t size)
    {
        void* result = realloc(p,size);
        if(result == nullptr)
        {
            result = oom_realloc(p,size);
        }
        return result;
    }

    static void set_malloc_handler(void (*f)())
    {
        //void (*old)() = oom_handler;
        oom_handler = f;
        //return old;
    }
};


template<int inst>
void (* uy_alloctor_1<inst>::oom_handler )() = 0;

template<int inst>
void* uy_alloctor_1<inst>::oom_malloc(size_t size)
{
    void (*my_handler)();
    void* result;
    while(1)
    {
        my_handler = oom_handler;
        if(my_handler == 0)
        {
            cerr<<"out of memory"<<endl;
            exit(1);
        }
        (*my_handler)();
        result = malloc(size);
        return result;
    }
}

template<int inst>
void* uy_alloctor_1<inst>:: oom_realloc(void *p,size_t size)
{
    void (*my_handler)();
    void* result;
    while(1)
    {
        my_handler = oom_handler;
        if(my_handler == 0)
        {
            cerr<<"out of memory"<<endl;
            exit(1);
        }
        (*my_handler)();
        result = realloc(p,size);
        if(result)return result;
    }
}

typedef uy_alloctor_1<0> malloc_alloc;