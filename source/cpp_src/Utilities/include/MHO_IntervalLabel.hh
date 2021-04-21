#ifndef MHO_IntervalLabel_HH__
#define MHO_IntervalLabel_HH__

/*
*File: MHO_IntervalLabel.hh
*Class: MHO_IntervalLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cstdint>
#include <string>
#include <utility>

#include "MHO_Interval.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_Serializable.hh"

namespace hops
{

//TODO: Make sure this set of types is complete for data-labelling.
//Consider what other types might be needed (float? short? dates?)
typedef MHO_MultiTypeMap< std::string, char, bool, int, double, std::string > MHO_IntervalLabelMap;

class MHO_IntervalLabel:
    public MHO_Interval< std::size_t >,
    public MHO_IntervalLabelMap,
    virtual public MHO_Serializable
{
    public:

        MHO_IntervalLabel();
        MHO_IntervalLabel( std::size_t lower_bound, std::size_t upper_bound);
        MHO_IntervalLabel(const MHO_IntervalLabel& copy);
        virtual ~MHO_IntervalLabel();

        bool HasKey(const std::string& key) const;

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

        //none

    public: //MHO_Serializable interface

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number

            total_size += 2*sizeof(std::size_t); //upper/lower bounds

            std::vector< std::string > keys;
            keys = this->DumpKeys< char >();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += keys[i].size();
                total_size += sizeof(char);
            }
            keys.clear();

            keys = this->DumpKeys<bool>();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += keys[i].size();
                total_size += sizeof(bool);
            }
            keys.clear();

            keys = this->DumpKeys<int>();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += keys[i].size();
                total_size += sizeof(int);
            }
            keys.clear();

            keys = this->DumpKeys<double>();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += keys[i].size();
                total_size += sizeof(double);
            }
            keys.clear();

            keys = this->DumpKeys<std::string>();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                std::string val;
                total_size += keys[i].size();
                Retrieve(keys[i], val);
                total_size += val.size();
            }
            keys.clear();
            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_IntervalLabel& aData)
        {
            //first item to stream is always the version number
            MHO_ClassVersion vers;
            s >> vers;
            //check if incoming data is equal to the current class version
            if( vers != aData.GetVersion() )
            {
                MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                //Flag this as an unknown object version so we can skip over this data
                MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            else
            {
                //first need to stream the integer interval pair
                std::size_t low, up;
                s >> low;
                s >> up;
                aData.SetBounds(low, up);

                //then for each type in the multi-type map, we need to stream in
                //the size of the map and then each subsequent key-value pair
                std::size_t n_elem;
                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    char val;
                    s >> key;
                    s >> val;
                    aData.Insert(key, val);
                }
                n_elem = 0;

                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    bool val;
                    s >> key;
                    s >> val;
                    aData.Insert(key, val);
                }
                n_elem = 0;

                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    int val;
                    s >> key;
                    s >> val;
                    aData.Insert(key, val);
                }
                n_elem = 0;

                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    double val;
                    s >> key;
                    s >> val;
                    aData.Insert(key, val);
                }
                n_elem = 0;

                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    std::string val;
                    s >> key;
                    s >> val;
                    aData.Insert(key, val);
                }
                n_elem = 0;
            }
            return s;
        };

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_IntervalLabel& aData)
        {
            //first item to stream is always the version number
            s << aData.GetVersion();

            //first need to stream the integer interval pair
            s << aData.GetLowerBound();
            s << aData.GetUpperBound();

            //then for each type in the multi-type map, we need to stream out
            //the size of the map and then each subsequent key-value pair

            std::vector< std::string > keys;
            keys = aData.DumpKeys< char >();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                char val;
                aData.Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = aData.DumpKeys<bool>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                bool val;
                aData.Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = aData.DumpKeys<int>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                int val;
                aData.Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = aData.DumpKeys<double>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                double val;
                aData.Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = aData.DumpKeys<std::string>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                std::string val;
                aData.Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            return s;
        };


};

}

#endif /* end of include guard: MHO_IntervalLabel */
