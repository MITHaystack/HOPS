#include <string>
#include <sstream>
#include <iostream>
#include "MHO_Timer.hh"

#include "MHO_JSONHeaderWrapper.hh"
using nlohmann::json;

using namespace hops;

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



    double val = 23.0;
    double val2 = 100.0;
    double val3 = 200.0;

    //add a whole bunch of numbers 
    for(std::size_t i=0; i<10; i++)
    {
        std::stringstream ss;
        ss << "key-";
        ss << i;
        j2[ss.str()] = i*3.3;
    }
    
    j2["label_test"] = mho_json::array();
    std::cout<<"label_test size = "<<j2["label_test"].size()<<std::endl;
    //.reserve(100);
    j2["label_test"].get_ptr<json::array_t*>()->reserve(100);
    for(std::size_t i=0;i<100;i++)
    {
        mho_json empty;
        empty["index"] = i;
        j2["label_test"][i] = empty;
    }
    
    std::cout<< j2.dump(2) <<std::endl;




    MHO_Timer tmr;
    
    tmr.Start();
    // serialize to BSON
    std::vector<std::uint8_t> v_bson = json::to_bson(j2);
    tmr.Stop();
    
    std::cout<<"time to encode to bson: = "<<tmr.GetDurationAsDouble()<<std::endl;
    std::cout<<"size (bytes) "<<v_bson.size()<<std::endl;

    tmr.Start();
    json j_from_bson = json::from_bson(v_bson);
    tmr.Stop();
    
    std::cout<<"time to decode to bson: = "<<tmr.GetDurationAsDouble()<<std::endl;

    // serialize to CBOR
    tmr.Start();
    std::vector<std::uint8_t> v_cbor = json::to_cbor(j2);
    tmr.Stop();
    
    std::cout<<"time to encode to cbor: = "<<tmr.GetDurationAsDouble()<<std::endl;
    std::cout<<"size (bytes) "<<v_cbor.size()<<std::endl;
    
    tmr.Start();
    json j_from_cbor = json::from_cbor(v_cbor);
    tmr.Stop();
    
    std::cout<<"time to decode to cbor: = "<<tmr.GetDurationAsDouble()<<std::endl;


    return 0;
}
