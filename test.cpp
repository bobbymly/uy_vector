#include "uy_vector.h"
#include <iostream>
using namespace std;

// enum{__ALIGNTTT = 8};
// static size_t round_up(size_t bytes)
//     {
//         return (bytes+__ALIGNTTT-1) & ~(__ALIGNTTT-1);
//     }
int main()
{

    // for(int i=0;i<20;++i){
    //     cout<<i<<":";
    //     int temp = round_up((size_t) i);
    //     cout<<temp<<endl;
    // }


    uy_vector<int> test;

    for(int i=0;i<100;++i)
    {
        test.push_back(i);
        for(int j=0;j<test.size();++j)
        {
            cout<<test[j]<<" "<<endl;
        }
        cout<<endl;
        
        
    }


    
    // getchar();
    // getchar();
    // test.push_back(12321);
    // cout<<test[0]<<endl;
    // test.push_back(999);
    // cout<<test[1]<<endl;
    // test.push_back(1);
    // cout<<test[2]<<endl;
    // test.push_back(2);
    // cout<<test[3]<<endl;
    // test.push_back(3);
    // cout<<test[4]<<endl;
    // test.push_back(4);
    // cout<<test[5]<<endl;
    // cout<<"begin:"<<endl;
    //  for(int i=0;i<test.size();++i)cout<<test[i]<<endl;

    return 0;
}