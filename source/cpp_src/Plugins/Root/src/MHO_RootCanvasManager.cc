#include "MHO_RootCanvasManager.hh"

namespace hops 
{

MHO_RootCanvasManager::MHO_RootCanvasManager()
{
    ConfigureStyle();
}

MHO_RootCanvasManager::~MHO_RootCanvasManager()
{
    delete fStyle;
    DestroyCanvasList();
}

void MHO_RootCanvasManager::ConfigureStyle()
{
    fStyle = new TStyle("Plain", "Plain");
    fStyle->SetCanvasBorderMode(0);
    fStyle->SetPadBorderMode(0);
    fStyle->SetPadColor(0);
    fStyle->SetCanvasColor(0);
    fStyle->SetTitleColor(1);
    fStyle->SetPalette(1,0);   // nice color scale for z-axis
    fStyle->SetCanvasBorderMode(0); // gets rid of the stupid raised edge around the canvas
    fStyle->SetTitleFillColor(0); //turns the default dove-grey background to white
    fStyle->SetCanvasColor(0);
    fStyle->SetPadColor(0);
    fStyle->SetTitleFillColor(0);
    fStyle->SetStatColor(0); //this one may not work
    const int NRGBs = 5;
    const int NCont = 48;
    double stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    double red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    double green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    double blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    fStyle->SetNumberContours(NCont);
    fStyle->cd();
}



void 
MHO_RootCanvasManager::CreateCanvas(std::string name, unsigned int width = 500, unsigned int height = 500)
{
    TCanvas* c = new TCanvas(name.c_str(), name.c_str(), 0, 0, width, height);
    c->cd();
    fCanvasList.push_back(c);
}


TCanvas* 
MHO_RootCanvasManager::SelectCanvasByName(std::string name)
{
    for(std::size_t i=0; i<fCanvasList.size(); i++)
    {
        std::string tmp = fCanvasList[i]->GetName();
        if(tmp == name)
        {
            fCanvasList[i]->cd();
            return fCanvasList[i];
        }
    }
}

void 
MHO_RootCanvasManager::DestroyCanvasList()
{
    for(std::size_t i=0; i<fCanvasList.size(); i++)
    {
        delete fCanvasList[i];
    }
}

}//end namespace