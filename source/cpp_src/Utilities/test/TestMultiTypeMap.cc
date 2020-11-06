#include <iostream>
#include <string>
#include <sstream>

#include "MHOTimeStampConverter.hh"
#include "MHOMultiTypeMap.hh"

#include "MHOMeta.hh"

using namespace hops;


typedef std::string key_type1;

int main(int /*argc*/, char** /*argv*/)
{

    //std::cout<<"size of the typelist is: "<< MHOTypelistSize< a_typelist >::value <<std::endl;
    MHOMultiTypeMap< key_type1, int, double, float, std::string > myMap;
    MHOMultiTypeMap< key_type1, int, double, float, std::string > myMap2;

    std::string key1("i_am_an_int");
    int val1 = 1;

    std::string key2("i_am_a_double");
    double val2 = 3.14159;

    myMap.Insert( key1, val1);
    myMap.Insert( key2, val2);

    int ret1 = 0;
    bool ok = myMap.Retrieve(key1, ret1);
    if(ok)
    {
        std::cout<<"got an int with key: "<<key1<<", value = "<<ret1<<std::endl;
    }
    else
    {
        std::cout<<"could not find int with key: "<<key1<<std::endl;
    }

    double ret2 = 0;
    bool ok2 = myMap.Retrieve(key2, ret2);
    if(ok2)
    {
        std::cout<<"got an double with key: "<<key2<<", value = "<<ret2<<std::endl;
    }
    else
    {
        std::cout<<"could not find double with key: "<<key2<<std::endl;
    }


    for(int i=0; i<10; i++)
    {
        std::stringstream ss;
        ss << "key-int-";
        ss << i;
        myMap.Insert( ss.str(), 2*i );

        std::stringstream ssd;
        ssd << "key-double-";
        ssd << i;
        double value = 3.14159*i;
        myMap.Insert( ssd.str(),  value);
    }

    myMap.Insert(std::string("blah"), std::string("blah") );
    myMap.Insert(std::string("blah2"), std::string("blah2") );
    myMap.Insert(std::string("blah3"), std::string("blah3") );


    std::cout<<"-- dumping the map of integers --"<<std::endl;
    myMap.DumpMap<int>();

    std::cout<<"-- dumping the map of doubles -- "<<std::endl;
    myMap.DumpMap<double>();

    std::cout<<"-- dumping the map of strings -- "<<std::endl;
    myMap.DumpMap<std::string>();

    // myMap.SetName(std::string("myMap"));
    // std::cout<<myMap.GetName()<<std::endl;
    myMap2.CopyFrom<double>(myMap);

    std::cout<<"-- dumping the map of doubles of map2-- "<<std::endl;
    myMap2.DumpMap<double>();

    myMap2.Insert(std::string("blah4"), std::string("blah4"));

    myMap2.CopyTo<std::string>(myMap);

    std::cout<<"-- dumping the map of strings of map after copy -- "<<std::endl;
    myMap.DumpMap<std::string>();


    if(myMap.ContainsKey<std::string>(std::string("blah4")))
    {
        std::cout<<"my map contains the key"<<std::endl;
    }

    return 0;
}
