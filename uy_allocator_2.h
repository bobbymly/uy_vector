//第二级空间配置器
//避免太多的小额区块造成内存的碎片

//空间管理策略是
//当申请的空间够大 （大于 128 bytes）时，直接交由第一级空间配置器处理
//当申请的空间较小 （小于 128 bytes）时，以内存池管理

//第二级空间配置器 使用一个 free_list 管理不同大小的内存块（所有内存块的管理与申请均提升为 8 bytes 的整数倍）
//当所申请的内存块大小在 free_list 中有对应大小的剩余内存块可以使用时，直接从 free_list 中分配
//否则，从内存池中一次分配 20 个申请大小的内存块，一个用于使用，19 个添加到 free_list 中备用

//当内存池中内存不足 20 个 申请的内存块大小时
//若足够一个则给与一个放回使用
//剩余的小区块内存直接添加到 free_list 中（充分利用）
//然后调用第一级空间适配器 向内存池中补充内存


#include "uy_allocator_1.h"

using namespace std;
enum {__ALIGN = 8};
enum {__MAX_BYTES = 128};
enum {__NFREE_LIST = __MAX_BYTES/__ALIGN};

//obj 结构重复利用内存，无需额外内存进行链式结构的管理
union obj
{
    union obj* free_list_link;
    char client_data[1];
};

template <int inst>
class uy_allocator_2
{


private:
    static obj*  free_list[__NFREE_LIST];
    static char* start_free;
    static char* end_free;
    static size_t heap_size;

    //将需求的内存块大小提升为 8 的整数倍
    static size_t round_up(size_t bytes)
    {
        return (bytes+__ALIGN-1) & ~(__ALIGN-1);
    }

    
    static size_t free_list_index(size_t bytes)
    {
        return (bytes+__ALIGN-1)/__ALIGN-1;
    }

    static void* refill(size_t n);

    static char* chunk_alloc(size_t size,int &nobjs);

    

    public:
        static void* allocate(size_t size);
        static void deallocate(void* p,size_t size);
        static void* reallocate(void* p,size_t old_size,size_t new_size);




};

template <int inst>
char* uy_allocator_2<inst>::start_free = 0;

template <int inst>
char* uy_allocator_2<inst>::end_free = 0;

template <int inst>
size_t uy_allocator_2<inst>::heap_size = 0;

template <int inst>

//以链式结构管理内存
obj* uy_allocator_2<inst>::free_list[__NFREE_LIST] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

template <int inst>
void* uy_allocator_2<inst>::allocate(size_t n)
{
    obj** my_free_list;
    obj* result;
    //大于128就使用一级配置器
    if(n > (size_t) __MAX_BYTES)
    {
        return uy_allocator_1<inst>::allocate(n);
    }
    //否则使用二级配置器
    my_free_list = free_list + free_list_index(n);
    result = *my_free_list;
    //先从 free_list 中找，若没有对应大小的内存块，就使用 refill 向 free_list 中填充内存块
    if(result == NULL)
    {
        void* res = refill(round_up(n));
        return res;
    }
    *my_free_list = result -> free_list_link;
    return  result;
}

template <int inst>
void uy_allocator_2<inst>::deallocate(void* p,size_t size)
{
    obj* q=(obj*) p;
    obj** my_free_list;
    if(size > __MAX_BYTES)
    {
        uy_allocator_1<inst>::deallocate(p,size);
        return ;
    }
    my_free_list =free_list + free_list_index(size);
    q->free_list_link = *my_free_list;
    *my_free_list = q; 

}

template <int inst>
void* uy_allocator_2<inst>::refill(size_t n)
{
    int nobjs = 20;
    char* chunk = chunk_alloc(n,nobjs);
    obj** my_free_list;
    obj* result,*next_obj,*current_obj;
    int i;

    if(nobjs == 1)
    {
        return chunk;
    }
    //返回一个内存块用于使用，其余的添加到 free_list 中
    my_free_list = free_list + free_list_index(n) ;
    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk + n);
    for(int i=1;;++i)
    {
        current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj + n);
        if(nobjs - 1 == i)
        {
            current_obj->free_list_link = 0;
            break;
        }else{
            current_obj->free_list_link = next_obj;
        }
    }
    return result;
}


//内存池管理
template <int inst>
char* uy_allocator_2<inst>::chunk_alloc(size_t size,int& nobjs)
{
    char* result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;

    //剩余内存满足
    if(bytes_left >= total_bytes)
    {
        result = start_free;
        start_free += total_bytes;
        return start_free;
    } else if(bytes_left >= size){  //剩余内存不满足所有的，但至少满足一个区块
        nobjs = bytes_left / size;
        total_bytes = nobjs * size;
        result = start_free;
        start_free += total_bytes;
        return result;
    }else{      //剩余内存连一个区块都无法满足
                
        size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >>4);
        //尝试把剩余内存编入 free_list
        if(bytes_left > 0)
        {
            obj** my_free_list = free_list +free_list_index(bytes_left);
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }
        //配置 heap 补充内存池
        start_free = (char*)malloc(bytes_to_get);

        if(0 == start_free)
        {
            int i;
            obj** my_free_list,*p;
            //在 free_list 中找足够大的区块
            for(i = size;i <=__MAX_BYTES;i += __ALIGN)
            {
                my_free_list = free_list + free_list_index(i);
                p = *my_free_list;
                if(0 != p)
                {
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free + i;
                    return(chunk_alloc(size,nobjs));
                }
            }
            end_free = 0;
            start_free = (char*)uy_allocator_1<inst>::allocate(bytes_to_get);
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return chunk_alloc(size,nobjs);
    }
}