#include "MHO_PlotDataExtractor.hh"
#include <iomanip>
#include <sstream>


namespace hops 
{


std::vector< double > 
MHO_PlotDataExtractor::extract_vector(const mho_json& plot_dict, const std::string& key)
{
    try
    {
        if(plot_dict.contains(key))
        {
            if(plot_dict[key].is_array())
            {
                return plot_dict[key].get< std::vector< double > >();
            }
        }
    }
    catch(const std::exception& e)
    {
        msg_warn("plot_extractor", "failed to extract vector for key '" << key << "': " << e.what() << eom);
    }
    return std::vector< double >();
}

std::vector< double > 
MHO_PlotDataExtractor::create_index_vector(size_t size)
{
    std::vector< double > indices(size);
    std::iota(indices.begin(), indices.end(), 0.0);
    return indices;
}

std::string 
MHO_PlotDataExtractor::format_scientific(double value, int precision)
{
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(precision) << value;
    return oss.str();
}

std::string 
MHO_PlotDataExtractor::format_standard(double value, int minDigits, int roundPlaces)
{
    double scale = std::pow(10.0, roundPlaces);
    double rounded = std::round(value * scale) / scale;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(std::max(minDigits, roundPlaces));
    oss << rounded;
    return oss.str();
}

int 
MHO_PlotDataExtractor::extract_int(const mho_json& plot_dict, const std::string& key, int default_value)
{
    try
    {
        if(plot_dict.contains(key))
        {
            if(plot_dict[key].is_number_integer())
            {
                return plot_dict[key].get< int >();
            }
            else if(plot_dict[key].is_string())
            {
                return std::stoi(plot_dict[key].get< std::string >());
            }
        }
    }
    catch(const std::exception& e)
    {
        msg_warn("plot_extractor", "failed to extract int for key '" << key << "': " << e.what() << eom);
    }
    return default_value;
}

std::string 
MHO_PlotDataExtractor::extract_string(const mho_json& plot_dict, const std::string& key,
                                                  const std::string& default_value)
{
    try
    {
        if(plot_dict.contains(key))
        {
            if(plot_dict[key].is_string())
            {
                return plot_dict[key].get< std::string >();
            }
        }
    }
    catch(const std::exception& e)
    {
        msg_warn("plot_extractor", "failed to extract string for key '" << key << "': " << e.what() << eom);
    }
    return default_value;
}

double 
MHO_PlotDataExtractor::extract_double(const mho_json& plot_dict, const std::string& key, double default_value)
{
    try
    {
        if(plot_dict.contains(key))
        {
            if(plot_dict[key].is_number())
            {
                return plot_dict[key].get< double >();
            }
            else if(plot_dict[key].is_string())
            {
                return std::stod(plot_dict[key].get< std::string >());
            }
        }
    }
    catch(const std::exception& e)
    {
        msg_warn("plot_extractor", "failed to extract double for key '" << key << "': " << e.what() << eom);
    }
    return default_value;
}


}
