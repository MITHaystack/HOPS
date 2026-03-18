#include "MHO_OutputVisitorFactory.hh"


#include "MHO_HopsOutputVisitor.hh"
#include "MHO_Mark4OutputVisitor.hh"

// #ifdef HOPS_USE_HDF5
// #include "MHO_HDF5OutputVisitor.hh"
// #endif

namespace hops
{

MHO_OutputVisitorFactory::MHO_OutputVisitorFactory(){}

MHO_OutputVisitorFactory::~MHO_OutputVisitorFactory()
{
    for(auto it = fOutputVisitors.begin(); it != fOutputVisitors.end(); it++)
    {
        delete it->second;
        it->second = nullptr;
    }
    fOutputVisitors.clear();
}

MHO_FringeFitterVisitor* MHO_OutputVisitorFactory::GetOutputVisitor(std::string format)
{
    MHO_FringeFitterVisitor* output_visitor = nullptr;
    msg_debug("fringe", "output format choice is: "<< format << eom);

    if(format == "hops4")
    {   
        auto it = fOutputVisitors.find("hops4");
        if(it == fOutputVisitors.end())
        {
            output_visitor = new MHO_HopsOutputVisitor();
            fOutputVisitors["hops4"] = output_visitor;
            return output_visitor;
        }
        else 
        {
            output_visitor = it->second;
        }
    }
    else if(format == "mark4")
    {
        auto it = fOutputVisitors.find("mark4");
        if(it == fOutputVisitors.end())
        {
            output_visitor = new MHO_Mark4OutputVisitor();
            fOutputVisitors["mark4"] = output_visitor;
            return output_visitor;
        }
        else 
        {
            output_visitor = it->second;
        }
    }
    // else if(format = "hdf5")
    // {
    //     #ifdef HOPS_USE_HDF5
    //         auto it = fOutputVisitors.find("hdf5");
    //         if(it == fOutputVisitors.end())
    //         {
    //             output_visitor = new MHO_HDF5OutputVisitor();
    //             fOutputVisitors["hdf5"] = output_visitor;
    //             return output_visitor;
    //         }
    //         else 
    //         {
    //             output_visitor = it->second;
    //         }
    //     #else 
    //         msg_warn("fringe", "output format choice: "<< format <<" is not available on this system " << eom);
    //     #endif
    // 
    // }

    //if we don't have the output format enabled/available, this will return nullptr
    return output_visitor;
}

} // namespace hops
