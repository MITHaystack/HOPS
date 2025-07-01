#ifndef MHO_MultidimensionalFastFourierTransformInterface_HH__
#define MHO_MultidimensionalFastFourierTransformInterface_HH__

#include <cstring>

#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_MultidimensionalFastFourierTransformInterface.hh
 *@class MHO_MultidimensionalFastFourierTransformInterface
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Jul 31 17:16:37 2023 -0400
 *@brief
 */

/**
 * @brief Class MHO_MultidimensionalFastFourierTransformInterface
 */
template< typename XArgType > class MHO_MultidimensionalFastFourierTransformInterface
{
    public:
        static_assert(is_complex< typename XArgType::value_type >::value,
                      "Array element type must be a complex floating point type.");
        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_MultidimensionalFastFourierTransformInterface()
        {
            for(size_t i = 0; i < XArgType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
            fTransformAxisLabels = true;
        };

        virtual ~MHO_MultidimensionalFastFourierTransformInterface(){};

        /**
         * @brief Setter for forward
         * @note This is a virtual function.
         */
        virtual void SetForward() { fForward = true; }

        /**
         * @brief Setter for backward
         * @note This is a virtual function.
         */
        virtual void SetBackward() { fForward = false; };

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        /**
         * @brief Selects all axes for transformation.
         * @note This is a virtual function.
         */
        virtual void SelectAllAxes()
        {
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fAxesToXForm[i] = true;
                fInitialized = false;
            }
        }

        /**
         * @brief Deselects all axes by setting fAxesToXForm to false and resetting initialization.
         * @note This is a virtual function.
         */
        virtual void DeselectAllAxes()
        {
            for(std::size_t i = 0; i < XArgType::rank::value; i++)
            {
                fAxesToXForm[i] = false;
                fInitialized = false;
            }
        }

        /**
         * @brief Selects an axis for transformation if within array rank.
         * 
         * @param axis_index Index of the axis to select.
         * @note This is a virtual function.
         */
        virtual void SelectAxis(std::size_t axis_index)
        {
            fInitialized = false;
            if(axis_index < XArgType::rank::value)
            {
                fAxesToXForm[axis_index] = true;
            }
            else
            {
                msg_error("operators", "Cannot transform axis with index: " << axis_index << "for array with rank: "
                                                                            << XArgType::rank::value << eom);
            }
        }

        /**
         * @brief Enables transformation for axis labels.
         */
        void EnableAxisLabelTransformation() { fTransformAxisLabels = true; }

        /**
         * @brief Disables axis label transformation by setting fTransformAxisLabels to false.
         */
        void DisableAxisLabelTransformation() { fTransformAxisLabels = false; }

    protected:
        //default...does nothing
        /**
         * @brief Transforms axis of input data if transformation is enabled and axis was transformed.
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param !in Parameter description
         * @param !axis_index Parameter description
         */
        template< typename XCheckType = XArgType >
        typename std::enable_if< !std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableTransformAxis(XArgType* /*!in*/, std::size_t /*!axis_index*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        /**
         * @brief Transforms axis of input table if specified dimension was transformed.
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param in Input table of type XArgType
         * @param axis_index Index of axis to transform
         */
        template< typename XCheckType = XArgType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        IfTableTransformAxis(XArgType* in, std::size_t axis_index)
        {
            if(fTransformAxisLabels)
            {
                if(fAxesToXForm[axis_index]) //only xform axis if this dimension was transformed
                {
                    TransformAxis axis_xformer;
                    apply_at< typename XArgType::axis_pack_tuple_type, TransformAxis >(*in, axis_index, axis_xformer);
                }
            }
        }

        /**
         * @brief Class TransformAxis
         */
        class TransformAxis
        {
            public:
                TransformAxis(){};
                ~TransformAxis(){};

                //generic axis, do nothing
                template< typename XAxisType > void operator()(XAxisType& /*!axis1*/){};

                //overload for doubles
                void operator()(MHO_Axis< double >& axis1)
                {
                    //this is under the expectation that all axis labels are equi-spaced
                    //this should be a safe assumption since we are doing DFT anyway
                    //one issue here is that we are not taking into account units (e.g. nanosec or MHz)
                    std::size_t N = axis1.GetSize();
                    double length = N;
                    if(N > 1)
                    {
                        double delta = axis1(1) - axis1(0);
                        double spacing = (1.0 / delta) * (1.0 / length);
                        double start = 0; //-1*length/2;
                        for(std::size_t i = 0; i < N; i++)
                        {
                            double x = i;
                            if(i < N / 2)
                            {
                                start = 0;
                                double value = (x + start) * spacing;
                                axis1(i) = value;
                            }
                            else
                            {
                                start = -1 * length;
                                double value = (x + start) * spacing;
                                axis1(i) = value;
                            }
                        }
                    }
                }

                //overload for floats
                void operator()(MHO_Axis< float >& axis1)
                {
                    //this is under the expectation that all axis labels are equi-spaced
                    //this should be a safe assumption since we are doing DFT anyway
                    //one issue here is that we are not taking into account units (e.g. nanosec or MHz)
                    std::size_t N = axis1.GetSize();
                    float length = N;
                    if(N > 1)
                    {
                        float delta = axis1(1) - axis1(0);
                        float spacing = (1.0 / delta) * (1.0 / length);
                        float start = 0; //-1*length/2;
                        for(std::size_t i = 0; i < N; i++)
                        {
                            float x = i;
                            if(i < N / 2)
                            {
                                start = 0;
                                float value = (x + start) * spacing;
                                axis1(i) = value;
                            }
                            else
                            {
                                start = -1 * length;
                                float value = (x + start) * spacing;
                                axis1(i) = value;
                            }
                        }
                    }
                }
        };

        //data
        bool fIsValid;
        bool fForward;
        bool fInitialized;
        bool fTransformAxisLabels;

        size_t fDimensionSize[XArgType::rank::value];
        bool fAxesToXForm[XArgType::rank::value];
};

} // namespace hops

#endif /*! MHO_MultidimensionalFastFourierTransformInterface_H__ */
