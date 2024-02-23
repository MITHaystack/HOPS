#ifndef MHO_IntervalLabel_HH__
#define MHO_IntervalLabel_HH__

/*!
*@file MHO_IntervalLabel.hh
*@class MHO_IntervalLabel
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

#include <string>
#include <utility>

#include "MHO_Types.hh"
#include "MHO_Interval.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_Serializable.hh"

namespace hops
{


class MHO_IntervalLabel:
    public MHO_Interval< std::size_t >,
    public MHO_CommonLabelMap,
    public MHO_Serializable
{
    public:

        MHO_IntervalLabel();
        MHO_IntervalLabel( std::size_t lower_bound, std::size_t upper_bound);
        MHO_IntervalLabel(const MHO_IntervalLabel& copy);
        virtual ~MHO_IntervalLabel();

        bool HasKey(const std::string& key) const;

        void SetIsValidFalse(){fIsValid = false;};
        void SetIsValidTrue(){fIsValid = true;}
        bool IsValid() const {return fIsValid;}

        MHO_IntervalLabel& operator=(const MHO_IntervalLabel& rhs)
        {
            if(this != &rhs)
            {
                SetIntervalImpl(rhs.fLowerBound, rhs.fUpperBound );
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
                this->CopyFrom<std::string>(rhs);
            }
            return *this;
        }

    private:

        bool fIsValid;

    public: //MHO_Serializable interface

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number
            total_size += 2*sizeof(std::size_t); //upper/lower bounds

            total_size += cm_aggregate_serializable_item_size<char>(*this);
            total_size += cm_aggregate_serializable_item_size<bool>(*this);
            total_size += cm_aggregate_serializable_item_size<int>(*this);
            total_size += cm_aggregate_serializable_item_size<double>(*this);
            total_size += cm_aggregate_serializable_item_size<std::string>(*this);

            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_IntervalLabel& aData)
        {
            MHO_ClassVersion vers;
            s >> vers;
            switch(vers)
            {
                case 0:
                    aData.StreamInData_V0(s);
                break;
                default:
                    MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                    //Flag this as an unknown object version so we can skip over this data
                    MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            return s;
        }

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_IntervalLabel& aData)
        {
            switch( aData.GetVersion() )
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers",
                        "error, cannot stream out MHO_IntervalLabel object with unknown version: "
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            //first need to stream the integer interval pair
            uint64_t low, up;
            s >> low;
            s >> up;
            this->SetBounds(low, up);

            cm_stream_importer<XStream, char>(s, *this);
            cm_stream_importer<XStream, bool>(s, *this);
            cm_stream_importer<XStream, int>(s, *this);
            cm_stream_importer<XStream, double>(s, *this);
            cm_stream_importer<XStream, std::string>(s, *this);
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            //first need to stream the integer interval pair
            s << (uint64_t) this->GetLowerBound();
            s << (uint64_t) this->GetUpperBound();

            cm_stream_exporter<XStream, char>(s, *this);
            cm_stream_exporter<XStream, bool>(s, *this);
            cm_stream_exporter<XStream, int>(s, *this);
            cm_stream_exporter<XStream, double>(s, *this);
            cm_stream_exporter<XStream, std::string>(s, *this);
        };

        virtual MHO_UUID DetermineTypeUUID() const override
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(*this);
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }

};

}

#endif /*! end of include guard: MHO_IntervalLabel */
