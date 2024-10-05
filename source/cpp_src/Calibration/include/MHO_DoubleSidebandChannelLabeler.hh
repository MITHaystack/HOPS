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

template< typename XArrayType > class MHO_DoubleSidebandChannelLabeler: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_DoubleSidebandChannelLabeler()
        {
            fEps = 1e-6; //tolerance when check if channels share a sky freq
        };

        virtual ~MHO_DoubleSidebandChannelLabeler(){};

        //allow channel freq association to use a difference tolerance
        void SetTolerance(double tol) { fEps = tol; }

    protected:
        virtual bool InitializeInPlace(XArrayType* in) override { return true; }

        virtual bool InitializeOutOfPlace(const XArrayType* /*!in*/, XArrayType* /*!out*/) override { return true; }

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(in != nullptr)
            {
                //need to use the user provided frequency <-> channel label map
                auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
                std::size_t nchans = chan_ax->GetSize();

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

                                //make sure each channel has a RELATIVE reference to index of the other
                                chan_ax->InsertIndexLabelKeyValue(ch, "dsb_partner", 1);
                                chan_ax->InsertIndexLabelKeyValue(next_ch, "dsb_partner", -1);
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
        double fEps;
};

} // namespace hops

#endif /*! end of include guard: MHO_DoubleSidebandChannelLabeler_HH__ */
