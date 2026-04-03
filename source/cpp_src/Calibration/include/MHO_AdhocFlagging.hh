#ifndef MHO_AdhocFlagging_HH__
#define MHO_AdhocFlagging_HH__

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_AdhocFlagging.hh
 *@class MHO_AdhocFlagging
 *@author J. Barrett - barrettj@mit.edu
 *@brief Port of the legacy fourfit adhoc_flag() capability into the HOPS4
 * calibration operator framework.
 *
 * Reads up to two ASCII flag files (one per station: reference and remote)
 * and zeroes out weights for (channel, time) combinations where the flag
 * data indicates data should be discarded.
 *
 * Flag file format:
 *   * comment line
 *   <t_fpday>  <hex_string>
 *   ...
 *
 * where t_fpday is fractional days since beginning of year and hex_string
 * encodes 64 flag bytes (one per frequency channel).  Each byte has the
 * bit assignments (msb to lsb):
 *
 *   USB-RL, LSB-RL, USB-LR, LSB-LR, USB-RR, LSB-RR, USB-LL, LSB-LL
 *
 * A bit set to 1 means the corresponding data should be RETAINED.
 * Short hex strings are padded by repeating the last nibble pair.
 *
 * USB data for channel ch is retained when (ref_byte[ch] & rem_byte[ch] & 0xAA) != 0.
 * LSB data for channel ch is retained when (ref_byte[ch] & rem_byte[ch] & 0x55) != 0.
 *
 * If a station file is empty/unset, that station's bytes are treated as 0xFF
 * (no flagging contribution from that station).  Time values outside the
 * file's range are also treated as 0xFF (no flagging).
 */

class MHO_AdhocFlagging: public MHO_UnaryOperator< weight_type >
{
    public:
        MHO_AdhocFlagging();
        virtual ~MHO_AdhocFlagging();

        /**
         * @brief Set the flag file for the reference station.
         * @param filename Path to the flag file; empty string disables ref flagging.
         */
        void SetRefFlagFile(const std::string& filename) { fFlagFile[0] = filename; }

        /**
         * @brief Set the flag file for the remote station.
         * @param filename Path to the flag file; empty string disables rem flagging.
         */
        void SetRemFlagFile(const std::string& filename) { fFlagFile[1] = filename; }

        /**
         * @brief Get the flag file path for the reference station.
         */
        const std::string& GetRefFlagFile() const { return fFlagFile[0]; }

        /**
         * @brief Get the flag file path for the remote station.
         */
        const std::string& GetRemFlagFile() const { return fFlagFile[1]; }

        /**
         * @brief Provide a parameter store so the operator can update
         *        /fringe/total_summed_weights after zeroing flagged APs.
         */
        void SetParameterStore(MHO_ParameterStore* pstore) { fParameterStore = pstore; }

    protected:
        virtual bool InitializeInPlace(weight_type* in) override;
        virtual bool InitializeOutOfPlace(const weight_type* in, weight_type* out) override;
        virtual bool ExecuteInPlace(weight_type* in) override;
        virtual bool ExecuteOutOfPlace(const weight_type* in, weight_type* out) override;

    private:
        // Maximum number of frequency channels encoded per flag file row.
        static constexpr std::size_t MAX_FLAG_FREQS = 64;

        // Bit masks for USB/LSB channel retention checks.
        //
        // NOTE: These follow the *legacy fourfit (adhoc_flag.c) convention*,
        // where the C variable names are inverted relative to the flag-file
        // bit-assignment documentation:
        //   doc bit layout (msb->lsb): USB-RL,LSB-RL,USB-LR,LSB-LR,USB-RR,LSB-RR,USB-LL,LSB-LL
        //   doc USB bits: 0xAA (bits 7,5,3,1),  doc LSB bits: 0x55 (bits 6,4,2,0)
        //
        // However in the legacy code:
        //   usb_flag = ref & rem & 0x55  (tests the doc-LSB bits to gate USB data)
        //   lsb_flag = ref & rem & 0xAA  (tests the doc-USB bits to gate LSB data)
        //
        // We preserve this inversion so that existing flag files, which were
        // written to match the legacy tool's behavior, continue to work correctly.
        //
        // Practical effect: a flag byte of 0x80 zeros USB channels and retains LSB,
        // even though bit 7 is labelled "USB-RL" in the documentation.
        static constexpr uint8_t USB_MASK = 0x55u; // legacy "usb_flag" mask
        static constexpr uint8_t LSB_MASK = 0xAAu; // legacy "lsb_flag" mask

        struct FlagTableRow
        {
                double time_fpday;
                std::array< uint8_t, MAX_FLAG_FREQS > bytes;
        };

        /**
         * @brief Decode a hex token string into MAX_FLAG_FREQS flag bytes.
         *
         * Iterates through the token one nibble at a time (upper then lower).
         * When the token is exhausted the last seen nibble pair is repeated
         * for all remaining bytes, matching the legacy populate_entry() logic.
         *
         * @param hex_token  Null-terminated hex string (e.g. "FF", "80FFAA").
         * @param bytes      Output array to fill.
         */
        static void DecodeHexToken(const char* hex_token, std::array< uint8_t, MAX_FLAG_FREQS >& bytes);

        /**
         * @brief Open and parse a flag file into fFlagTable[stn_idx].
         *
         * @param stn_idx  0 = reference station, 1 = remote station.
         * @return true on success (including empty filename, which is legal).
         */
        bool LoadFlagFile(std::size_t stn_idx);

        /**
         * @brief Return a pointer to the 64 flag bytes for station stn_idx
         *        at the largest table time <= ap_center_fpday (lower bound).
         *
         * Returns a pointer to an all-0xFF sentinel array if the table is
         * empty or if ap_center_fpday is outside the table's time range
         * (matching the legacy "no flagging" behaviour for out-of-range APs).
         *
         * @param stn_idx         0 = reference, 1 = remote.
         * @param ap_center_fpday AP centre time in fractional days since BOY.
         * @return Pointer to MAX_FLAG_FREQS bytes (valid until the next call).
         */
        const uint8_t* LookupFlagBytes(std::size_t stn_idx, double ap_center_fpday) const;

        // Optional parameter store for updating /fringe/total_summed_weights
        MHO_ParameterStore* fParameterStore;

        // Flag file paths: [0] = ref, [1] = rem
        std::string fFlagFile[2];

        // Parsed flag tables: [0] = ref, [1] = rem
        std::vector< FlagTableRow > fFlagTable[2];

        // Scan timing (filled during InitializeInPlace)
        double fScanStartFpDay; // scan start as fractional days since BOY
        int fScanYear;          // year of scan start
        double fAccPeriod;      // accumulation period in seconds

        // Metadata key names
        std::string fStartKey;    // "start"
        std::string fSidebandKey; // "net_sideband"
};

} // namespace hops

#endif /*! end of include guard: MHO_AdhocFlagging_HH__ */
