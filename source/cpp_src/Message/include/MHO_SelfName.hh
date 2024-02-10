#ifndef MHO_SelfName_HH__
#define MHO_SelfName_HH__

#include <atomic>
#include <thread>
#include <cstdint>

namespace hops
{
    namespace selfname
    {
        //needed for stripping the path prefix from __FILE__ macro contents
        //see https://stackoverflow.com/questions/31050113
        constexpr const char* str_end(const char *str){ return *str ? str_end(str + 1) : str; }
        constexpr bool str_slash(const char *str){ return *str == '/' ? true : (*str ? str_slash(str + 1) : false); }
        constexpr const char* r_slash(const char* str){ return *str == '/' ? (str + 1) : r_slash(str - 1); }
        constexpr const char* file_basename(const char* str) { return str_slash(str) ? r_slash(str_end(str)) : str; }

        // static std::atomic<uint64_t> thread_counter;
        // uint64_t thread_id() 
        // {
        //     thread_local uint64_t tid = ++local_thread_counter;
        //     return tid;
        // }

    }//end selfname namespace

}//end hops namespace

#endif /* end of include guard: MHO_SelfName_HH__ */
