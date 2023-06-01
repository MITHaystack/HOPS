#ifndef MHO_ChannelLabeller_HH__
#define MHO_ChannelLabeller_HH__

#include <string>
#include <map>
#include <stack>

#include "MHO_Message.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_UnaryOperator.hh"

/*
*File: MHO_ChannelLabeller.hh
*Class: MHO_ChannelLabeller
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Applies 'fourfit' labels to each channel (e.g. a, b,...), if no map is supplied then 
the default mapping is in order of frequency low -> high, starting with 'a'
*/


namespace hops
{

template< typename XArrayType > 
class MHO_ChannelLabeller: public MHO_UnaryOperator< XArrayType >
{
    public:

        MHO_ChannelLabeller()
        {
            //we inherited this set of 64 characters from fourfit
            fChannelChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%";
            fBase = fChannelChars.size();
            fIndexToChannelLabel.clear();
        };
        
        virtual ~MHO_ChannelLabeller(){};

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override {return true;}

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            //first determine the number of channels the input array has.
            std::size_t nchans = in->GetDimension(CHANNEL_AXIS);
            //the first 64 channels are given single character labels 'a', 'b', etc.
            //if we have more than 64 channels we start labelling channels with 2-chars
            //from this set, starting with 'ba', 'bc', ... 'b%', 'ca', ...
            //if we still have more than 4096 channels, we move to 3-char labels and so on.
            FillIndexToChannelLabel(nchans);

            
            
            return true;
        }
        
        virtual bool InitializeOutOfPlace(const XArrayType* /*in*/, XArrayType* /*out*/) override {return true;}

        
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            out->Copy(*in);
            return ExecuteInPlace(out);
        }

    private:
        
    public:
        
        void FillIndexToChannelLabel(std::size_t nchans)
        {
            fIndexToChannelLabel.clear();
            for(std::size_t i=0; i<nchans; i++)
            {
                std::stack<char> cstack;
                std::size_t q,r;
                q = i;
                do
                {
                    r = q%64; 
                    q = q/64; 
                    cstack.push(fChannelChars[r]);
                }
                while( q > 0);
                
                std::string ch_label = "";
                while(cstack.size() != 0)
                {
                    ch_label += cstack.top();
                    cstack.pop();
                }
                
                fIndexToChannelLabel[i] = ch_label;
                std::cout<<i<<" : "<<ch_label<<std::endl;
            }
        }
        
        //user supplied channel label to frequency map (if available)
        std::map< std::string, double > fChannelLabelToFrequency; 
        //legal characters in channel labels
        std::string fChannelChars;
        std::size_t fBase;
        //channel index to channel name map
        std::map<std::size_t, std::string > fIndexToChannelLabel; 
};
    
    
} //end of namespace

#endif /* end of include guard: MHO_ChannelLabeller_HH__ */
