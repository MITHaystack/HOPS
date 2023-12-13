#ifndef MHO_SamplerLabeller_HH__
#define MHO_SamplerLabeller_HH__

#include <string>
#include <map>
#include <stack>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_UnaryOperator.hh"



/*
*File: MHO_SamplerLabeller.hh
*Class: MHO_SamplerLabeller
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: When the 'samplers' keyword is encountered, this operator loops 
* over all channels and inserts a label for each channel which contains the sampler index 
* associated with that channel. This can later be used to look up the station sampler delay (ambiguities)
* for this channel by the pcal operators
* e.g.:
* samplers 4 abcdefgh ijklmnop qrstuvwx yzABCDEF
*/


namespace hops
{

template< typename XArrayType >
class MHO_SamplerLabeller: public MHO_UnaryOperator< XArrayType >
{
    public:

        MHO_SamplerLabeller()
        {
            fComma = ",";
            fChannelLabelKey = "channel_label";
            fSamplerIndexKey = "sampler_index";
        };
        
        virtual ~MHO_SamplerLabeller(){};

        void SetSamplerChannelSets(const std::vector< std::string >& channel_sets){fSamplerChanSets = channel_sets;}

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override {return true;}
        virtual bool InitializeOutOfPlace(const XArrayType* /*in*/, XArrayType* /*out*/) override {return true;}

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            ConstructChannelToSamplerIDMap();
            if(in != nullptr)
            {
                //need to retrieve the labels of each channel, then look up the 
                //sampler delay index using the map, and then insert that key:value pair
                auto chan_axis_ptr = &( std::get<CHANNEL_AXIS>(*in) );
                std::size_t nchans = chan_axis_ptr->GetSize();
            
                for(std::size_t ch=0; ch<nchans; ch++)
                {
                    std::string chan_label = "";
                    auto labels = chan_axis_ptr->GetIntervalsWhichIntersect(ch);
                    for(auto iter = labels.begin(); iter != labels.end(); iter++)
                    {
                        if(iter->IsValid())
                        {
                            if( iter->HasKey(fChannelLabelKey) )
                            {
                                iter->Retrieve(fChannelLabelKey, chan_label);
                                break;
                            }
                        }
                    }

                    //this inserts an entirely new label (is this necessary?)
                    //why not just add the key:value to an existing interval label?
                    if( fChanToSamplerID.find(chan_label) != fChanToSamplerID.end() )
                    {
                        MHO_IntervalLabel label(ch,ch);
                        label.Insert(fSamplerIndexKey, fChanToSamplerID[chan_label] );
                        chan_axis_ptr->InsertLabel(label);
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
        
        //data
        std::vector< std::string > fSamplerChanSets;
        std::map< std::string, int > fChanToSamplerID;
        
        std::string fComma;
        std::string fChannelLabelKey;
        std::string fSamplerIndexKey;
        MHO_Tokenizer fTokenizer;

        void ConstructChannelToSamplerIDMap()
        {
            //figure out which sampler index each channel has been assigned to
            for(std::size_t sampler_id = 0; sampler_id < fSamplerChanSets.size(); sampler_id++)
            {
                std::string chans = fSamplerChanSets[sampler_id];
                std::vector< std::string > split_chans = SplitChannelLabels(chans);
                for(auto it = split_chans.begin(); it != split_chans.end(); it++)
                {
                    fChanToSamplerID[ *it ] = (int)sampler_id;
                }
            }
        }
        
        std::vector< std::string > SplitChannelLabels(std::string channels)
        {
            std::vector< std::string > split_chans;
            //we have two possibilities, either channels are delimited by ',' or 
            //they are all single-char labels that are smashed together into a single string
            
            if( (channels.find(',') != std::string::npos) )
            {
                //found a comma, need to use the tokenizer
                fTokenizer.SetDelimiter(fComma);
                fTokenizer.SetUseMulticharacterDelimiterFalse();
                fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
                fTokenizer.SetIncludeEmptyTokensFalse();
                fTokenizer.SetString(&channels);
                fTokenizer.GetTokens(&split_chans);
            }
            else 
            {
                //just split up the channels into individual characters
                for(std::size_t i=0; i< channels.size(); i++)
                {
                    split_chans.push_back( std::string(1, channels[i]) );
                }
            }
            return split_chans;
        }


};


} //end of namespace

#endif /* end of include guard: MHO_SamplerLabeller_HH__ */
