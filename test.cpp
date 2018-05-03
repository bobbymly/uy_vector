#include "uy_vector.h"
#include <iostream>
using namespace std;

template <class T>
void show_vec(uy_vector<T> &test)
{
    for(auto it = test.begin();it != test.end();++it)
    {
        cout<<*it<<" ";
    }
    cout<<endl;
}


//class for test uy_vector
class uy_a
{
public:
    uy_a()
    {
        cout<< "uy_a() "<<endl;
    }
    ~uy_a()
    {
        cout<<"~uy_a() "<<endl;
    }
    uy_a(const uy_a& target)
    {
        cout<<"copy uy_a()"<<endl;
    }
    operator =(const uy_a& target)
    {
        cout<<"operator = uy_a()"<<endl;
    }
};


int main()
{

    uy_vector<double> test;

    for(int i=0;i<10;++i)
    {
        test.push_back(i);
        cout<<test[i]<<" size: "<<test.size()<<" capacity: "<<test.capacity()<<endl;
    }


    cout<<endl;

    cout<<"erase the begin of vector:"<<endl;
    test.erase(test.begin());
    show_vec(test);
    
    cout<<"pop_back of vector:"<<endl;
    test.pop_back();
    show_vec(test);

    cout<<"insert(test.begin()+2,3,6"<<endl;
    test.insert(test.begin()+2,3,6);
    show_vec(test);


    cout<<"test construct and destroy of obj"<<endl;
    //测试 uy_vector 对于对象的构造和析构
    uy_vector<uy_a>a_test;
    for(int i=0;i<5;++i)a_test.push_back(uy_a());
    
    


    return 0;
}