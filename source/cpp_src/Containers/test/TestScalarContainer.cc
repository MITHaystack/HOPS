#include <iostream>
#include <string>

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_FileKey.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_UUIDGenerator.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ScalarContainer< double >* A = new MHO_ScalarContainer< double >();
    double x = 1.23423423e30;

    A->SetValue(x);

    A->Insert("foo", 1.0);
    A->Insert("bar", "baz");

    std::cout << "stored valued =  " << A->GetValue() << std::endl;

    std::string filename = "./test.bin";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);

    if(status)
    {
        std::string shortname = "junk";
        inter.Write(*A, shortname);
        inter.Close();
    }
    else
    {
        std::cout << "error opening file" << std::endl;
    }

    std::cout << " class name: " << MHO_ClassIdentity::ClassName(*A) << std::endl;
    std::cout << " class version: " << A->GetVersion() << std::endl;

    inter.Close();

    delete A;

    MHO_ScalarContainer< double >* B = new MHO_ScalarContainer< double >();

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*B, key);
        //std::cout<<"B object label = "<<blabel<<std::endl;
        std::cout << "B object value = " << B->GetValue() << std::endl;
    }
    else
    {
        std::cout << " error opening file to read" << std::endl;
    }

    //see if we can get the tag values:
    double t1;
    std::string t2;
    bool ok = B->Retrieve("foo", t1);
    ok = B->Retrieve("bar", t2);

    std::cout << "foo = " << t1 << ", bar = " << t2 << std::endl;

    inter.Close();

    delete B;

    return 0;
}
