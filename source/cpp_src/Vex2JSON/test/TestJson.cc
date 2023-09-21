#include <string>
#include <iostream>

#include "MHO_JSONHeaderWrapper.hh"
using nlohmann::json;

int main(int argc, char** argv)
{
    json j2 = {
      {"pi", 3.141},
      {"happy", true},
      {"name", "Niels"},
      {"nothing", nullptr},
      {"answer", {
        {"everything", 42}
      }},
      {"list", {1, 0, 2}},
      {"object", {
        {"currency", "USD"},
        {"value", 42.99}
      }}
    };

    std::cout<< j2.dump(2) <<std::endl;

    double val = 23.0;
    double val2 = 100.0;
    double val3 = 200.0;
    std::vector< std::string > path = { std::string("item1"), std::string("item2"), std::string("item3"), std::string("item_key") };
    std::vector< std::string > path2 = { std::string("item1"), std::string("item4") };
    std::vector< std::string > path3 = { std::string("item1"), std::string("item4") }; //to change the value of item4


    //test insertion/construction of object
    json j3;
    auto* p = &j3;
    for(auto it = path.begin(); it != path.end(); it++)
    {
        if (!p->is_object()){ *p = json::object(); }
        if( *it != *(path.rbegin()) )
        {
            if( p->contains(*it) ){ p = &(p->at(*it) ); }
            else
            {
                std::cout<<*it<<std::endl;
                p = &(*p)[ *it ];
            }
        }
        else
        {
            p = &(*p)[ *it ];
            *p = val;
        }
    }

    p = &j3;
    for(auto it = path2.begin(); it != path2.end(); it++)
    {
        if (!p->is_object()){ std::cout<<"oops"<<std::endl; *p = json::object(); }
        if( *it != *(path2.rbegin()) )
        {
            if( p->contains(*it) )
            {
                std::cout<<"skip to: "<<*it<<std::endl;
                p = &(p->at(*it) );
            }
            else
            {
                std::cout<<*it<<std::endl;
                p = &(*p)[ *it ];
            }
        }
        else
        {
            p = &(*p)[ *it ];
            *p = val2;
        }
    }

    //now overwrite val2 with val3
    p = &j3;
    for(auto it = path3.begin(); it != path3.end(); it++)
    {
        if (!p->is_object()){ std::cout<<"oops"<<std::endl; *p = json::object(); }
        if( *it != *(path3.rbegin()) )
        {
            if( p->contains(*it) )
            {
                std::cout<<"skip to: "<<*it<<std::endl;
                p = &(p->at(*it) );
            }
            else
            {
                std::cout<<*it<<std::endl;
                p = &(*p)[ *it ];
            }
        }
        else
        {
            p = &(*p)[ *it ];
            *p = val3;
        }
    }

    std::cout<< j3.dump(2) <<std::endl;

    return 0;
}
