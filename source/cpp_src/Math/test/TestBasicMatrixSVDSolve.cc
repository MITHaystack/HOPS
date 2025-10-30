#include <iostream>
#include <cmath>
#include <cassert>
#include "MHO_LinearAlgebraUtilities.hh"

using namespace hops;

int test_square_system()
{
    std::cout << "===== Testing SVD Solve (Square system) =====" << std::endl;

    // define test system A x = b (3x3)
    MHO_linalg_matrix<double> A(3, 3);
    A(0,0) = 3.0;  A(0,1) = 2.0;  A(0,2) = -1.0;
    A(1,0) = 2.0;  A(1,1) = -2.0; A(1,2) = 4.0;
    A(2,0) = -1.0; A(2,1) = 0.5;  A(2,2) = -1.0;

    //set true solution
    MHO_linalg_vector<double> x_true(3);
    x_true(0) = 1.0;
    x_true(1) = -2.0;
    x_true(2) = 3.0;

    //b = A * x_true
    MHO_linalg_vector<double> b(3);
    MHO_linalg_matrix_vector_product(A, x_true, b);

    //compute SVD of A
    MHO_linalg_matrix<double> U, V;
    MHO_linalg_vector<double> S;
    MHO_linalg_matrix_svd(A, U, S, V);

    //solve for x using SVD solve routine
    MHO_linalg_vector<double> x(3);
    MHO_linalg_matrix_svd_solve(U, S, V, b, x);

    //compute the residual r = A*x - b
    MHO_linalg_vector<double> Ax(3);
    MHO_linalg_matrix_vector_product(A, x, Ax);

    MHO_linalg_vector<double> r = Ax - b;
    double residual = r.norm();

    std::cout << "Residual ||A*x - b|| = " << residual << std::endl;

    //compare x vs true solution
    MHO_linalg_vector<double> diff = x - x_true;
    double error = diff.norm();

    std::cout << "Solution error ||x - x_true|| = " << error << std::endl;
    std::cout << "Computed x: [ " << x(0) << ", " << x(1) << ", " << x(2) << " ]" << std::endl;

    double tol = 1e-10;
    if (residual < tol && error < tol)
    {
        std::cout << "SVD solve test passed" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "SVD solve test FAILED" << std::endl;
        return 1;
    }

}

int test_overconstrained_system()
{
    std::cout << "\n=== Test 2: Overconstrained System (Least Squares) ===\n";

    //define a 4x2 system (more equations than unknowns -- overconstrained)
    MHO_linalg_matrix<double> A(4, 2);
    A(0,0) = 1.0;  A(0,1) = 1.0;
    A(1,0) = 1.0;  A(1,1) = 2.0;
    A(2,0) = 1.0;  A(2,1) = 3.0;
    A(3,0) = 1.0;  A(3,1) = 4.0;

    //set true solution for least-squares model: b ≈ A * x_true
    MHO_linalg_vector<double> x_true(2);
    x_true(0) = 2.0;  // intercept
    x_true(1) = 0.5;  // slope

    //construct b with some slight noise
    MHO_linalg_vector<double> b(4);
    for (unsigned int i = 0; i < 4; ++i)
    {
        double x_i = static_cast<double>(i+1);
        b(i) = x_true(0) + x_true(1) * x_i + ((i==2)? 0.01 : 0.0); //add some fake noise
    }

    //compute SVD
    MHO_linalg_matrix<double> U, V;
    MHO_linalg_vector<double> S;
    MHO_linalg_matrix_svd(A, U, S, V);

    //solve for x
    MHO_linalg_vector<double> x(2);
    MHO_linalg_matrix_svd_solve(U, S, V, b, x);

    //compute fitted values and residual
    MHO_linalg_vector<double> Ax(4);
    MHO_linalg_matrix_vector_product(A, x, Ax);
    MHO_linalg_vector<double> r = Ax - b;

    double residual_norm = r.norm();
    std::cout << "Residual ||A*x - b|| = " << residual_norm << std::endl;

    std::cout << "Computed least-squares x = [ " << x(0) << ", " << x(1) << " ]" << std::endl;
    std::cout << "True x_true = [ "<< x_true(0) << ", " << x_true(1) << " ]" <<std::endl;

    //check that residual is small and x is close to x_true
    double error = (x - x_true).norm();
    std::cout << "Parameter error ||x - x_true|| = " << error << std::endl;

    if (error < 1e-2)
    {
        std::cout << "Overconstrained (least-squares) test passed" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Overconstrained (least-squares) test FAILED"<< std::endl;
        return 1;
    }
}



// ------------------------------------------------------
int main()
{
    int res1 = test_square_system();
    int res2 = test_overconstrained_system();
    return res1+res2;

}
