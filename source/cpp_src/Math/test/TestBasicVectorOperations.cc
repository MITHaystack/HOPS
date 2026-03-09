#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>

#include "MHO_LinearAlgebraUtilities.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{

    const unsigned int NVectors = 100;
    const unsigned int NVectorSize = 8;

    //temporary storage space
    double p1[NVectorSize];
    double p2[NVectorSize];
    double sum1;
    double sum2;

    //allocate a vector
    MHO_linalg_vector<double> v1(NVectorSize);
    MHO_linalg_vector<double> v2(NVectorSize);

    for (unsigned int n = 0; n < NVectors; n++) 
    {
        sum1 = 0.0;
        sum2 = 0.0;

        //generate three points to make the triangle and compute centroid
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            p1[j] = ((double) rand() / (double) RAND_MAX);
            p2[j] = ((double) rand() / (double) RAND_MAX);

            sum1 += p1[j] * p1[j];
            sum2 += p2[j] * p2[j];
        }

        //normalize the test vectors
        sum1 = std::sqrt(sum1);
        sum2 = std::sqrt(sum2);
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            v1(j) = p1[j] / sum1;
            v2(j) = p2[j] / sum2;
        }

        //test the norm routine
        std::cout << "v1 norm = " << v1.norm() << std::endl;;
        std::cout << "v2 norm = " << v2.norm() << std::endl;;

        //test the add/substract routine
        std::cout << "v1 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;;
        v1 = v1 - v2;
        std::cout << "v1 - v2 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;;
        v1 = v1 + v2;
        std::cout << "v1 - v2 + v2 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;;

        //test the scaling routine
        v1.scale(2.0);
        std::cout << "2.0*v1 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;;
        v1.scale(0.5);
        std::cout << "0.5*2.0*v1 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;;


        //test inner product
        std::cout << " v1*v1 = " << v1.inner_product(v1) << std::endl;
        std::cout << " v1*v2 = " << v1.inner_product(v2) << std::endl;
        std::cout << " v2*v2 = " << v2.inner_product(v2) << std::endl;
    }


    return 0;
}
