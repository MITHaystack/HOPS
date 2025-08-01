#ifndef MHO_ChannelLabeler_HH__
#define MHO_ChannelLabeler_HH__

#include <map>
#include <stack>
#include <string>

#include "MHO_Message.hh"

#include "MHO_EncodeDecodeValue.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_ChannelLabeler.hh
 *@class MHO_ChannelLabeler
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jun 1 13:20:19 2023 -0400
 *@brief Applies 'fourfit' labels to each channel (e.g. a, b,...),
 * if no user-defined map is supplied then the default mapping is in order
 * of frequency low -> high, starting with 'a'
 */

/**
 * @brief Class MHO_ChannelLabeler
 */
template< typename XArrayType > class MHO_ChannelLabeler: public MHO_UnaryOperator< XArrayType >, public MHO_ChannelIndexLabeler
{
    public:
        MHO_ChannelLabeler()
        {
            fIndexToChannelLabel.clear();
            fEps = 1e-4; //tolerance when mapping freq to indices
            fChannelLabelKey = "channel_label";
        };

        virtual ~MHO_ChannelLabeler(){};

        /**
         * @brief Setter for tolerance - allows channel freq association to use a (freq) difference tolerance
         *
         * @param tol New tolerance value for frequency mapping
         */
        void SetTolerance(double tol) { fEps = tol; }

        /**
         * @brief Setter for channel label to frequency map so if there is a user provided labeling scheme, use that (i.e. chan_ids)
         *
         * @param map Input map of channel labels to frequencies
         */
        void SetChannelLabelToFrequencyMap(const std::map< std::string, double >& map) { fChannelLabelToFrequency = map; }

    protected:
        /**
         * @brief Initializes XArrayType in-place and returns success.
         *
         * @param in Pointer to XArrayType object to initialize.
         * @return True if initialization was successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override { return true; }

        /**
         * @brief Initializes output array out-of-place from input array
         *
         * @param !in Const input XArrayType
         * @param !out Output XArrayType initialized out-of-place
         * @return True if initialization was successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* /*!in*/, XArrayType* /*!out*/) override { return true; }

        /**
         * @brief Function ExecuteInPlace - attaches channel labels based on sky frequency or user specified map
         *
         * @param in (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(in != nullptr)
            {
                //need to use the user provided frequency <-> channel label map
                auto chan_axis_ptr = &(std::get< CHANNEL_AXIS >(*in));
                std::size_t nchans = chan_axis_ptr->GetSize();

                //grab info about DSB channels (if it exists)
                std::vector< mho_json > dsb_labels = chan_axis_ptr->GetMatchingIntervalLabels("double_sideband");

                if(fChannelLabelToFrequency.size() == 0)
                {
                    //apply default channel labels
                    FillDefaultMap(nchans);
                    std::size_t label_count = 0;
                    for(std::size_t i = 0; i < nchans; i++)
                    {
                        std::string ch_label = fIndexToChannelLabel[label_count];

                        std::string dummy; //check if this channel was already labeled
                        bool already_labeled = chan_axis_ptr->RetrieveIndexLabelKeyValue(i, "channel_label", dummy);
                        if(!already_labeled)
                        {
                            chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label);
                            //check if this channel is a member of a double-sideband pair,
                            //and if so make sure its partner gets the same label
                            if(dsb_labels.size() != 0)
                            {
                                int partner_offset;
                                bool has_dsb_partner =
                                    chan_axis_ptr->RetrieveIndexLabelKeyValue(i, "dsb_partner", partner_offset);
                                int partner_idx = i + partner_offset;
                                if(has_dsb_partner && (0 <= partner_offset) && (partner_offset < nchans))
                                {
                                    // chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label);
                                    // chan_axis_ptr->InsertIndexLabelKeyValue(partner_idx, fChannelLabelKey, ch_label);
                                    chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label + "-");
                                    chan_axis_ptr->InsertIndexLabelKeyValue(partner_idx, fChannelLabelKey, ch_label + "+");
                                }
                            }
                            label_count++;
                        }
                    }
                }
                else
                {
                    if(fChannelLabelToFrequency.size() < nchans - dsb_labels.size())
                    {
                        msg_error("calibration", "not all channels given a user specified label, "
                                                     << "some channels will remain un-labelled." << eom);
                    }

                    //now do a brute force search over the channel frequencies, and
                    //determine which ones match the labels we've been given
                    for(auto it = fChannelLabelToFrequency.begin(); it != fChannelLabelToFrequency.end(); it++)
                    {
                        std::string ch_label = it->first;
                        double freq = it->second;
                        for(std::size_t i = 0; i < nchans; i++)
                        {
                            double ch_freq = chan_axis_ptr->at(i);
                            if(std::abs(freq - ch_freq) < fEps)
                            {
                                chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label);
                                //check if this channel is a member of a double-sideband pair,
                                //and if so make sure its partner gets the same label
                                if(dsb_labels.size() != 0)
                                {
                                    int partner_offset;
                                    bool has_dsb_partner =
                                        chan_axis_ptr->RetrieveIndexLabelKeyValue(i, "dsb_partner", partner_offset);
                                    int partner_idx = i + partner_offset;
                                    if(has_dsb_partner)
                                    {
                                        // chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label);
                                        // chan_axis_ptr->InsertIndexLabelKeyValue(partner_idx, fChannelLabelKey, ch_label);
                                        chan_axis_ptr->InsertIndexLabelKeyValue(i, fChannelLabelKey, ch_label + "-");
                                        chan_axis_ptr->InsertIndexLabelKeyValue(partner_idx, fChannelLabelKey, ch_label + "+");
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                return true;
            }
            return false;
        }

        /**
         * @brief Copies input array to output and executes in-place operation on output.
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
        /**
         * @brief Clears and populates a map with channel labels encoded from indices up to nchans.
         *
         * @param nchans Number of channels to generate labels for
         */
        void FillDefaultMap(std::size_t nchans)
        {
            fIndexToChannelLabel.clear();
            for(std::size_t i = 0; i < nchans; i++)
            {
                fIndexToChannelLabel[i] = EncodeValueToLabel(i);
            }
        }

        //data
        std::string fChannelLabelKey;

        //user supplied channel label to frequency map (if available)
        std::map< std::string, double > fChannelLabelToFrequency;
        double fEps;

        std::map< std::size_t, std::string > fIndexToChannelLabel;
};

} // namespace hops

#endif /*! end of include guard: MHO_ChannelLabeler_HH__ */
