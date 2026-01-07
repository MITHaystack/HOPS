#ifndef MHO_MultitonePhaseCorrectionBuilderBuilder_HH__
#define MHO_MultitonePhaseCorrectionBuilderBuilder_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_MultitonePhaseCorrectionBuilder.hh
 *@class MHO_MultitonePhaseCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Dec 7 13:29:58 2023 -0500
 *@brief builds a multitone phase-cal correction operator
 */

/**
 * @brief Class MHO_MultitonePhaseCorrectionBuilder
 */
class MHO_MultitonePhaseCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata)
        {
            fRefOpName = "ref_multitone_pcal";
            fRemOpName = "rem_multitone_pcal";
        };

        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore)
        {
            fRefOpName = "ref_multitone_pcal";
            fRemOpName = "rem_multitone_pcal";
        };

        virtual ~MHO_MultitonePhaseCorrectionBuilder(){};

        /**
         * @brief Builds and initializes a multitone phase-cal correction operator and adds to the toolbox
         *
         * @return Return value (bool)
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Extracts station 2-char code from parameter store based on input operation name.
         *
         * @param op_name Input operation name to determine which station's code to extract.
         * @return Station code as a string.
         */
        std::string ExtractStationCode(std::string op_name); //op_name indicates reference or remote station
        /**
         * @brief Extracts and returns PC period from either a generic path or station-specific path using mk4id.
         *
         * @param mk4id Station identifier used to construct the station-specific path
         * @return PC period value retrieved from the parameter store
         */
        int ExtractPCPeriod(std::string mk4id); //pulls the appropriate pc_period out of parameter store
        /**
         * @brief Attaches sampler delays to multitone PCAL data using mk4id and parameter store.
         *
         * @param pcal_data Input multitone PCAL data structure
         * @param mk4id Station ID for retrieving station-specific parameters
         */
        void AttachSamplerDelays(multitone_pcal_type* pcal_data, std::string mk4id); //attaches sampler delays to pcal data
        /**
         * @brief Attaches PC tone mask data to multitone pcal data using mk4id and parameter store.
         *
         * @param pcal_data Pointer to multitone_pcal_type structure for storing PC tone mask data
         * @param mk4id String containing the mk4id of the station
         */
        void AttachPCToneMask(multitone_pcal_type* pcal_data,
                              std::string mk4id); //attaches pc_tonemask infor to pcal data (if present)
        /**
         * @brief Getter for sampler delay key (i.e. control statement)
         *
         * @param pol Input polarization string ('X', 'Y', 'R', 'L', 'H', 'V')
         * @return Sampler delay key string
         */
        std::string GetSamplerDelayKey(std::string pol);

        std::string fRefOpName;
        std::string fRemOpName;
};

} // namespace hops

#endif /*! end of include guard: MHO_MultitonePhaseCorrectionBuilderBuilder_HH__ */
