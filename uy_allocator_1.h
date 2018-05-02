//第一级空间配置器
//直接使用 malloc 从系统中申请内存空间
 
#include <iostream>
#include <cstdlib>
#include <new>
using namespace std;

template<int inst>
class uy_allocator_1
{
private:
    //内存不足处理函数
    static void* oom_malloc(size_t size);
    static void* oom_realloc(void* p,size_t size);
    static void (*oom_handler)();


public:

    static void* allocate(size_t size)
    {
        void* p = malloc(size);
        if(p == NULL)
        {
            p = oom_malloc(size);
        }
        return p;
    }

    static void deallocate(void* p,size_t n)
    {
        free(p);
    }

    static void* reallocate(void* p,size_t size)
    {
        void* result = realloc(p,size);
        if(result == NULL)
        {
            result = oom_realloc(p,size);
        }
        return result;
    }

    static void set_malloc_handler(void (*f)())
    {
        oom_handler = f;
    }
};


template<int inst>
void (* uy_allocator_1<inst>::oom_handler )() = 0;

template<int inst>
void* uy_allocator_1<inst>::oom_malloc(size_t size)
{
    void (*my_handler)();
    void* result;
    //不断调用处理函数 尝试释放内存以供使用
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
void* uy_allocator_1<inst>:: oom_realloc(void *p,size_t size)
{
    void (*my_handler)();
    void* result;
    //不断调用处理函数 尝试释放内存以供使用
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

typedef uy_allocator_1<0> malloc_alloc;