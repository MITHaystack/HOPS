#ifndef MHO_RootGraphManager_HH__
#define MHO_RootGraphManager_HH__

#include "TAxis.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TMultiGraph.h"


#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

/*
*@file: MHO_RootGraphManager.hh
*@class: MHO_RootGraphManager
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops 
{

class MHO_RootGraphManager
{
    public:

        MHO_RootGraphManager(){};

        virtual ~MHO_RootGraphManager()
        {
            for(unsigned int i=0; i<f1DGraph.size(); i++){delete f1DGraph[i];}
            for(unsigned int i=0; i<f2DGraph.size(); i++){delete f2DGraph[i];}
        }

        template< typename XTableType, typename XAxisType >
        TGraph* GenerateGraph1D(const XTableType& table, const XAxisType& x_axis)
        {
            //assert that this is a 1d table 
            HOPS_ASSERT_EQUAL( table.GetRank(), 1 );

            //assume axis is labeled by doubles 
            std::size_t nxbins = table.GetDimension(0);

            TGraph* h = new TGraph();

            //now fill the histogram
            for(std::size_t i=0; i<nxbins; i++)
            {
                double sbd_bin = i* 0.5e6*(1.0/32e6);
                std::cout<<i<<", "<<sbd_bin<<", "<< std::abs( table.at(i) )<<std::endl;
                // h->SetPoint(i, x_axis(i), std::abs( table.at(i) ) );
                h->SetPoint(i, sbd_bin, std::abs( table.at(i) ) );
            }

            f1DGraph.push_back(h);
            return h;
        }

        template< typename XTableType, typename XAxisType, typename YAxisType >
        TGraph2D* GenerateGraph2D(const XTableType& table, const XAxisType& x_axis, const YAxisType& y_axis)
        {
            HOPS_ASSERT_EQUAL( table.GetRank(), 2 ); 

            std::size_t nxbins = table.GetDimension(0);
            std::size_t nybins = table.GetDimension(1);

            TGraph2D* h = new TGraph2D();

            //now fill the histogram
            std::size_t count = 0;
            for(std::size_t i=0; i<nxbins; i++)
            {
                for(std::size_t j=0; j<nybins; j++)
                {
                    h->SetPoint(count, x_axis(i), y_axis(j), table(i,j) );
                    count++;
                }
            }

            f2DGraph.push_back(h);
            return h;
        }

    private:

        std::vector<TGraph*> f1DGraph;
        std::vector<TGraph2D*> f2DGraph;

};


}//end of namespace

#endif /* end of include guard: MHO_RootGraphManager */