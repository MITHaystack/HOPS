#ifndef MHO_DiFXPCalProcessor_HH__
#define MHO_DiFXPCalProcessor_HH__

#include <complex>
#include <string>
#include <utility>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_DiFXTimeUtilities.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXPCalProcessor.hh
 *@class  MHO_DiFXPCalProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Mar 7 12:21:49 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_DiFXPCalProcessor
 */
class MHO_DiFXPCalProcessor
{
    public:
        MHO_DiFXPCalProcessor();
        virtual ~MHO_DiFXPCalProcessor();

        /**
         * @brief Setter for  (PCAL) filename
         *
         * @param filename Input filename to set and validate
         */
        void SetFilename(std::string filename);

        /**
         * @brief Setter for pcal accumulation period
         *
         * @param ap_sec pcal accumulation period duration in seconds
         */
        void SetAccumulationPeriod(double ap_sec) { fAPLength = ap_sec; }

        /**
         * @brief Setter for the scan start time, if not set, the first pcal record is assumed to be AP 0.
         *
         * @param mjd scan start time (MJD)
         */
        void SetScanStartMJD(double mjd)
        {
            fScanStartMJD = mjd;
            fHaveScanStart = true;
        }

        /**
         * @brief Getter for station code
         *
         * @return Current station code as a string
         */
        std::string GetStationCode() const { return fStationCode; }

        /**
         * @brief Reads and processes a PCal file
         */
        void ReadPCalFile();

        /**
         * @brief Organizes p-cal data by merging tone/phasor data from the same time period and stores them in a table container.
         */
        void Organize();

        /**
         * @brief Getter for (organized) pcal data
         *
         * @return Pointer to multitone_pcal_type
         */
        multitone_pcal_type* GetPCalData() { return &fPCal; }

    private:
        /**
         * @brief Checks if the first character of fLine is '#', indicating a comment.
         *
         * @return True if fLine starts with '#', false otherwise.
         */
        bool IsComment();

        /**
         * @brief Clears and populates fTokens by tokenizing fLine using fTokenizer.
         */
        void TokenizeLine();

        /**
         * @brief Processes token data and pushes parsed p-cal periods to fPCalData.
         */
        void ProcessTokens();

        /**
         * @brief Assigns the scan-relative AP index of each pcal record
         *
         * @param first_ap index of the first AP with pcal data
         * @param last_ap index of the last AP with pcal data
         * @return True if any pcal records remain
         */
        bool AssignAPIndices(int& first_ap, int& last_ap);

        /**
         * @brief Collects the sorted/unique set of tone frequencies present
         */
        void BuildToneAxis();

        /**
         * @brief Returns the index of a tone frequency on the tone axis, or fToneFreqs.size() if not present
         */
        std::size_t FindToneIndex(double tone_freq) const;

        /**
         * @brief Fills the dense (pol, ap, tone) table from the pcal records
         *
         * @param first_ap Scan-relative index of the first AP in the table
         */
        void FillPCalTable(int first_ap);

        /**
         * @brief Fills in table entries missing from the pcal records by linear interpolation
         */
        void FillMissingPhasors();

        /**
         * @brief Index into fPresent for a (pol, ap, tone) table entry
         */
        std::size_t ValidIndex(std::size_t pol, std::size_t ap, std::size_t tone) const
        {
            return (pol * fPCal.GetDimension(MTPCAL_TIME_AXIS) + ap) * fToneFreqs.size() + tone;
        }

        std::string fFilename;
        std::string fLine;
        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fTokens;
        double fAPLength;
        double fScanStartMJD;
        bool fHaveScanStart;
        double fGridStartMJD; //start of AP 0 of the pcal AP grid
        bool fValid;

        std::string fType;
        std::string fMJD_day;
        std::string fMJD_frac;
        std::string fStationCode;

        //TODO this constant isn't used directly for conversion
        //(just a tolerance check), but we should put this somewhere sensible
        double fSecondsPerDay;
        double fTolerance;

        //single pcal tone
        struct pcal_phasor
        {
                double tone_freq;
                pcal_phasor_type phasor;
        };

        //container for pcal data from a single AP
        struct pcal_period
        {
                std::string station;
                double mjd;
                double mjd_period;
                int ap;
                std::map< std::string, std::vector< pcal_phasor > > pc_phasors;
        };

        //pcal data in various forms on the way to getting it organized
        std::vector< pcal_period > fPCalData;
        std::set< std::string > fPolSet;
        std::vector< double > fToneFreqs;
        std::vector< char > fPresent; //flags (pol, ap, tone) table entries present in the pcal records

        //fully organized pcal data
        multitone_pcal_type fPCal;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXPCalProcessor */
