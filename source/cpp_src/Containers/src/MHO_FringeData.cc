#include "MHO_FringeData.hh"

namespace hops
{

std::string MHO_FringeData::ConstructFrngFileName(const std::string directory, const std::string& baseline,
                                                  const std::string& ref_station, const std::string& rem_station,
                                                  const std::string& frequency_group, const std::string& polprod,
                                                  const std::string& root_code, int seq_no)
{
    std::stringstream ss;
    ss << directory << "/" << baseline << ".";
    ss << ref_station << "-" << rem_station << ".";
    ss << frequency_group << "." << polprod << ".";
    ss << root_code << "." << seq_no << ".frng";
    return ss.str();
}

std::string MHO_FringeData::ConstructTempFileName(const std::string directory, const std::string& baseline,
                                                  const std::string& ref_station, const std::string& rem_station,
                                                  const std::string& frequency_group, const std::string& polprod,
                                                  const std::string& root_code, const std::string& temp_id)
{
    std::stringstream ss;
    ss << directory << "/" << baseline << ".";
    ss << ref_station << "-" << rem_station << ".";
    ss << frequency_group << "." << polprod << ".";
    ss << root_code << ".frng." << temp_id;
    return ss.str();
}

} // namespace hops
