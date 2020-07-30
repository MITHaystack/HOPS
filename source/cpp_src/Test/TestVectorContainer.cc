#include <iostream>
#include <string>

#include "HkVectorContainer.hh"


using namespace hops;



class Test: public HkVectorContainer< double >
{

    public:
        Test(){}
        virtual ~Test (){}

};

int main(int /*argc*/, char** /*argv*/)
{
    Test* A = new Test();


    A->SetName("a-test");

    std::cout<<A->GetName()<<", "<<A->GetValue()<<std::endl;

    delete A;

    return 0;
}
