#ifndef MHO_DoubleSidebandChannelLabeler_HH__
#define MHO_DoubleSidebandChannelLabeler_HH__

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
 *@file MHO_DoubleSidebandChannelLabeler.hh
 *@class MHO_DoubleSidebandChannelLabeler
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jun 1 13:20:19 2023 -0400
 *@brief  Detects adjacent LSB/USB channels pairs which share the same sky-frequency and bandwidth.
 * These are then marked as 'double-sideband' channels so they can recieve the legacy treatment
 *
 */

/**
 * @brief Class MHO_DoubleSidebandChannelLabeler
 */
template< typename XArrayType > class MHO_DoubleSidebandChannelLabeler: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_DoubleSidebandChannelLabeler()
        {
            fEps = 1e-6; //tolerance (MHz) when checking if channels share a sky freq
        };

        virtual ~MHO_DoubleSidebandChannelLabeler(){};

        //allow channel freq association to use a difference tolerance
        /**
         * @brief Setter for tolerance - in (MHz) when checking if channels share a sky freq
         * 
         * @param tol New tolerance value to use when checking if channels share a sky frequency.
         */
        void SetTolerance(double tol) { fEps = tol; }

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
         * @brief Initializes output array in-place from input array.
         * 
         * @param !in Const reference to input XArrayType
         * @param !out Reference to output XArrayType
         * @return Boolean indicating success of initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* /*!in*/, XArrayType* /*!out*/) override { return true; }

        /**
         * @brief Function ExecuteInPlace labels LSB/USB channel pairs as "double sideband" channels if they share and edge
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
                auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
                std::size_t nchans = chan_ax->GetSize();

                int double_sideband_pair_counter = 0; //count unique double-sideband pairs
                for(std::size_t ch = 0; ch < nchans - 1; ch++)
                {
                    std::size_t next_ch = ch + 1;
                    double f1 = chan_ax->at(ch);
                    double f2 = chan_ax->at(next_ch);

                    double bw1 = 0;
                    double bw2 = 0;
                    bool bw1_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bw1);
                    bool bw2_present = chan_ax->RetrieveIndexLabelKeyValue(next_ch, "bandwidth", bw2);

                    if(std::abs(f1 - f2) < fEps && std::abs(bw1 - bw2) < fEps)
                    {
                        //sky frequencies of these two channels are the same
                        //now check if they are a LSB/USB pair
                        std::string nsb1 = "";
                        std::string nsb2 = "";
                        bool nsb1_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", nsb1);
                        bool nsb2_present = chan_ax->RetrieveIndexLabelKeyValue(next_ch, "net_sideband", nsb2);

                        if(nsb1_present && nsb2_present)
                        {
                            //1st channel is LSB, 2nd channel is USB --> we have a 'double-sideband' channel
                            //note: we ignore the oddball case where U and L are inverted
                            if(nsb1 == "L" && nsb2 == "U")
                            {
                                bool value = true;
                                chan_ax->InsertIntervalLabelKeyValue(ch, next_ch, "double_sideband", value);

                                //make sure each channel has a reference to index of the other via a RELATIVE offset
                                chan_ax->InsertIndexLabelKeyValue(ch, "dsb_partner", 1);
                                chan_ax->InsertIndexLabelKeyValue(next_ch, "dsb_partner", -1);
                            }
                            double_sideband_pair_counter++;
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
        double fEps;
};

} // namespace hops

#endif /*! end of include guard: MHO_DoubleSidebandChannelLabeler_HH__ */
