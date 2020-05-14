#include <iostream>
#include <string>

#include "HTimeStampConverter.hh"
#include "HMultiTypeMap.hh"

#include "HMeta.hh"

using namespace hops;

//typedef HTypelist<int, double, float, int, long int> a_typelist;

//
// class A1
// {
//     A1(){};
//     virtual ~A1(){};
//
//     virtual void DoSomething() = 0;
//
// };
//
// class A2: public A1
// {
//     A2(){};
//     virtual ~A2(){};
//
//     virtual void DoSomething() override {std::cout<<"A2"<<std::endl;}
// };
//
// class A3: public A1
// {
//     A3(){};
//     virtual ~A3(){};
//
//     virtual void DoSomething() override {std::cout<<"A3"<<std::endl;}
// };
//
//
// template< class T >
// class AUnit
// {
//     void Call(){T->DoSomething();};
// };
//
//
// typedef HTypelist<A2, A3> example1;
//
//
//
// template<typename T> class Holder
// {
//   public:
//
//     Holder():
//     {
//         fObject = new T();
//     };
//
//     virtual ~KFMObjectHolder()
//     {
//         delete fObject;
//     };
//
//     void DoSomething()
//     {
//         fObject->Something();
//     }
//
//     private:
//         T* fObject;
// };
//
//
//
// template<typename TypeList> class TypelistHolder : public HGenScatterHierarchy<TypeList, Holder>
// {
//   public:
//     TypelistHolder(){};
//     ~TypelistHolder() override{};
//
//   private:
// };

int main(int /*argc*/, char** /*argv*/)
{

    //std::cout<<"size of the typelist is: "<< HTypelistSize< a_typelist >::value <<std::endl;
    HMultiTypeMap< int, double, float > myMap;

    std::string key1("i_am_an_int");
    int val1 = 1;

    std::string key2("i_am_a_double");
    double val2 = 3.14159;

    myMap.insert( key1, val1);
    myMap.insert( key2, val2);

    int ret1 = 0;
    bool ok = myMap.retrieve(key1, ret1);
    if(ok)
    {
        std::cout<<"got an int with key: "<<key1<<", value = "<<ret1<<std::endl;
    }
    else
    {
        std::cout<<"could not find int with key: "<<key1<<std::endl;
    }

    double ret2 = 0;
    bool ok2 = myMap.retrieve(key2, ret2);
    if(ok2)
    {
        std::cout<<"got an double with key: "<<key2<<", value = "<<ret2<<std::endl;
    }
    else
    {
        std::cout<<"could not find double with key: "<<key2<<std::endl;
    }

    return 0;
}
