#ifndef MHO_JlTableContainer_HH__
#define MHO_JlTableContainer_HH__

#include <array>
#include <complex>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "MHO_ExtensibleElement.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_TemplateTypenameDeduction.hh"

#include "jlcxx/jlcxx.hpp"
#include "jlcxx/array.hpp"
#include "jlcxx/stl.hpp"

namespace hops
{

/*!
 *@file MHO_JlTableContainer.hh
 *@class MHO_JlTableContainer
 *@author J. Barrett - barrettj@mit.edu
 *@brief Julia (CxxWrap) bindings for template MHO_TableContainer objects.
 * Exposes the N-dimensional array as a jlcxx::ArrayRef<T,N> (backed by a
 * jl_array_t* created via jl_ptr_to_array), giving zero-copy access with
 * dimensions reversed for Julia column-major layout.  No reshape is required
 * on the Julia side.  JSON metadata is returned as std::string; Julia parses
 * with JSON3.jl.
 */

template< typename XTableType > class MHO_JlTableContainer
{
    public:
        static constexpr std::size_t RANK = XTableType::rank::value;
        using value_type = typename XTableType::value_type;

        MHO_JlTableContainer(MHO_ExtensibleElement* element): fElement(element)
        {
            fTable = dynamic_cast< XTableType* >(element);
        };

        virtual ~MHO_JlTableContainer(){};

        //----------------------------------------------------------------------
        // Array interface
        //----------------------------------------------------------------------

        /*!
         * Return an N-dimensional CxxWrap ArrayRef wrapping the C++ memory (zero-copy).
         *
         * Dimensions are passed in REVERSED order (see GetNDArray_impl) so Julia's
         * column-major first index aligns with the C++ row-major last dimension:
         *
         *   Julia arr[i_freq, i_time, i_channel, i_pol]  (1-based)
         *   <->  C++  arr[pol][channel][time][freq]         (0-based)
         *
         * No reshape is needed on the Julia side - the array already has the correct
         * Julia column-major shape.  Scalar element access (arr[i,j,k,l]) is zero-copy.
         * Slice / broadcast operations (arr[:, :, ch, pp]) require an explicit copy:
         *
         *   arr = copy(get_array(obj))
         *
         * Do NOT resize!, push!, or append! to the returned object.
         */
        jlcxx::ArrayRef< value_type, RANK > GetNDArray()
        {
            return GetNDArray_impl(std::make_index_sequence< RANK >{});
        }

        //! Logical dimensions in C++ order (dim[0]=slowest, dim[RANK-1]=fastest).
        std::vector< int64_t > GetDimensions() const
        {
            std::vector< int64_t > dims(RANK);
            for(std::size_t i = 0; i < RANK; i++) { dims[i] = static_cast< int64_t >(fTable->GetDimension(i)); }
            return dims;
        }

        //----------------------------------------------------------------------
        // Convenience accessors exposed as Julia methods
        //----------------------------------------------------------------------

        std::size_t GetRank() const { return RANK; }

        std::size_t GetDimension(std::size_t index) const { return fTable->GetDimension(index); }

        std::string GetClassName() const { return MHO_ClassName< XTableType >(); }

        //! Return the N-th axis label values as a JSON string array (kept for compatibility).
        std::string GetCoordinateAxisJSON(std::size_t index) const
        {
            if(index < RANK)
            {
                JlAxisJSONFiller filler;
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisJSONFiller >(*fTable, index, filler);
                return filler.GetJSON();
            }
            msg_error("julia_bindings", "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            return "[]";
        }

        //! Return a string naming the axis value type: "string", "double", "float", "integer", or "unknown".
        std::string GetAxisValueTypeName(std::size_t index) const
        {
            if(index < RANK)
            {
                JlAxisTypeNameFiller filler;
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisTypeNameFiller >(*fTable, index, filler);
                return filler.GetTypeName();
            }
            msg_error("julia_bindings", "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            return "unknown";
        }

        //! Return the N-th axis labels as a vector of doubles (for numeric axes).
        //! Returns an empty vector for non-numeric axes.
        std::vector< double > GetCoordinateAxisAsDoubles(std::size_t index) const
        {
            if(index < RANK)
            {
                JlAxisDoublesFiller filler;
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisDoublesFiller >(*fTable, index, filler);
                return filler.GetValues();
            }
            msg_error("julia_bindings", "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            return {};
        }

        //! Return the N-th axis labels as a vector of strings (works for any axis type).
        std::vector< std::string > GetCoordinateAxisAsStrings(std::size_t index) const
        {
            if(index < RANK)
            {
                JlAxisStringsFiller filler;
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisStringsFiller >(*fTable, index, filler);
                return filler.GetValues();
            }
            msg_error("julia_bindings", "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            return {};
        }

        //! Set a single coordinate label from a JSON-encoded value string.
        void SetCoordinateLabelFromJSON(std::size_t axis_index, std::size_t label_index, std::string json_value)
        {
            if(axis_index < RANK)
            {
                JlAxisLabelModifier modifier(label_index, json_value);
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisLabelModifier >(*fTable, axis_index, modifier);
            }
            else
            {
                msg_error("julia_bindings",
                          "axis index: " << axis_index << " exceeds table rank of: " << RANK << eom);
            }
        }

        //! Return table metadata as a JSON string.
        std::string GetMetaDataJSON() const
        {
            mho_json md = fTable->GetMetaDataAsJSON();
            return md.dump();
        }

        //! Replace the table metadata from a JSON string.
        void SetMetaDataJSON(const std::string& json_str)
        {
            mho_json md = mho_json::parse(json_str);
            fTable->SetMetaDataAsJSON(md);
        }

        //! Return the metadata of axis 'index' as a JSON string.
        std::string GetCoordinateAxisMetaDataJSON(std::size_t index) const
        {
            if(index < RANK)
            {
                JlAxisMetaDataFiller filler;
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisMetaDataFiller >(*fTable, index, filler);
                return filler.GetJSON();
            }
            msg_error("julia_bindings", "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            return "{}";
        }

        //! Replace the metadata of axis 'index' from a JSON string.
        void SetCoordinateAxisMetaDataJSON(std::size_t index, const std::string& json_str)
        {
            if(index < RANK)
            {
                JlAxisMetaDataSetter setter(json_str);
                apply_at< typename XTableType::axis_pack_tuple_type, JlAxisMetaDataSetter >(*fTable, index, setter);
            }
            else
            {
                msg_error("julia_bindings",
                          "axis index: " << index << " exceeds table rank of: " << RANK << eom);
            }
        }

    private:
        // Reverse dimensions at compile time so Julia column-major <-> C++ row-major.
        template< std::size_t... I >
        jlcxx::ArrayRef< value_type, RANK > GetNDArray_impl(std::index_sequence< I... >)
        {
            // I = 0,1,...,RANK-1  ->  RANK-1-I = RANK-1,...,1,0  (reversed)
            return jlcxx::ArrayRef< value_type, RANK >(
                fTable->GetData(),
                static_cast< std::size_t >(fTable->GetDimension(RANK - 1 - I))...);
        }

        // Functor: return the axis value type name as a string
        class JlAxisTypeNameFiller
        {
            public:
                JlAxisTypeNameFiller(){};
                ~JlAxisTypeNameFiller(){};

                template< typename XAxisType > void operator()(const XAxisType& /*axis*/)
                {
                    using T = typename XAxisType::value_type;
                    if constexpr(std::is_same_v< T, std::string >)
                        fTypeName = "string";
                    else if constexpr(std::is_same_v< T, double >)
                        fTypeName = "double";
                    else if constexpr(std::is_same_v< T, float >)
                        fTypeName = "float";
                    else if constexpr(std::is_integral_v< T >)
                        fTypeName = "integer";
                    else
                        fTypeName = "unknown";
                }

                std::string GetTypeName() const { return fTypeName; }

            private:
                std::string fTypeName = "unknown";
        };

        // Functor: collect axis labels into a vector<double> (numeric axes only)
        class JlAxisDoublesFiller
        {
            public:
                JlAxisDoublesFiller(){};
                ~JlAxisDoublesFiller(){};

                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    using T = typename XAxisType::value_type;
                    if constexpr(std::is_arithmetic_v< T >)
                    {
                        fValues.reserve(axis.GetSize());
                        for(std::size_t i = 0; i < axis.GetSize(); i++)
                            fValues.push_back(static_cast< double >(axis[i]));
                    }
                    // non-numeric: leave fValues empty
                }

                std::vector< double > GetValues() const { return fValues; }

            private:
                std::vector< double > fValues;
        };

        // Functor: collect axis labels into a vector<string> (works for any axis type)
        class JlAxisStringsFiller
        {
            public:
                JlAxisStringsFiller(){};
                ~JlAxisStringsFiller(){};

                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    using T = typename XAxisType::value_type;
                    fValues.reserve(axis.GetSize());
                    if constexpr(std::is_same_v< T, std::string >)
                    {
                        for(std::size_t i = 0; i < axis.GetSize(); i++)
                            fValues.push_back(axis[i]);
                    }
                    else
                    {
                        for(std::size_t i = 0; i < axis.GetSize(); i++)
                            fValues.push_back(std::to_string(axis[i]));
                    }
                }

                std::vector< std::string > GetValues() const { return fValues; }

            private:
                std::vector< std::string > fValues;
        };

        // Functor: serialize an axis's labels to a JSON array string
        class JlAxisJSONFiller
        {
            public:
                JlAxisJSONFiller(){};
                ~JlAxisJSONFiller(){};

                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    fJSON = mho_json::array();
                    for(size_t i = 0; i < axis.GetSize(); i++) { fJSON.push_back(axis[i]); }
                }

                std::string GetJSON() const { return fJSON.dump(); }

            private:
                mho_json fJSON;
        };

        // Functor: serialize axis metadata to a JSON object string
        class JlAxisMetaDataFiller
        {
            public:
                JlAxisMetaDataFiller(){};
                ~JlAxisMetaDataFiller(){};

                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    fJSON = axis.GetMetaDataAsJSON();
                }

                std::string GetJSON() const { return fJSON.dump(); }

            private:
                mho_json fJSON;
        };

        // Functor: replace axis metadata from a JSON object string
        class JlAxisMetaDataSetter
        {
            public:
                JlAxisMetaDataSetter(const std::string& json_str): fJSON(mho_json::parse(json_str)){};
                ~JlAxisMetaDataSetter(){};

                template< typename XAxisType > void operator()(XAxisType& axis)
                {
                    axis.SetMetaDataAsJSON(fJSON);
                }

            private:
                mho_json fJSON;
        };

        // Functor: set a single label from a JSON-encoded value string
        class JlAxisLabelModifier
        {
            public:
                JlAxisLabelModifier(std::size_t index, const std::string& json_value)
                    : fIndex(index), fValue(mho_json::parse(json_value)){};
                ~JlAxisLabelModifier(){};

                template< typename XAxisType > void operator()(XAxisType& axis)
                {
                    if(fIndex < axis.GetSize())
                    {
                        axis(fIndex) = fValue.get< typename XAxisType::value_type >();
                    }
                    else
                    {
                        msg_error("julia_bindings", "axis coordinate index out of bounds: "
                                                        << fIndex << " >= " << axis.GetSize() << eom);
                    }
                }

            private:
                std::size_t fIndex;
                mho_json fValue;
        };

    private:
        MHO_ExtensibleElement* fElement;
        XTableType* fTable;
};

} // namespace hops

//==============================================================================
// Registration helper - call once per concrete XTableType in JLCXX_MODULE.
//==============================================================================
namespace hops
{

template< typename XTableType >
void DeclareJlTableContainer(jlcxx::Module& mod, const std::string& jl_type_name)
{
    mod.add_type< MHO_JlTableContainer< XTableType > >(jl_type_name)
        .method("get_rank",          &MHO_JlTableContainer< XTableType >::GetRank)
        .method("get_classname",     &MHO_JlTableContainer< XTableType >::GetClassName)
        .method("get_dimension",     &MHO_JlTableContainer< XTableType >::GetDimension)
        // get_array returns an N-dimensional Julia array (zero-copy, dims reversed for column-major).
        .method("get_array",           &MHO_JlTableContainer< XTableType >::GetNDArray)
        // get_dimensions returns C++ logical dims [dim0, ..., dimN-1] for reference.
        .method("get_dimensions",      &MHO_JlTableContainer< XTableType >::GetDimensions)
        .method("get_metadata",        &MHO_JlTableContainer< XTableType >::GetMetaDataJSON)
        .method("set_metadata",        &MHO_JlTableContainer< XTableType >::SetMetaDataJSON)
        // get_axis returns axis labels as a JSON string (kept for compatibility).
        .method("get_axis",            &MHO_JlTableContainer< XTableType >::GetCoordinateAxisJSON)
        // Typed axis accessors - prefer these over get_axis to avoid JSON round-trips.
        //   get_axis_value_type  -> "string" | "double" | "float" | "integer" | "unknown"
        //   get_axis_doubles     -> StdVector{Float64}   (use collect() for a plain Julia Vector)
        //   get_axis_strings     -> StdVector{StdString} (use String.() for a plain Julia Vector)
        .method("get_axis_value_type", &MHO_JlTableContainer< XTableType >::GetAxisValueTypeName)
        .method("get_axis_doubles",    &MHO_JlTableContainer< XTableType >::GetCoordinateAxisAsDoubles)
        .method("get_axis_strings",    &MHO_JlTableContainer< XTableType >::GetCoordinateAxisAsStrings)
        .method("get_axis_metadata",   &MHO_JlTableContainer< XTableType >::GetCoordinateAxisMetaDataJSON)
        .method("set_axis_metadata",   &MHO_JlTableContainer< XTableType >::SetCoordinateAxisMetaDataJSON)
        .method("set_axis_label",      &MHO_JlTableContainer< XTableType >::SetCoordinateLabelFromJSON);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlTableContainer */
