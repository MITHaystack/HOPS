#include <iostream>
#include <cmath>
#include <iomanip>

#include "MHO_LinearAlgebraUtilities.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{

    const unsigned int NVectors = 100;
    const unsigned int NVectorSize = 3;

    //allocate a vector
    MHO_linalg_vector<double> v1(NVectorSize);
    MHO_linalg_vector<double> v2(NVectorSize);
    MHO_linalg_vector<double> v3(NVectorSize);

    //allocate 4 matrices, to construct an euler rotation
    MHO_linalg_matrix<double> m1(NVectorSize, NVectorSize);
    MHO_linalg_matrix<double> m2(NVectorSize, NVectorSize);
    MHO_linalg_matrix<double> m3(NVectorSize, NVectorSize);
    MHO_linalg_matrix<double> m4(NVectorSize, NVectorSize);
    MHO_linalg_matrix<double> m4_inv(NVectorSize, NVectorSize);
    MHO_linalg_matrix<double> temp(NVectorSize, NVectorSize);

    //generate three angles
    double alpha = M_PI / 2.2313;
    double beta = M_PI / 4.934857;
    double gamma = M_PI / 3.9487;

    //construct some rotation matrices operating on Z, Y', Z''
    m1.set_as_identity();
    m1(0, 0) = std::cos(alpha);
    m1(0, 1) = -1.0 * std::sin(alpha);
    m1(1, 0) = std::sin(alpha);
    m1(1, 1) = std::cos(alpha);

    MHO_linalg_matrix_print(m1);

    m2.set_as_identity();
    m2(0, 0) = std::cos(beta);
    m2(0, 2) = -1.0 * std::sin(beta);
    m2(2, 0) = std::sin(beta);
    m2(2, 2) = std::cos(beta);

    MHO_linalg_matrix_print(m2);

    m3.set_as_identity();
    m3(0, 0) = std::cos(gamma);
    m3(0, 1) =  -1.0 * std::sin(gamma);
    m3(1, 0) = std::sin(gamma);
    m3(1, 1) = std::cos(gamma);

    MHO_linalg_matrix_print(m3);

    m4.set_as_identity();
    m4_inv.set_as_identity();

    //compute the euler matrix
    MHO_linalg_matrix_multiply(m1, m4, temp);
    m4 = temp;
    MHO_linalg_matrix_multiply(m2, m4, temp);
    m4 = temp;
    MHO_linalg_matrix_multiply(m3, m4, temp);
    m4 = temp;

    MHO_linalg_matrix_print(m4);

    //compute the euler matrix inverse
    MHO_linalg_matrix_multiply_with_transpose(true, false, m3, m4_inv, temp);
    m4_inv = temp;
    MHO_linalg_matrix_multiply_with_transpose(true, false, m2, m4_inv, temp);
    m4_inv = temp;
    MHO_linalg_matrix_multiply_with_transpose(true, false, m1, m4_inv, temp);
    m4_inv = temp;

    MHO_linalg_matrix_print(m4_inv);

    for (unsigned int n = 0; n < NVectors; n++) 
    {
        //generate three points to make the triangle and compute centroid
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            v1(j) =  ((double) rand() / (double) RAND_MAX);
        }
        v1.normalize();

        //test the norm routine
        std::cout << "|v1| = " << v1.norm() << std::endl;

        std::cout << "v1 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            std::cout << v1(j) << ", ";
        }
        std::cout << std::endl;

        //now test against series of discrete rotations
        v2 = v1;
        //MHO_linalg_vector_set(v1, v2);
        MHO_linalg_matrix_vector_product(m1, v2, v3);
        MHO_linalg_matrix_vector_product(m2, v3, v2);
        MHO_linalg_matrix_vector_product(m3, v2, v3);
        std::cout << "M3*M2*M1*v = ";
        for (unsigned int j = 0; j < NVectorSize; j++)
        {
            std::cout << v3(j) << ", ";
        }
        std::cout << std::endl;

        MHO_linalg_matrix_transpose_vector_product(m3, v3, v2);
        MHO_linalg_matrix_transpose_vector_product(m2, v2, v3);
        MHO_linalg_matrix_transpose_vector_product(m1, v3, v2);

        //multiply by series of inverse rotations
        std::cout << "(M1^T)*(M2^T)*(M3^T)*v = ";
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            std::cout << v2(j) << ", ";
        }
        std::cout << std::endl;

        std::cout << "----------------------------------------------------" << std::endl;

        //multiply by full rotation
        //MHO_linalg_vector_set(v1, v2);
        v2 = v1;
        MHO_linalg_matrix_vector_product(m4, v2, v3);
        std::cout << "m4*v1 = ";
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            std::cout << v3(j) << ", ";
        }
        std::cout << std::endl;

        //multiply by full inverse rotation
        MHO_linalg_matrix_vector_product(m4_inv, v3, v2);

        std::cout << "(m4^T)*(M4*v) = ";
        for (unsigned int j = 0; j < NVectorSize; j++) 
        {
            std::cout << v2(j) << ", ";
        }
        std::cout << std::endl;

        std::cout << "----------------------------------------------------" << std::endl;


        std::cout << "####################################################" << std::endl;
    }


    //multiply rotation by inverse
    MHO_linalg_matrix_multiply(m4_inv, m4, temp);
    MHO_linalg_matrix_print(temp);


    return 0;
}
