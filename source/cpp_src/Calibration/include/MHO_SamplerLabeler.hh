#ifndef MHO_SamplerLabeler_HH__
#define MHO_SamplerLabeler_HH__

#include <map>
#include <stack>
#include <string>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_SamplerLabeler.hh
 *@class MHO_SamplerLabeler
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Dec 13 16:55:25 2023 -0500
 *@brief When the 'samplers' keyword is encountered, this operator loops
 * over all channels and inserts a label for each channel which contains the sampler index
 * associated with that channel. This can later be used to look up the station sampler delay (ambiguities)
 * for this channel by the pcal operators
 * e.g.:
 * samplers 4 abcdefgh ijklmnop qrstuvwx yzABCDEF
 */

/**
 * @brief Class MHO_SamplerLabeler
 */
template< typename XArrayType > class MHO_SamplerLabeler: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_SamplerLabeler()
        {
            fComma = ",";
            fChannelLabelKey = "channel_label";
            fRefSamplerIndexKey = "ref_sampler_index";
            fRemSamplerIndexKey = "rem_sampler_index";
        };

        virtual ~MHO_SamplerLabeler(){};

        /**
         * @brief Setter for reference station sampler channel sets
         *
         * @param channel_sets Input vector of string channel set names
         */
        void SetReferenceStationSamplerChannelSets(const std::vector< std::string >& channel_sets)
        {
            fRefSamplerChanSets = channel_sets;
        }

        /**
         * @brief Setter for remote station sampler channel sets
         *
         * @param channel_sets Input vector of strings representing channel sets
         */
        void SetRemoteStationSamplerChannelSets(const std::vector< std::string >& channel_sets)
        {
            fRemSamplerChanSets = channel_sets;
        }

    protected:
        /**
         * @brief Initializes in-place mapping for channel labels to sampler indices.
         *
         * @param in Input XArrayType pointer for initializing in-place.
         * @return True if initialization is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override { return true; }

        /**
         * @brief Initializes output array in-place from input array.
         *
         * @param !in Const reference to input XArrayType
         * @param !out Reference to output XArrayType
         * @return Boolean indicating success of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* /*!in*/, XArrayType* /*!out*/) override { return true; }

        /**
         * @brief Function ExecuteInPlace - actual implementation, map channel label (e.g. 'a', 'b', etc.) to sampler
         * index for both reference and remote stations, and attaches label to visibility data
         *
         * @param in (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            //map channel label (e.g. 'a', 'b', etc.) to sampler index for both reference and remote stations
            ConstructChannelToSamplerIDMap(fRefSamplerChanSets, fRefChanToSamplerID);
            ConstructChannelToSamplerIDMap(fRemSamplerChanSets, fRemChanToSamplerID);

            if(in != nullptr)
            {
                //need to retrieve the labels of each channel, then look up the
                //sampler delay index using the map, and then insert that key:value pair
                auto chan_axis_ptr = &(std::get< CHANNEL_AXIS >(*in));
                std::size_t nchans = chan_axis_ptr->GetSize();

                for(std::size_t ch = 0; ch < nchans; ch++)
                {
                    std::string chan_label = "";
                    chan_axis_ptr->RetrieveIndexLabelKeyValue(ch, fChannelLabelKey, chan_label);
                    //add sampler labels
                    if(fRefChanToSamplerID.find(chan_label) != fRefChanToSamplerID.end())
                    {
                        int ref_id = fRefChanToSamplerID[chan_label];
                        chan_axis_ptr->InsertIndexLabelKeyValue(ch, fRefSamplerIndexKey, ref_id);
                    }
                    if(fRemChanToSamplerID.find(chan_label) != fRemChanToSamplerID.end())
                    {
                        int rem_id = fRemChanToSamplerID[chan_label];
                        chan_axis_ptr->InsertIndexLabelKeyValue(ch, fRemSamplerIndexKey, rem_id);
                    }
                }
                return true;
            }
            return false;
        }

        /**
         * @brief Copies input array and executes in-place operation on output.
         *
         * @param in Const reference to input XArrayType
         * @param out Reference to output XArrayType
         * @return Result of ExecuteInPlace operation on out
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            out->Copy(*in);
            return ExecuteInPlace(out);
        }

    private:
        //data maps channels to sampler
        std::vector< std::string > fRefSamplerChanSets;
        std::vector< std::string > fRemSamplerChanSets;
        std::map< std::string, int > fRefChanToSamplerID;
        std::map< std::string, int > fRemChanToSamplerID;

        std::string fComma;
        std::string fChannelLabelKey;
        std::string fRefSamplerIndexKey;
        std::string fRemSamplerIndexKey;
        MHO_Tokenizer fTokenizer;

        /**
         * @brief Constructs a map from channel labels to sampler IDs.
         *
         * @param chan_set Input vector of channel label strings.
         * @param chan2id Output map assigning each channel label an integer ID.
         */
        void ConstructChannelToSamplerIDMap(std::vector< std::string >& chan_set, std::map< std::string, int >& chan2id)
        {
            //figure out which sampler index each channel has been assigned to
            for(std::size_t sampler_id = 0; sampler_id < chan_set.size(); sampler_id++)
            {
                std::string chans = chan_set[sampler_id];
                std::vector< std::string > split_chans = SplitChannelLabels(chans);
                for(auto it = split_chans.begin(); it != split_chans.end(); it++)
                {
                    chan2id[*it] = (int)sampler_id;
                }
            }
        }

        /**
         * @brief Function SplitChannelLabels, splits the channel strings into individual channel labels
         *
         * @param channels (std::string)
         * @return Return value (std::string)
         */
        std::vector< std::string > SplitChannelLabels(std::string channels)
        {
            std::vector< std::string > split_chans;
            //we have two possibilities, either channels are delimited by ',' or
            //they are all single-char labels that are smashed together into a single string

            if((channels.find(',') != std::string::npos))
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
                for(std::size_t i = 0; i < channels.size(); i++)
                {
                    split_chans.push_back(std::string(1, channels[i]));
                }
            }
            return split_chans;
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_SamplerLabeler_HH__ */
