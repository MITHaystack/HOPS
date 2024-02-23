#ifndef MHO_RootHistogramManager_HH__
#define MHO_RootHistogramManager_HH__

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

/*!
*@file  MHO_RootHistogramManager.hh
*@class  MHO_RootHistogramManager
*@author  J. Barrett - barrettj@mit.edu 
*
*@date 
*@brief 
*/

namespace hops
{

class MHO_RootHistogramManager
{
    public:

        MHO_RootHistogramManager(){};

        virtual ~MHO_RootHistogramManager()
        {
            for(unsigned int i=0; i<f1DHist.size(); i++){delete f1DHist[i];}
            for(unsigned int i=0; i<f2DHist.size(); i++){delete f2DHist[i];}
            for(unsigned int i=0; i<f3DHist.size(); i++){delete f3DHist[i];}
        }

        template< typename XTableType >
        TH1D* GenerateHistogram1D(std::string name, XTableType* table)
        {
            //assert that this is a 1d table
            HOPS_ASSERT_EQUAL( table->GetRank(), 1 );

            //assume axis is labeled by doubles
            auto x_axis = (std::get<0>(*table));
            std::size_t nxbins = table->GetDimension(0);
            double* xbins = x_axis.GetData();

            TH1D* h = new TH1D(name.c_str(), name.c_str(), nxbins, xbins);

            //now fill the histogram
            for(std::size_t i=0; i<nxbins; i++)
            {
                h->Fill(x_axis(i), table->at(i) );
            }

            f1DHist.push_back(h);
            return h;
        }

        template< typename XTableType >
        TH2D* GenerateHistogram2D(std::string name, XTableType* table)
        {
            HOPS_ASSERT_EQUAL( table->GetRank(), 2 );

            //assume axes are labeled by doubles
            auto x_axis = (std::get<0>(*table));
            std::size_t nxbins = table->GetDimension(0);
            double* xbins = x_axis.GetData();

            auto y_axis = (std::get<1>(*table));
            std::size_t nybins = table->GetDimension(1);
            double* ybins = y_axis.GetData();

            TH2D* h = new TH2D(name.c_str(), name.c_str(), nxbins, xbins, nybins, ybins);

            //now fill the histogram
            for(std::size_t i=0; i<nxbins; i++)
            {
                for(std::size_t j=0; j<nybins; j++)
                {
                    h->Fill(x_axis(i), y_axis(j), table->at(i,j) );
                }
            }

            f2DHist.push_back(h);
            return h;
        }

        template< typename XTableType >
        TH3D* GenerateHistogram3D(std::string name, XTableType* table)
        {
            HOPS_ASSERT_EQUAL( table->GetRank(), 3 );

            //assume axes are labeled by doubles
            auto x_axis = (std::get<0>(*table));
            std::size_t nxbins = table->GetDimension(0);
            double* xbins = x_axis.GetData();

            auto y_axis = (std::get<1>(*table));
            std::size_t nybins = table->GetDimension(1);
            double* ybins = y_axis.GetData();

            auto z_axis = (std::get<2>(*table));
            std::size_t nzbins = table->GetDimension(2);
            double* zbins = z_axis.GetData();

            TH3D* h = new TH3D(name.c_str(), name.c_str(), nxbins, xbins, nybins, ybins, nzbins, zbins);

            //now fill the histogram
            for(std::size_t i=0; i<nxbins; i++)
            {
                for(std::size_t j=0; j<nybins; j++)
                {
                    for(std::size_t k=0; k<nzbins; k++)
                    {
                        h->Fill(x_axis(i), y_axis(j), z_axis(k), table->at(i,j,k) );
                    }
                }
            }

            f3DHist.push_back(h);
            return h;
        }


    private:

        std::vector<TH1D*> f1DHist;
        std::vector<TH2D*> f2DHist;
        std::vector<TH3D*> f3DHist;

};


}//end of namespace

#endif /*! end of include guard: MHO_RootHistogramManager */