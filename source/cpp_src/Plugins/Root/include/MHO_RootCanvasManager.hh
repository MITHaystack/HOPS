#ifndef MHO_RootCanvasManager_HH__
#define MHO_RootCanvasManager_HH__

//ROOT includes
#include "TCanvas.h"
#include "TStyle.h"
#include "TColor.h"
#include "TMath.h"
#include "TGraph.h"

#include <vector>



/*
*@file: MHO_RootCanvasManager.hh
*@class: MHO_RootCanvasManager
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{

class MHO_RootCanvasManager
{
    public:
        MHO_RootCanvasManager();
        virtual ~MHO_RootCanvasManager();

        TCanvas* CreateCanvas(std::string name, unsigned int width = 500, unsigned int height = 500);

        TCanvas* SelectCanvasByName(std::string name);
        TCanvas* GetCurrentCanvas(){return fCurrentCanvas;}

    private:

        void ConfigureStyle();
        void DestroyCanvasList();

        TStyle* fStyle;
        TCanvas* fCurrentCanvas;
        std::vector< TCanvas* > fCanvasList;
};

}//end of namespace

#endif /* end of include guard: MHO_RootCanvasManager */