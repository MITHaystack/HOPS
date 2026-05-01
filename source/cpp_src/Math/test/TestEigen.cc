#include <cmath>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/QR>
#include <Eigen/SVD>

using namespace Eigen;

int main(int /*argc*/, char** /*argv*/)
{
    bool passed = true;
    const double tolerance = 1e-10;

    // --- Test 1: Basic matrix creation and arithmetic ---
    {
        MatrixXd A(3, 3);
        A << 1, 2, 3,
             4, 5, 6,
             7, 8, 9;

        MatrixXd B = MatrixXd::Identity(3, 3);

        MatrixXd C = A * B;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (std::fabs(C(i, j) - A(i, j)) > tolerance)
                {
                    std::cerr << "FAIL: identity multiplication" << std::endl;
                    passed = false;
                }

        std::cout << "Test 1 (identity multiplication): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 2: Vector operations ---
    {
        Vector3d v(1, 2, 3);
        Vector3d w(4, 5, 6);

        double dot = v.dot(w);
        double expected_dot = 1*4 + 2*5 + 3*6;  // 32

        if (std::fabs(dot - expected_dot) > tolerance)
        {
            std::cerr << "FAIL: dot product (got " << dot << ", expected " << expected_dot << ")" << std::endl;
            passed = false;
        }

        Vector3d cross = v.cross(w);
        Vector3d expected_cross(-3, 6, -3);

        if ((cross - expected_cross).norm() > tolerance)
        {
            std::cerr << "FAIL: cross product" << std::endl;
            passed = false;
        }

        std::cout << "Test 2 (vector dot/cross): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 3: Matrix inversion ---
    {
        MatrixXd M(3, 3);
        M << 2, 1, 1,
             1, 3, 2,
             1, 0, 0;

        MatrixXd Minv = M.inverse();
        MatrixXd product = M * Minv;
        MatrixXd I = MatrixXd::Identity(3, 3);

        if ((product - I).norm() > tolerance)
        {
            std::cerr << "FAIL: matrix inversion" << std::endl;
            passed = false;
        }

        std::cout << "Test 3 (matrix inversion): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 4: Solving linear system Ax = b ---
    {
        Matrix3d A;
        A << 1, 2, 3,
             4, 0, 6,
             5, 7, 0;

        Vector3d b(2, 5, 8);
        Vector3d x = A.colPivHouseholderQr().solve(b);

        // Verify: Ax should equal b
        if ((A * x - b).norm() > tolerance)
        {
            std::cerr << "FAIL: linear solve (residual = " << (A * x - b).norm() << ")" << std::endl;
            passed = false;
        }

        std::cout << "Test 4 (linear solve Ax=b): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 5: Eigenvalue decomposition (self-adjoint) ---
    {
        Matrix3d S;
        S << 4, 1, 1,
             1, 3, 0,
             1, 0, 2;

        SelfAdjointEigenSolver<Matrix3d> eig(S);

        if (eig.info() != Success)
        {
            std::cerr << "FAIL: eigenvalue decomposition failed" << std::endl;
            passed = false;
        }
        else
        {
            // Verify: S * V = V * Lambda (check each eigenpair)
            Matrix3d SV = S * eig.eigenvectors();
            Matrix3d VL = eig.eigenvectors() * eig.eigenvalues().asDiagonal();
            if ((SV - VL).norm() > tolerance)
            {
                std::cerr << "FAIL: eigenvalue verification (S*V != V*Lambda)" << std::endl;
                passed = false;
            }
        }

        std::cout << "Test 5 (eigenvalue decomposition): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 6: SVD ---
    {
        MatrixXd A(3, 2);
        A << 1, 0,
             0, 2,
             3, 0;

        JacobiSVD<MatrixXd> svd(A, ComputeFullU | ComputeFullV);

        // Reconstruct A = U * D * V^T with correct sizing
        MatrixXd D = MatrixXd::Zero(A.rows(), A.cols());
        for (int i = 0; i < svd.singularValues().size(); i++)
            D(i, i) = svd.singularValues()(i);
        MatrixXd reconstructed = svd.matrixU() * D * svd.matrixV().adjoint();

        if ((A - reconstructed).norm() > tolerance)
        {
            std::cerr << "FAIL: SVD reconstruction" << std::endl;
            passed = false;
        }

        std::cout << "Test 6 (SVD): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Test 7: Array vs Matrix ---
    {
        ArrayXXd a(2, 2);
        a << 1, 2,
             3, 4;

        // Element-wise square
        ArrayXXd squared = a.square();
        bool ok = (squared(0, 0) == 1) && (squared(0, 1) == 4) &&
                  (squared(1, 0) == 9) && (squared(1, 1) == 16);

        if (!ok)
        {
            std::cerr << "FAIL: element-wise array operations" << std::endl;
            passed = false;
        }

        std::cout << "Test 7 (element-wise array ops): ";
        std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    }

    // --- Summary ---
    std::cout << std::endl;
    if (passed)
    {
        std::cout << "All Eigen tests PASSED." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "Some Eigen tests FAILED." << std::endl;
        return 1;
    }
}
