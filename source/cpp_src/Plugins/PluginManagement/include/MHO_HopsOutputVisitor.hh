#ifndef MHO_HopsOutputVisitor_HH__
#define MHO_HopsOutputVisitor_HH__

#include "MHO_FringeFitter.hh"

namespace hops 
{

class MHO_HopsOutputVisitor: public MHO_FringeFitterVisitor
{
    public:
        MHO_HopsOutputVisitor();
        virtual ~MHO_HopsOutputVisitor();

        virtual void Visit(MHO_FringeFitter* fitter) override;


    protected:

        int WriteDataObjects(MHO_FringeData* data, std::string filename);


        std::string ConstructFrngFileName(const std::string directory, const std::string& baseline,
                                          const std::string& ref_station, const std::string& rem_station,
                                          const std::string& frequency_group, const std::string& polprod,
                                          const std::string& root_code, int seq_no);

        std::string ConstructTempFileName(const std::string directory, const std::string& baseline,
                                          const std::string& ref_station, const std::string& rem_station,
                                          const std::string& frequency_group, const std::string& polprod,
                                          const std::string& root_code, const std::string& temp_id);

};

}//end namespace

#endif /* end of include guard: MHO_HopsOutputVisitor_HH__ */
