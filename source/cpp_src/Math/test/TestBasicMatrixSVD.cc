#include <iostream>
#include <cmath>
#include <cassert>

#include "MHO_LinearAlgebraUtilities.hh"

using namespace hops;

int main()
{
    //simple test matrix
    unsigned int m = 3, n = 3;
    MHO_linalg_matrix<double> A(m, n);
    A(0,0) = 1.0;  A(0,1) = 2.0;  A(0,2) = 3.0;
    A(1,0) = 4.0;  A(1,1) = 5.0;  A(1,2) = 6.0;
    A(2,0) = 7.0;  A(2,1) = 8.0;  A(2,2) = 10.0;

    //set up ouput matrix/vectors
    MHO_linalg_matrix<double> U, V;
    MHO_linalg_vector<double> S;

    U.resize(n,m);
    V.resize(m,m);
    S.resize(m);

    //execute the SVD
    MHO_linalg_matrix_svd(A, U, S, V);

    //check reconstruction of the original matrix A ≈ U * diag(S) * V^T
    auto D = MHO_linalg_diag_matrix(S);
    auto VT = MHO_linalg_transpose_matrix(V);
    auto temp = A; 
    auto A_recon = A;
    temp.zero();
    A_recon.zero();
    MHO_linalg_matrix_multiply(D,VT, temp);
    MHO_linalg_matrix_multiply(U,temp, A_recon);

    // MHO_linalg_matrix_print(A);
    // MHO_linalg_matrix_print(A_recon);

    double recon_error = (A - A_recon).frobenius_norm();
    std::cout << "Reconstruction error: " << recon_error << std::endl;

    //check orthogonality of U/V 
    auto UUT = U; 
    UUT.zero();
    MHO_linalg_matrix_multiply(U, MHO_linalg_transpose_matrix(U), UUT);
    auto VVT = V;
    VVT.zero();
    MHO_linalg_matrix_multiply(V, MHO_linalg_transpose_matrix(V), VVT);

    MHO_linalg_matrix<double> Iu(UUT.n_rows(), UUT.n_cols());
    Iu.set_as_identity();
    MHO_linalg_matrix<double> Iv(VVT.n_rows(), VVT.n_cols());
    Iv.set_as_identity();

    double U_orth_err = (UUT - Iu).frobenius_norm();
    double V_orth_err = (VVT - Iv).frobenius_norm();

    std::cout << "U orthogonality error: " << U_orth_err << std::endl;
    std::cout << "V orthogonality error: " << V_orth_err << std::endl;

    //check if we pass/fail
    double tol = 1e-10;
    bool pass = (recon_error < tol && U_orth_err < tol && V_orth_err < tol);

    if(pass)
    {
        std::cout << "SVD test passed" <<std::endl;
        return 0;
    }
    else
    {
        std::cout << "SVD test FAILED"<<std::endl;
        return 1;
    }
}
