#include <iostream>
#include <string>
#include <cmath>

#include "HkTensorContainer.hh"
#include "HkVectorContainer.hh"


#include "TCanvas.h"
#include "TApplication.h"
#include "TStyle.h"
#include "TColor.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TMultiGraph.h"


using namespace hops;

#define NDIM 3
typedef HkAxisPack< HkVectorContainer<double>, HkVectorContainer<double>, HkVectorContainer<char> > axis_pack_test;

int main(int argc, char** argv)
{

    size_t* dim = new size_t[NDIM];
    dim[0] = 256; //x
    dim[1] = 256; //y
    dim[2] = 3; // r,g,b

    HkTensorContainer<double, axis_pack_test >* test = new HkTensorContainer<double, axis_pack_test >(dim);

    for(size_t i=0; i<NDIM; i++)
    {
        std::cout<<"dimension @ "<<i<<" ="<<test->GetDimension(i)<<std::endl;
    }

    //set up the axis labels
    auto* x_axis = &(std::get<0>(*test));
    size_t x_axis_size = x_axis->GetDimension(0);
    for(size_t i=0; i<x_axis_size; i++)
    {
        x_axis->at(i) = i*(2.0*M_PI/(double)x_axis_size);
    }

    auto* y_axis = &(std::get<1>(*test));
    size_t y_axis_size = y_axis->GetDimension(0);
    for(size_t i=0; i<y_axis_size; i++)
    {
        y_axis->at(i) = i*(2.0*M_PI/(double)y_axis_size);
    }

    auto* z_axis = &(std::get<2>(*test));
    size_t z_axis_size = z_axis->GetDimension(0);
    z_axis->at(0) = 'r';
    z_axis->at(1) = 'g';
    z_axis->at(2) = 'b';

    for(size_t i=0; i<x_axis_size; i++)
    {
        for(size_t j=0; j<y_axis_size; j++)
        {
            for(size_t k=0; k<z_axis_size; k++)
            {
                double value = std::cos( 2*(k+1)*x_axis->at(i) )*std::sin( 2*(k+1)*y_axis->at(j) );
                (*test)(i,j,k) = value;
            }
        }
    }



    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots
    TApplication* App = new TApplication("PowerPlot",&argc,argv);
    TStyle* myStyle = new TStyle("Plain", "Plain");
    myStyle->SetCanvasBorderMode(0);
    myStyle->SetPadBorderMode(0);
    myStyle->SetPadColor(0);
    myStyle->SetCanvasColor(0);
    myStyle->SetTitleColor(1);
    myStyle->SetPalette(1,0);   // nice color scale for z-axis
    myStyle->SetCanvasBorderMode(0); // gets rid of the stupid raised edge around the canvas
    myStyle->SetTitleFillColor(0); //turns the default dove-grey background to white
    myStyle->SetCanvasColor(0);
    myStyle->SetPadColor(0);
    myStyle->SetTitleFillColor(0);
    myStyle->SetStatColor(0); //this one may not work
    const int NRGBs = 5;
    const int NCont = 48;
    double stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    double red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    double green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    double blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    myStyle->SetNumberContours(NCont);
    myStyle->cd();

    //plotting objects
    std::vector< TCanvas* > canvas;
    std::vector< TGraph* > graph;
    std::vector< TGraph2D* > graph2d;

    TGraph2D *gr = new TGraph2D(x_axis_size*y_axis_size);
    TGraph2D *gg = new TGraph2D(x_axis_size*y_axis_size);
    TGraph2D *gb = new TGraph2D(x_axis_size*y_axis_size);

    size_t count = 0;
    for(size_t i=0; i<x_axis_size; i++)
    {
        for(size_t j=0; j<y_axis_size; j++)
        {
            for(size_t k=0; k<z_axis_size; k++)
            {
                gr->SetPoint(count, x_axis->at(i), y_axis->at(j), (*test)(i,j,0) );
                gg->SetPoint(count, x_axis->at(i), y_axis->at(j), (*test)(i,j,1) );
                gb->SetPoint(count, x_axis->at(i), y_axis->at(j), (*test)(i,j,2) );
            }
            count++;
        }
    }

    std::string name("test");
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    c->Divide(1,3);
    c->cd(1);
    gr->Draw("PCOL");
    c->Update();
    c->cd(2);
    gg->Draw("PCOL");
    c->Update();
    c->cd(3);
    gb->Draw("PCOL");
    c->Update();

    App->Run();


    delete gr;
    delete gg;
    delete gb;
    delete c;
    delete test;
    delete dim;

    return 0;
}
