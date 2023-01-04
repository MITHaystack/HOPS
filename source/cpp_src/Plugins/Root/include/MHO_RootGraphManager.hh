#ifndef MHO_RootGraphManager_HH__
#define MHO_RootGraphManager_HH__

#include "TAxis.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TMultiGraph.h"

#include <complex>

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

            //now fill the graph
            for(std::size_t i=0; i<nxbins; i++)
            {
                h->SetPoint(i, x_axis(i), table.at(i) );
            }

            f1DGraph.push_back(h);
            return h;
        }

        template< typename XTableType, typename XAxisType >
        TGraph* GenerateComplexGraph1D(const XTableType& table, const XAxisType& x_axis, int plot_mode )
        {
            //assert that this is a 1d table 
            HOPS_ASSERT_EQUAL( table.GetRank(), 1 );

            //assume axis is labeled by doubles 
            std::size_t nxbins = table.GetDimension(0);

            TGraph* h = new TGraph();

            //now fill the graph
            for(std::size_t i=0; i<nxbins; i++)
            {
                double value = 0;
                if(plot_mode == 0) //plot real part
                {
                    value = std::real( table(i) );
                }
                else if(plot_mode == 1) //plot imaginary part
                {
                    value = std::imag( table(i) );
                }
                else if(plot_mode == 2) //plot absolute value
                {
                    value = std::abs( table(i) );
                }
                else if(plot_mode == 3) //plot phase
                {
                    value = std::arg( table(i) );
                }
                else //plot magnitude squared
                {
                    value = std::real( table(i)*std::conj(table(i) ) );
                }

                h->SetPoint(i, x_axis(i), value);
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

            //now fill the graph
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


        template< typename XTableType, typename XAxisType, typename YAxisType >
        TGraph2D* GenerateComplexGraph2D(const XTableType& table, const XAxisType& x_axis, const YAxisType& y_axis, int plot_mode)
        {
            HOPS_ASSERT_EQUAL( table.GetRank(), 2 ); 

            std::size_t nxbins = table.GetDimension(0);
            std::size_t nybins = table.GetDimension(1);

            std::size_t xax_size = x_axis.GetSize();
            std::size_t yax_size = y_axis.GetSize();

            if(nxbins != xax_size){msg_error("root_plugin ", "table dimension and x-axis size mismatch." << eom); return nullptr;}
            if(nybins != yax_size){msg_error("root_plugin ", "table dimension and y-axis size mismatch." << eom); return nullptr;}

            TGraph2D* h = new TGraph2D();
            //now fill the graph
            std::size_t count = 0;
            for(std::size_t i=0; i<nxbins; i++)
            {
                for(std::size_t j=0; j<nybins; j++)
                {
                    //now fill the graph
                    double value = 0;
                    if(plot_mode == 0) //plot real part
                    {
                        value = std::real( table(i,j) );
                    }
                    else if(plot_mode == 1) //plot imaginary part
                    {
                        value = std::imag( table(i,j) );
                    }
                    else if(plot_mode == 2) //plot absolute value
                    {
                        value = std::abs( table(i,j) );
                    }
                    else if(plot_mode == 3) //plot phase
                    {
                        value = std::arg( table(i,j) );
                    }
                    else //plot magnitude squared
                    {
                        value = std::real( table(i,j)*std::conj(table(i,j) ) );
                    }
                    h->SetPoint(count, x_axis(i), y_axis(j), value );
                    count++;
                }
            }

            h->SetNpx(128);
            h->SetNpy(128);

            f2DGraph.push_back(h);
            return h;
        }


    private:

        std::vector<TGraph*> f1DGraph;
        std::vector<TGraph2D*> f2DGraph;

};


}//end of namespace

#endif /* end of include guard: MHO_RootGraphManager */