#ifndef MHO_ChannelLabeller_HH__
#define MHO_ChannelLabeller_HH__

#include <string>
#include <map>

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
            //from this set, like: 'ab', 'ac', ... 'a%', 'ba', ...
            //if we still have more than 4096 channels, we move to 3-char labels and so on.
            
            
            

        }
        
        virtual bool InitializeOutOfPlace(const XArrayType* /*in*/, XArrayType* /*out*/) override {return true;}

        
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            out->Copy(*in);
            ExecuteInPlace(out);
        }

    private:
        
        //user supplied channel label to frequency map (if available)
        std::map< std::string, double > fChannelNameToFrequency; 
        //legal characters in channel labels
        std::string fChannelChars;
        //channel index to channel name map
        std::map<std::size_t, std::string > fIndexToChannelName; 
};
    
    
} //end of namespace

#endif /* end of include guard: MHO_ChannelLabeller_HH__ */
