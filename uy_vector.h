#include <iostream>
#include <new>
#include "uy_allocator_2.h"
#include "simple_alloc.h"
using namespace std;


template <class T1,class T2>
inline void construct(T1* p,const T2& target)
{
    //placement new , 在已分配内存上构造对象
    new (p) T1 (target);
}

template <class T1,class T2>
inline T2 uninitialized_copy(T1 first,T1 last,T2 result_first)
{
    return copy(first,last,result_first);
}



template <class T>
inline void destroy(T* p)
{
    p->~T();
}

template <class iterater>
inline void destroy(iterater start,iterater finish)
{
    for(;start < finish;++start)
    {
        destroy(start);
    }
}


// template <class iterater,class Size,class T>
// inline iterater uninitialized_fill_n(iterater start,iterater finish,const T& target)
// {
//     for(;start < finish;++ start)
//     {
//         construct(start,target);
//     }
//     return start;
// }

template <class iterater,class Size,class T>
inline iterater uninitialized_fill_n(iterater start,Size n,const T& target)
{
    iterater finish = start + n;
    for(;start < finish;++ start)
    {
        construct(start,target);
    }
    return start;
}



template <class value_type,class Alloc = uy_allocator_2<0> >
class uy_vector
{

public:

    //迭代器直接使用指针
    typedef value_type* iterater;
    typedef value_type& reference;
    typedef ptrdiff_t difference_type;
    
    uy_vector():start(0),finish(0),end_of_storage(0){}
    uy_vector(size_t n,const value_type& target){ fill_initialized(n,target);}
    uy_vector(int n,const value_type& target){ fill_initialized(n,target);}
    uy_vector(long n,const value_type& target){ fill_initialized(n,target);}
    explicit uy_vector(size_t n){ fill_initialized(n,value_type());}
    ~uy_vector()
    {
        destroy(start,finish);
        deallocate();
    }


    iterater begin() {return start;}
    iterater end() {return finish;}
    size_t size() const {return finish - start;}
    size_t capacity() const {return end_of_storage - begin();}
    bool empty() const {return end() == begin();}
    reference operator [] (size_t n) {return *(begin() + n);}
    
    reference front() {return *begin();};
    reference back() {return *(end()-1);}
    void push_back(const value_type& target)
    {
        if(end() != end_of_storage)
        {
            *end() = target;
            ++ finish;
        }else{
            insert_aux(end(),target);







        }
    }
    void pop_back()
    {
        --finish;
        destroy(finish);
    }


protected:
    typedef simple_alloc <value_type,Alloc> data_allocator;
    iterater start;
    iterater finish;
    iterater end_of_storage;

    void insert_aux(iterater pos,const value_type& target);
    void insert(iterater pos,size_t n,const value_type& target);
    void deallocate()
    {
        if(start)data_allocator::deallocate(start,end_of_storage-start);
    }


    void fill_initialized(size_t n,const value_type& target)
    {
        start = data_allocator::allocate(n);
        uninitialized_fill_n(start,n,target);
        finish = start + n;
        end_of_storage = finish;
    }

    iterater erase(iterater pos)
    {
        destroy(pos);
        if(pos != end() -1)copy(pos + 1,end() ,pos);
        --finish;
        destroy(finish);
        return pos;
    }

    iterater erase(iterater first,iterater last)
    {
        destroy(first,last);
        int i = copy(last,end(),first);
        destroy(i,end());
        finish = finish - (last - first);
        return first;
    }

    void resize(size_t new_size,const value_type& target)
    {
        if(new_size < size())
        {
            erase(begin() + new_size,end());
        }else{
            insert(end(),new_size - size(),target);
        }
    }

    void resize(size_t new_size)
    {
        resize(new_size,value_type());
    }

    void clear() { erase(begin(),end());}










};


template <class value_type,class Alloc>
void uy_vector<value_type,Alloc> :: insert_aux(iterater pos,const value_type& target)
{
    if(finish != end_of_storage)
    {
        //有可用空间，直接插入
        construct(finish,*(finish - 1));
        ++finish;
        copy_backward(pos,finish - 2,finish - 1);
        *pos = target;
    }else{
        //无可用空间时，申请新空间->复制原有对象->插入新对象->析构旧空间上的对象->释放旧的内存空间

        const size_t old_size = size();
        //扩容策略采用增大为原来的两倍
        const size_t len = old_size != 0 ? (2 * old_size) : 1;
        iterater new_start = (iterater)malloc(sizeof(value_type)*len);
        //iterater new_start = data_allocator::allocate(len);
        iterater new_finish = new_start;
        cout<<"**"<<*new_start<<"**"<<endl;
        try
        {
            new_finish = uninitialized_copy(start,pos,new_start);
            // cout<<"*"<<new_finish<<endl;
            //iterater old_finish_pos = new_finish;
            
            construct(new_finish,target);
            //*old_finish_pos = target;
            cout<<"*"<<*(new_finish-1)<<" "<<"target = "<<target<<"*"<<endl;
            ++ new_finish;
            new_finish = uninitialized_copy(pos,finish,new_finish);
            
        }catch (...){
            destroy(new_start,new_finish);
            data_allocator::deallocate(new_start,len);
            cout<<"err"<<endl;
            throw;
        }

        destroy(start,finish);
        deallocate();

        start = new_start;   
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}




//need test
template <class value_type,class Alloc>
void uy_vector<value_type,Alloc>::insert(iterater pos,size_t n,const value_type& target)
{
    if(n != 0)
    {
        if(end_of_storage - finish >= n)
        {
            copy_backward(pos,finish,pos+n);
            fill(pos,n,target);
            finish = finish + n;
        }else{
            const size_t len = capacity() + max(capacity(),n);
            iterater new_start = data_allocator::allocate(len);
            iterater new_finish = new_start;
            
            try
            {
                new_finish = uninitialized_copy(start,pos,new_finish);
                new_finish = uninitialized_fill_n(new_finish,n,target);
                new_finish = uninitialized_copy(pos,finish,new_finish);
            }catch (...){
                destroy(new_start,new_finish);
                data_allocator::deallocate(new_start,len);
                throw;

            }

            destroy(start,finish);
            deallocate();


            start = new_start;
            finish = new_finish;
            end_of_storage = start + len;

        }




    }
}