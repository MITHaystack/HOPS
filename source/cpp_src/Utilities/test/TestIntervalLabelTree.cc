#include <iostream>
#include <string>

#include "MHOIntervalLabelTree.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHOIntervalLabelTree test;

    MHOIntervalLabel label1(0,4);
    MHOIntervalLabel label2(4,6);
    MHOIntervalLabel label3(6,9);
    MHOIntervalLabel label4(9,12);
    MHOIntervalLabel label5(0,6);
    MHOIntervalLabel label6(6,12);

    label1.Insert(std::string("channel"), 'a');
    label1.Insert(std::string("bandwidth"), 32.0e6);
    label2.Insert(std::string("channel"), 'b');
    label2.Insert(std::string("bandwidth"), 32.0e6);
    label3.Insert(std::string("channel"), 'c');
    label3.Insert(std::string("bandwidth"), 32.0e6);
    label4.Insert(std::string("channel"), 'd');
    label4.Insert(std::string("bandwidth"), 32.0e6);
    label5.Insert(std::string("sampler"), std::string("r2dbe-1"));
    label6.Insert(std::string("sampler"), std::string("r2dbe-2"));

    test.InsertLabel(label1);
    test.InsertLabel(label2);
    test.InsertLabel(label3);
    test.InsertLabel(label4);
    test.InsertLabel(label5);
    test.InsertLabel(label6);

    auto label_vec1 = test.GetIntervalsWhichIntersect(5);
    for(auto iter = label_vec1.begin(); iter != label_vec1.end(); iter++)
    {
        std::cout<<"label found for interval = ["<<(*iter)->GetLowerBound()<<", "<<(*iter)->GetUpperBound()<<") with key:val pairs = "<<std::endl;
        (*iter)->DumpMap<char>();
        (*iter)->DumpMap<std::string>();
        (*iter)->DumpMap<int>();
        (*iter)->DumpMap<double>();
    }

    auto label_vec2 = test.GetIntervalsWithKeyValue(std::string("channel"), 'c');
    for(auto iter = label_vec2.begin(); iter != label_vec2.end(); iter++)
    {
        std::cout<<"label found for interval = ["<<(*iter)->GetLowerBound()<<", "<<(*iter)->GetUpperBound()<<") with key:val pairs = "<<std::endl;
        (*iter)->DumpMap<char>();
        (*iter)->DumpMap<std::string>();
        (*iter)->DumpMap<int>();
        (*iter)->DumpMap<double>();
    }

    return 0;
}
