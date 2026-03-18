#include "MHO_Mark4OutputVisitor.hh"

#include "MHO_FringeData.hh"
#include "MHO_MK4FringeExport.hh"

#include <cstdio>

namespace hops 
{


MHO_Mark4OutputVisitor::MHO_Mark4OutputVisitor(){}

MHO_Mark4OutputVisitor::~MHO_Mark4OutputVisitor(){}

void 
MHO_Mark4OutputVisitor::Visit(MHO_FringeFitter* fitter)
{
    MHO_FringeData* fringeData = fitter->GetFringeData();
    if(!fringeData)
    {
        msg_error("fringe", "fringe data is null, aborting output write" << eom);
        return;
    }
    
    MHO_MK4FringeExport fexporter;
    fexporter.SetParameterStore(fringeData->GetParameterStore());
    fexporter.SetPlotData(fringeData->GetPlotData());
    fexporter.SetContainerStore(fringeData->GetContainerStore());
    fexporter.ExportFringeFile();
}

} // namespace hops
