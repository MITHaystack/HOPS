#include "HkMessenger.hh"


using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    HkMessenger::GetInstance().AddKey("math");
    HkMessenger::GetInstance().AddKey("core");
    HkMessenger::GetInstance().SetMessageLevel(eDebug);

    double pi_value = 3.14159265;
    int n = 5;
    msg_info("math", "I have a math message for you: pi = " << pi_value << eom );
    msg_status("core", "I have core message for you: n = " << n << eom);

    return 0;
}
