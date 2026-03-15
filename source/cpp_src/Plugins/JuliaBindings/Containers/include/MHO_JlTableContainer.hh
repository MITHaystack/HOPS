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
 * Exposes the N-dimensional array directly as a Julia AbstractArray via
 * CxxWrap's ArrayTraits specialization. This gives zero-copy, correct
 * multidimensional indexing (row-major strides) with no reshape required.
 * JSON metadata is returned as std::string; Julia parses with JSON3.jl.
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
         * Return an N-dimensional native Julia Array wrapping the C++ memory (zero-copy).
         *
         * Returns jl_value_t* so CxxWrap passes it through directly to Julia as a
         * concrete Base.Array{T,N} — not a CxxWrap AbstractArray wrapper.  This
         * means all Julia array operations (slicing, broadcasting, etc.) work without
         * any data copy.
         *
         * Dimensions are passed in REVERSED order so that Julia's column-major first
         * index aligns with the C++ row-major last (fastest-varying) dimension:
         *
         *   Julia vis[i_freq, i_time, i_channel, i_pol]  (1-based)
         *   <->  C++  arr[pol][channel][time][freq]         (0-based)
         *
         * own_buffer=0: Julia will NOT free the underlying C++ data pointer.
         * Do NOT resize!, push!, or append! to the returned array.
         */
        /*!
         * Return the array data as a flat 1-D Julia Array wrapping C++ memory (zero-copy).
         *
         * Using jl_ptr_to_array_1d avoids the multi-dimensional dims-tuple
         * construction required by jl_ptr_to_array, whose behaviour varies
         * across Julia minor versions.  Julia callers should reshape the result:
         *
         *   raw  = get_array(obj)                              # 1-D view
         *   arr  = reshape(raw, reverse(get_dimensions(obj))...) # N-D view
         *
         * get_dimensions() returns C++ logical order [dim0 (slowest) … dimN-1 (fastest)].
         * Reversing gives Julia column-major order so that
         *   arr[i_freq, i_time, i_channel, i_pol]  ↔  C++ arr[pol][ch][time][freq].
         *
         * own_buffer=0: Julia will NOT free the C++ data pointer.
         * Do NOT resize!, push!, or append! to the returned array.
         */
        jl_value_t* GetNDArray()
        {
            std::size_t nel = 1;
            for(std::size_t i = 0; i < RANK; ++i) nel *= fTable->GetDimension(i);

            jl_value_t* arr_type = jl_apply_array_type(julia_value_type(), 1);
            return reinterpret_cast< jl_value_t* >(
                jl_ptr_to_array_1d(arr_type, fTable->GetData(), nel, 0));
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

        //! Return the N-th axis label values as a JSON string array.
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
        // Map value_type to the corresponding Julia primitive type constant,
        // bypassing CxxWrap's registry (which doesn't cover std::complex<>).
        // jl_complex_type is a UnionAll; jl_apply_type1 instantiates it.
        static jl_value_t* julia_value_type()
        {
            if constexpr(std::is_same_v< value_type, double >)
                return reinterpret_cast< jl_value_t* >(jl_float64_type);
            else if constexpr(std::is_same_v< value_type, float >)
                return reinterpret_cast< jl_value_t* >(jl_float32_type);
            else if constexpr(std::is_same_v< value_type, std::complex< double > >)
                return jl_apply_type1(jl_get_global(jl_base_module, jl_symbol("Complex")),
                                      reinterpret_cast< jl_value_t* >(jl_float64_type));
            else if constexpr(std::is_same_v< value_type, std::complex< float > >)
                return jl_apply_type1(jl_get_global(jl_base_module, jl_symbol("Complex")),
                                      reinterpret_cast< jl_value_t* >(jl_float32_type));
            else
            {
                static_assert(sizeof(value_type) == 0,
                              "MHO_JlTableContainer: unsupported value_type for Julia array binding");
                return nullptr;
            }
        }

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
        .method("get_array",         &MHO_JlTableContainer< XTableType >::GetNDArray)
        // get_dimensions returns C++ logical dims [dim0, ..., dimN-1] for reference.
        .method("get_dimensions",    &MHO_JlTableContainer< XTableType >::GetDimensions)
        .method("get_metadata",      &MHO_JlTableContainer< XTableType >::GetMetaDataJSON)
        .method("set_metadata",      &MHO_JlTableContainer< XTableType >::SetMetaDataJSON)
        .method("get_axis",          &MHO_JlTableContainer< XTableType >::GetCoordinateAxisJSON)
        .method("get_axis_metadata", &MHO_JlTableContainer< XTableType >::GetCoordinateAxisMetaDataJSON)
        .method("set_axis_metadata", &MHO_JlTableContainer< XTableType >::SetCoordinateAxisMetaDataJSON)
        .method("set_axis_label",    &MHO_JlTableContainer< XTableType >::SetCoordinateLabelFromJSON);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlTableContainer */
