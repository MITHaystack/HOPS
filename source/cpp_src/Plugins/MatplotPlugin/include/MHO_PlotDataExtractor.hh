#ifndef MHO_PlotDataExtractor_HH__
#define MHO_PlotDataExtractor_HH__

#include "MHO_JSONHeaderWrapper.hh"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>


namespace hops 
{


/**
 * @brief Utility class for extracting and processing plot data from JSON
 */
class MHO_PlotDataExtractor
{
    public:
        /**
         * @brief Extracts a vector of doubles from JSON plot dictionary
         * @param plot_dict JSON object containing plot data
         * @param key Key to extract from the JSON object
         * @return Vector of doubles, empty if key not found
         */
        static std::vector< double > extract_vector(const mho_json& plot_dict, const std::string& key);

        /**
         * @brief Creates an index vector (0, 1, 2, ..., size-1)
         * @param size Size of the index vector to create
         * @return Vector containing sequential indices
         */
        static std::vector< double > create_index_vector(size_t size);

        /**
         * @brief Formats a double as scientific notation string
         * @param value Value to format
         * @param precision Number of decimal places
         * @return Formatted string
         */
        static std::string format_scientific(double value, int precision = 3);

        static std::string format_standard(double value, int minDigits, int roundPlaces);

        /**
         * @brief Extracts integer value from JSON with fallback
         * @param plot_dict JSON object containing plot data
         * @param key Key to extract
         * @param default_value Default value if key not found
         * @return Integer value
         */
        static int extract_int(const mho_json& plot_dict, const std::string& key, int default_value = 0);

        /**
         * @brief Extracts string value from JSON with fallback
         * @param plot_dict JSON object containing plot data
         * @param key Key to extract
         * @param default_value Default value if key not found
         * @return String value
         */
        static std::string extract_string(const mho_json& plot_dict, const std::string& key,
                                          const std::string& default_value = "");

        /**
         * @brief Extracts double value from JSON with fallback
         * @param plot_dict JSON object containing plot data
         * @param key Key to extract
         * @param default_value Default value if key not found
         * @return Double value
         */
        static double extract_double(const mho_json& plot_dict, const std::string& key, double default_value = 0.0);
};


}


#endif /* end of include guard: MHO_PlotDataExtractor_HH__ */


