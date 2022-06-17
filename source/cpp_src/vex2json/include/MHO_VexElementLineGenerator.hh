#ifndef MHO_VexElementLineGenerator_HH__
#define MHO_VexElementLineGenerator_HH__

/*
*@file: MHO_VexElementLineGenerator.hh
*@class: MHO_VexElementLineGenerator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops 
{

class MHO_VexElementLineGenerator
{
    public:
        MHO_VexElementLineGenerator();
        virtual ~MHO_VexElementLineGenerator();

        std::string ConstructElementLine(mho_json& element, mho_json& format, std::vector< std::string >& lines);

    private:

        vex_element_type DetermineType(std::string etype);

        enum vex_element_type
        {
            vex_int_type,
            vex_list_int_type,
            vex_real_type,
            vex_string_type,
            vex_list_string_type,
            vex_epoch_type,
            vex_radec_type,
            vex_list_real_type,
            vex_compound_type,
            vex_list_compound_type,
            vex_link_type,
            vex_keyword_type,
            vex_reference_type,
            vex_unknown_type
            //TODO FIXME -- add type for RA and Dec, to handle special treatment of "
        };
};

}

#endif /* end of include guard: MHO_VexElementLineGenerator */