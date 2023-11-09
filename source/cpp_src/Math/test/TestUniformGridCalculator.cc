#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <getopt.h>

#include "MHO_UniformGridPointsCalculator.hh"

using namespace hops;

int main(int argc, char** argv)
{

    std::vector<double> pts1 = {
    //A:
    2832.4, 2864.4, 2896.4, 3024.4, 3088.4, 3152.4, 3248.4, 3280.4,
   //B:
    6616.4, 6648.4, 6680.4, 6808.4, 6904.4, 6968.4, 7032.4, 7064.4,
    //C:
    8529.4, 8561.4, 8593.4, 8721.4, 8785.4, 8849.4, 8881.4,  8913.4,
    //D:
    12036.4, 12068.4, 12100.4, 12228.4, 12324.4, 12388.4,  12452.4, 12484.4};


    std::vector<double> pts = {
    //A:
    //6195.1
        2840.4,    2872.4,    2904.4,    3032.4,    3128.4,    3192.4,    3256.4,    3288.4,
   //B:
    //7083.1
        6392.4,    6424.4,    6456.4,    6584.4,    6680.4,    6744.4,    6808.4,    6840.4,
    //C:
    // 7619.1
         8536.4,    8568.4,    8600.4,    8728.4,    8824.4,    8888.4,    8952.4,    8984.4,
    //D:
    //8491.1
       12024.4,    12056.4,    12088.4,    12216.4,    12312.4,    12376.4,    12440.4,    12472.4 };

    //
    // A
    //         0    1    2    6    9    11    13    14
    //
    //         i    j    k    l    m    n    o    p
    // B
    //     vo Band C    111    112    113    117    120    122    124    125
    //
    //         q    r    s    t    u    v    w    x
    // C
    //     covers X-band    178    179    180    184    187    189    191    192
    //
    //         y    z    A    B    C    D    E    F
    // D


//     upper edge of channel
// vr2304 (0)    15    14    13    9    6    4    2    1
// UDC(MHz)    a    b    c    d    e    f    g    h
// A    6195.1    2840.4    2872.4    2904.4    3032.4    3128.4    3192.4    3256.4    3288.4
// 0    1    2    6    9    11    13    14
//
// i    j    k    l    m    n    o    p
// B    7083.1    6392.4    6424.4    6456.4    6584.4    6680.4    6744.4    6808.4    6840.4
// vo Band C    111    112    113    117    120    122    124    125
//
// q    r    s    t    u    v    w    x
// C    7619.1    8536.4    8568.4    8600.4    8728.4    8824.4    8888.4    8952.4    8984.4
// covers X-band    178    179    180    184    187    189    191    192
//
// y    z    A    B    C    D    E    F
// D    8491.1    12024.4    12056.4    12088.4    12216.4    12312.4    12376.4    12440.4    12472.4
// 287    288    289    293    296    298    300    301
    // // pts.push_back(1.0);
    // // pts.push_back(2.0);
    // pts.push_back(3.0);
    // pts.push_back(5.0);
    // pts.push_back(7.0);
    // // pts.push_back(11.32332342);
    // pts.push_back(13.0);
    // pts.push_back(17.0);
    // pts.push_back(17.1);
    // pts.push_back(17.33);
    // pts.push_back(19.0);
    // pts.push_back(71.0);
    // pts.push_back(72.0);



    MHO_UniformGridPointsCalculator grid_calc;
    grid_calc.SetPoints(pts);
    grid_calc.Calculate();

    std::cout<<"info: "<<grid_calc.GetGridStart()<<", "<<grid_calc.GetGridSpacing()<<", "<<grid_calc.GetNGridPoints()<<std::endl;

    return 0;


}
