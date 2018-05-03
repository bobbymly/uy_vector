#include <iostream>
#include <new>
#include <string.h>
#include <algorithm>
#include "uy_allocator_2.h"
#include "simple_alloc.h"
using namespace std;


template<class T> fill(T* pos,size_t n,const T& target)
{
    for(size_t i=0;i<n;++i)
    {
        *pos = target;
        ++pos;
    }
}

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


//适配非基础数据类型进行析构
template <class T>
inline void destroy(T* p)
{
    p->~T();
}

template <class iterator>
inline void destroy(iterator start,iterator finish)
{
    for(;start < finish;++start)
    {
        destroy(start);
    }
}


// template <class iterator,class Size,class T>
// inline iterator uninitialized_fill_n(iterator start,iterator finish,const T& target)
// {
//     for(;start < finish;++ start)
//     {
//         construct(start,target);
//     }
//     return start;
// }

template <class iterator,class Size,class T>
inline iterator uninitialized_fill_n(iterator start,Size n,const T& target)
{
    iterator finish = start + n;
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
    typedef value_type* iterator;
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


    iterator begin() {return start;}
    iterator end() {return finish;}
    size_t size() const {return finish - start;}
    size_t capacity() const {return end_of_storage - start;}
    bool empty() const {return finish == start;}
    reference operator [] (size_t n) {return *(start + n);}
    
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

    iterator erase(iterator pos)
    {
        destroy(pos);
        if(pos != end() -1)copy(pos + 1,end() ,pos);
        --finish;
        destroy(finish);
        return pos;
    }

    iterator erase(iterator first,iterator last)
    {
        destroy(first,last);
        iterator i = copy(last,end(),first);
        destroy(i,end());
        finish = finish - (last - first);
        return first;
    }

    void clear() { erase(begin(),end());}
    void insert_aux(iterator pos,const value_type& target);
    void insert(iterator pos,size_t n,const value_type& target);

    
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
protected:
    //使用两级空间配置器 uy_allocator_2 封装的 simple_alloc
    typedef simple_alloc <value_type,Alloc> data_allocator;
    iterator start;
    iterator finish;
    iterator end_of_storage;

    
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

    


};


template <class value_type,class Alloc>
void uy_vector<value_type,Alloc> :: insert_aux(iterator pos,const value_type& target)
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
        //iterator new_start = (iterator)malloc(sizeof(value_type)*len);
        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        //cout<<"**"<<*new_start<<"**"<<endl;
        try
        {
            new_finish = uninitialized_copy(start,pos,new_start);
            // cout<<"*"<<new_finish<<endl;
            //iterator old_finish_pos = new_finish;
            
            construct(new_finish,target);
            //*old_finish_pos = target;
            //cout<<"*"<<*(new_finish-1)<<" "<<"target = "<<target<<"*"<<endl;
            ++ new_finish;
            new_finish = uninitialized_copy(pos,finish,new_finish);
            
        }catch (...){
            destroy(new_start,new_finish);
            data_allocator::deallocate(new_start,len);
            cout<<"err out of memory"<<endl;
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
void uy_vector<value_type,Alloc>::insert(iterator pos,size_t n,const value_type& target)
{
    if(n != 0)
    {
        if(end_of_storage - finish >= n)
        {
            const size_t elems_after = finish - pos;
            const iterator old_finish = finish;

            if(elems_after > n)
            {
                uninitialized_copy(finish - n,finish,finish);
                finish += n;
                copy_backward(pos,old_finish - n,old_finish);
                fill(pos,pos+n,target);
            }else{
                uninitialized_fill_n(finish,n - elems_after,target);
                finish += n-elems_after;
                uninitialized_copy(pos,old_finish,finish);
                finish += elems_after;
                fill(pos,old_finish,target);
            }
            // copy_backward(pos,finish,pos+n);
            // fill(pos,n,target);
            // finish = finish + n;
        }else{
            const size_t old_size = size();
            const size_t len = capacity() + max(capacity(),n);
            iterator new_start = data_allocator::allocate(len);
            iterator new_finish = new_start;
            
            try
            {
                new_finish = uninitialized_copy(start,pos,new_start);
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
            end_of_storage = new_start + len;

        }
    }
}