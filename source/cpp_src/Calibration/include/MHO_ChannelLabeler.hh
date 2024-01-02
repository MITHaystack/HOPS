#ifndef MHO_ChannelLabeler_HH__
#define MHO_ChannelLabeler_HH__

#include <string>
#include <map>
#include <stack>

#include "MHO_Message.hh"

#include "MHO_EncodeDecodeValue.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_UnaryOperator.hh"



/*
*File: MHO_ChannelLabeler.hh
*Class: MHO_ChannelLabeler
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Applies 'fourfit' labels to each channel (e.g. a, b,...),
* if no user-defined map is supplied then the default mapping is in order
* of frequency low -> high, starting with 'a'
*/


namespace hops
{

template< typename XArrayType >
class MHO_ChannelLabeler: public MHO_UnaryOperator< XArrayType >
{
    public:

        MHO_ChannelLabeler()
        {
            //we inherited the set of 64 characters from fourfit
            //consider how we may want to change this in the future
            #pragma message("TODO FIXME: re-think mult-char labeling scheme, what is most user friendly?")
            fDefaultChannelChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%";
            fExtendedChannelChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //used beyond 64
            fIndexToChannelLabel.clear();
            fEps = 1e-4; //tolerance when mapping freq to indices
            fChannelLabelKey = "channel_label";
        };

        virtual ~MHO_ChannelLabeler(){};

        //allow channel freq association to use a difference tolerance
        void SetTolerance(double tol){fEps = tol;}

        //provide the option to use different character sets
        void SetDefaultChannelChars(const std::string& ch_set){fDefaultChannelChars = ch_set;}
        void SetExtendedChannelChars(const std::string& ex_set){fExtendedChannelChars = ex_set;}

        //if there is a user provided labeling scheme, use that (i.e. chan_ids)
        void SetChannelLabelToFrequencyMap(const std::map< std::string, double >& map)
        {
            fChannelLabelToFrequency = map;
        }

        //default encoding/decoding scheme
        std::string EncodeValueToLabel(const uint64_t& value) const
        {
            if(value < fDefaultChannelChars.size()){return encode_value(value, fDefaultChannelChars);}
            //else multi-char code
            uint64_t j = value - fDefaultChannelChars.size() + fExtendedChannelChars.size();
            return encode_value(j, fExtendedChannelChars);
        }

        uint64_t DecodeLabelToValue(const std::string& label) const
        {
            if(label.size() == 1){ return decode_value(label, fDefaultChannelChars);}
            //else multi-char code
            uint64_t extended_value = decode_value(label, fExtendedChannelChars);
            extended_value -= fExtendedChannelChars.size();
            extended_value += fDefaultChannelChars.size();
            return extended_value;
        }

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override {return true;}
        virtual bool InitializeOutOfPlace(const XArrayType* /*in*/, XArrayType* /*out*/) override {return true;}

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(in != nullptr)
            {
                //need to use the user provided frequency <-> channel label map
                auto chan_axis_ptr = &( std::get<CHANNEL_AXIS>(*in) );
                std::size_t nchans = chan_axis_ptr->GetSize();

                if(fChannelLabelToFrequency.size() == 0)
                {
                    //apply default channel labels
                    FillDefaultMap(nchans);
                    for(std::size_t i=0; i<nchans; i++)
                    {
                        chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, fIndexToChannelLabel[i]);
                    }
                }
                else
                {
                    if(fChannelLabelToFrequency.size() < nchans)
                    {
                        msg_warn("calibration", "not all channels given a user specified label, "
                                  << "some channels will remain un-labelled." << eom);
                    }

                    //now do a brute force search over the channel frequencies, and
                    //determine which ones match the labels we've been given
                    for(auto it= fChannelLabelToFrequency.begin(); it != fChannelLabelToFrequency.end(); it++)
                    {
                        std::string ch_label = it->first;
                        double freq = it->second;
                        for(std::size_t i=0; i<nchans; i++)
                        {
                            double ch_freq = chan_axis_ptr->at(i);
                            if( std::abs(freq - ch_freq) < fEps )
                            {
                                chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label);
                                break;
                            }
                        }
                    }

                }
                return true;
            }
            return false;
        }

        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            out->Copy(*in);
            return ExecuteInPlace(out);
        }

    private:

        void FillDefaultMap(std::size_t nchans)
        {
            fIndexToChannelLabel.clear();
            for(std::size_t i=0; i<nchans; i++)
            {
                fIndexToChannelLabel[i] = EncodeValueToLabel(i);
            }
        }

        //data
        std::string fChannelLabelKey;

        //user supplied channel label to frequency map (if available)
        std::map< std::string, double > fChannelLabelToFrequency;
        double fEps;

        //legal characters in channel labels
        std::string fDefaultChannelChars;
        std::string fExtendedChannelChars;
        //channel index to channel name map
        std::map<std::size_t, std::string > fIndexToChannelLabel;
};


} //end of namespace

#endif /* end of include guard: MHO_ChannelLabeler_HH__ */
