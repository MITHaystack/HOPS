#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <string>

#include "MHO_LinearAlgebraUtilities.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

// Error state helper
namespace hops
{
inline void clear_last_error()
{
    last_error = MHO_linalg_error::Success;
    last_error_msg.clear();
}
} // namespace hops

int main()
{
    // Suppress MHO_Message output
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eFatal);

    using hops::MHO_linalg_error;
    using hops::MHO_linalg_matrix;
    using hops::MHO_linalg_vector;

    // =========================================================================
    // Vector tests (V1-V10)
    // =========================================================================

    // V1: Constructor zeros
    {
        MHO_linalg_vector< double > v(5);
        for(unsigned int i = 0; i < 5; ++i)
            CHECK_CLOSE(v(i), 0.0, 1e-15);
    }

    // V2: inner_product
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        MHO_linalg_vector< double > b(3);
        b(0) = 4;
        b(1) = 5;
        b(2) = 6;
        CHECK_CLOSE(a.inner_product(b), 32.0, 1e-12);
    }

    // V3: norm and normalize
    {
        MHO_linalg_vector< double > a(2);
        a(0) = 3;
        a(1) = 4;
        CHECK_CLOSE(a.norm(), 5.0, 1e-12);
        a.normalize();
        CHECK_CLOSE(a(0), 0.6, 1e-12);
        CHECK_CLOSE(a(1), 0.8, 1e-12);
    }

    // V4: operator+= and operator-=
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        MHO_linalg_vector< double > b(3);
        b(0) = 4;
        b(1) = 5;
        b(2) = 6;
        a += b;
        CHECK_CLOSE(a(0), 5.0, 1e-12);
        CHECK_CLOSE(a(1), 7.0, 1e-12);
        CHECK_CLOSE(a(2), 9.0, 1e-12);
        a -= b;
        CHECK_CLOSE(a(0), 1.0, 1e-12);
        CHECK_CLOSE(a(1), 2.0, 1e-12);
        CHECK_CLOSE(a(2), 3.0, 1e-12);
    }

    // V5: operator+ and operator-
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        MHO_linalg_vector< double > b(3);
        b(0) = 4;
        b(1) = 5;
        b(2) = 6;
        MHO_linalg_vector< double > c = a + b;
        CHECK_CLOSE(c(0), 5.0, 1e-12);
        CHECK_CLOSE(c(1), 7.0, 1e-12);
        CHECK_CLOSE(c(2), 9.0, 1e-12);
        MHO_linalg_vector< double > d = b - a;
        CHECK_CLOSE(d(0), 3.0, 1e-12);
        CHECK_CLOSE(d(1), 3.0, 1e-12);
        CHECK_CLOSE(d(2), 3.0, 1e-12);
    }

    // V6: scale
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        a.scale(2.5);
        CHECK_CLOSE(a(0), 2.5, 1e-12);
        CHECK_CLOSE(a(1), 5.0, 1e-12);
        CHECK_CLOSE(a(2), 7.5, 1e-12);
    }

    // V7: cross_product 3D
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 0;
        a(2) = 0;
        MHO_linalg_vector< double > b(3);
        b(0) = 0;
        b(1) = 1;
        b(2) = 0;
        MHO_linalg_vector< double > c = a.cross_product(b);
        CHECK_CLOSE(c(0), 0.0, 1e-12);
        CHECK_CLOSE(c(1), 0.0, 1e-12);
        CHECK_CLOSE(c(2), 1.0, 1e-12);
    }

    // V8: cross_product error (size!=3)
    {
        hops::clear_last_error();
        MHO_linalg_vector< double > a(4);
        MHO_linalg_vector< double > b(4);
        MHO_linalg_vector< double > c = a.cross_product(b);
        (void)c;
        REQUIRE(hops::get_last_error() == MHO_linalg_error::MismatchedDimension);
    }

    // V9: normalize-by-zero
    {
        hops::clear_last_error();
        MHO_linalg_vector< double > a(3);
        a.normalize();
        REQUIRE(hops::get_last_error() == MHO_linalg_error::DivideByZero);
    }

    // V10: Bounds check
    {
        hops::clear_last_error();
        MHO_linalg_vector< double > a(3);
        double x = a(7);
        (void)x;
        REQUIRE(hops::get_last_error() == MHO_linalg_error::ArrayOverrun);
    }

    // =========================================================================
    // Matrix tests (M1-M9)
    // =========================================================================

    // M1: Constructor zeros
    {
        MHO_linalg_matrix< double > M(3, 4);
        for(unsigned int i = 0; i < 3; ++i)
            for(unsigned int j = 0; j < 4; ++j)
                CHECK_CLOSE(M(i, j), 0.0, 1e-15);
    }

    // M2: set_as_identity
    {
        MHO_linalg_matrix< double > A(3, 3);
        A.set_as_identity();
        for(unsigned int i = 0; i < 3; ++i)
            for(unsigned int j = 0; j < 3; ++j)
                CHECK_CLOSE(A(i, j), (i == j) ? 1.0 : 0.0, 1e-15);

        MHO_linalg_matrix< double > B(2, 5);
        B.set_as_identity();
        CHECK_CLOSE(B(0, 0), 1.0, 1e-15);
        CHECK_CLOSE(B(1, 1), 1.0, 1e-15);
        // off-diag should be zero
        CHECK_CLOSE(B(0, 1), 0.0, 1e-15);
        CHECK_CLOSE(B(0, 4), 0.0, 1e-15);
        CHECK_CLOSE(B(1, 0), 0.0, 1e-15);
        CHECK_CLOSE(B(1, 4), 0.0, 1e-15);
    }

    // M3: Frobenius norm
    {
        MHO_linalg_matrix< double > A(2, 2);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(1, 0) = 3;
        A(1, 1) = 4;
        CHECK_CLOSE(A.frobenius_norm(), std::sqrt(30.0), 1e-12);
    }

    // M4: +=, -=, scale, +, -
    {
        MHO_linalg_matrix< double > A(2, 2);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(1, 0) = 3;
        A(1, 1) = 4;
        MHO_linalg_matrix< double > B(2, 2);
        B(0, 0) = 5;
        B(0, 1) = 6;
        B(1, 0) = 7;
        B(1, 1) = 8;

        MHO_linalg_matrix< double > C = A + B;
        CHECK_CLOSE(C(0, 0), 6.0, 1e-12);
        CHECK_CLOSE(C(0, 1), 8.0, 1e-12);
        CHECK_CLOSE(C(1, 0), 10.0, 1e-12);
        CHECK_CLOSE(C(1, 1), 12.0, 1e-12);

        C = B - A;
        CHECK_CLOSE(C(0, 0), 4.0, 1e-12);
        CHECK_CLOSE(C(0, 1), 4.0, 1e-12);
        CHECK_CLOSE(C(1, 0), 4.0, 1e-12);
        CHECK_CLOSE(C(1, 1), 4.0, 1e-12);

        A.scale(0.5);
        CHECK_CLOSE(A(0, 0), 0.5, 1e-12);
        CHECK_CLOSE(A(0, 1), 1.0, 1e-12);
        CHECK_CLOSE(A(1, 0), 1.5, 1e-12);
        CHECK_CLOSE(A(1, 1), 2.0, 1e-12);
    }

    // M5: matrix-vector product
    {
        MHO_linalg_matrix< double > A(2, 3);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(0, 2) = 3;
        A(1, 0) = 4;
        A(1, 1) = 5;
        A(1, 2) = 6;
        MHO_linalg_vector< double > v(3);
        v(0) = 1;
        v(1) = 1;
        v(2) = 1;
        MHO_linalg_vector< double > out(2);
        hops::MHO_linalg_matrix_vector_product(A, v, out);
        CHECK_CLOSE(out(0), 6.0, 1e-12);
        CHECK_CLOSE(out(1), 15.0, 1e-12);
    }

    // M6: transpose-matrix-vector product
    {
        MHO_linalg_matrix< double > A(2, 3);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(0, 2) = 3;
        A(1, 0) = 4;
        A(1, 1) = 5;
        A(1, 2) = 6;
        MHO_linalg_vector< double > v(2);
        v(0) = 1;
        v(1) = 1;
        MHO_linalg_vector< double > out(3);
        hops::MHO_linalg_matrix_transpose_vector_product(A, v, out);
        CHECK_CLOSE(out(0), 5.0, 1e-12);
        CHECK_CLOSE(out(1), 7.0, 1e-12);
        CHECK_CLOSE(out(2), 9.0, 1e-12);
    }

    // M7: outer product
    {
        MHO_linalg_vector< double > a(3);
        a(0) = 1;
        a(1) = 2;
        a(2) = 3;
        MHO_linalg_vector< double > b(2);
        b(0) = 4;
        b(1) = 5;
        MHO_linalg_matrix< double > p(3, 2);
        hops::MHO_linalg_vector_outer_product(a, b, p);
        CHECK_CLOSE(p(0, 0), 4.0, 1e-12);
        CHECK_CLOSE(p(0, 1), 5.0, 1e-12);
        CHECK_CLOSE(p(1, 0), 8.0, 1e-12);
        CHECK_CLOSE(p(1, 1), 10.0, 1e-12);
        CHECK_CLOSE(p(2, 0), 12.0, 1e-12);
        CHECK_CLOSE(p(2, 1), 15.0, 1e-12);
    }

    // M8: matrix multiply A*B
    {
        MHO_linalg_matrix< double > A(2, 2);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(1, 0) = 3;
        A(1, 1) = 4;
        MHO_linalg_matrix< double > B(2, 2);
        B(0, 0) = 5;
        B(0, 1) = 6;
        B(1, 0) = 7;
        B(1, 1) = 8;
        MHO_linalg_matrix< double > C(2, 2);
        hops::MHO_linalg_matrix_multiply(A, B, C);
        CHECK_CLOSE(C(0, 0), 19.0, 1e-12);
        CHECK_CLOSE(C(0, 1), 22.0, 1e-12);
        CHECK_CLOSE(C(1, 0), 43.0, 1e-12);
        CHECK_CLOSE(C(1, 1), 50.0, 1e-12);
    }

    // M9: multiply_with_transpose - all 4 combos
    {
        // A is 2x3, B is 2x3
        MHO_linalg_matrix< double > A(2, 3);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(0, 2) = 3;
        A(1, 0) = 4;
        A(1, 1) = 5;
        A(1, 2) = 6;
        MHO_linalg_matrix< double > B(2, 3);
        B(0, 0) = 1;
        B(0, 1) = 0;
        B(0, 2) = 1;
        B(1, 0) = 0;
        B(1, 1) = 1;
        B(1, 2) = 0;

        // (false, false): A * B -> 2x3 * 2x3 -> inner dims 3!=2 -> MismatchedDimension
        {
            hops::clear_last_error();
            MHO_linalg_matrix< double > C(2, 2);
            hops::MHO_linalg_matrix_multiply_with_transpose(false, false, A, B, C);
            REQUIRE(hops::get_last_error() == MHO_linalg_error::MismatchedDimension);
        }

        // (false, true): A * B^T -> 2x3 * 3x2 = 2x2
        {
            auto C2 = hops::MHO_linalg_matrix_multiply_with_transpose(false, true, A, B);
            REQUIRE(C2.n_rows() == 2);
            REQUIRE(C2.n_cols() == 2);
            // C(i,j) = sum_k A(i,k) * B(j,k)  (i.e. A * B^T)
            // C(0,0) = 1*1+2*0+3*1 = 4
            // C(0,1) = 1*0+2*1+3*0 = 2
            // C(1,0) = 4*1+5*0+6*1 = 10
            // C(1,1) = 4*0+5*1+6*0 = 5
            CHECK_CLOSE(C2(0, 0), 4.0, 1e-12);
            CHECK_CLOSE(C2(0, 1), 2.0, 1e-12);
            CHECK_CLOSE(C2(1, 0), 10.0, 1e-12);
            CHECK_CLOSE(C2(1, 1), 5.0, 1e-12);
        }

        // (true, false): A^T * B -> 3x2 * 2x3 = 3x3
        {
            auto C2 = hops::MHO_linalg_matrix_multiply_with_transpose(true, false, A, B);
            REQUIRE(C2.n_rows() == 3);
            REQUIRE(C2.n_cols() == 3);
            // C(i,j) = sum_k A(k,i) * B(k,j)  (i.e. A^T * B)
            // C(0,0) = 1*1+4*0 = 1
            // C(0,1) = 1*0+4*1 = 4
            // C(0,2) = 1*1+4*0 = 1
            // C(1,0) = 2*1+5*0 = 2
            // C(1,1) = 2*0+5*1 = 5
            // C(1,2) = 2*1+5*0 = 2
            // C(2,0) = 3*1+6*0 = 3
            // C(2,1) = 3*0+6*1 = 6
            // C(2,2) = 3*1+6*0 = 3
            CHECK_CLOSE(C2(0, 0), 1.0, 1e-12);
            CHECK_CLOSE(C2(0, 1), 4.0, 1e-12);
            CHECK_CLOSE(C2(0, 2), 1.0, 1e-12);
            CHECK_CLOSE(C2(1, 0), 2.0, 1e-12);
            CHECK_CLOSE(C2(1, 1), 5.0, 1e-12);
            CHECK_CLOSE(C2(1, 2), 2.0, 1e-12);
            CHECK_CLOSE(C2(2, 0), 3.0, 1e-12);
            CHECK_CLOSE(C2(2, 1), 6.0, 1e-12);
            CHECK_CLOSE(C2(2, 2), 3.0, 1e-12);
        }

        // (true, true): A^T * B^T -> 3x2 * 3x2 -> inner dims 2!=3 -> MismatchedDimension
        {
            hops::clear_last_error();
            MHO_linalg_matrix< double > C(3, 2);
            hops::MHO_linalg_matrix_multiply_with_transpose(true, true, A, B, C);
            REQUIRE(hops::get_last_error() == MHO_linalg_error::MismatchedDimension);
        }
    }

    // =========================================================================
    // SVD tests (S1-S5)
    // =========================================================================

    // S1: SVD round-trip on tall matrix (8x3, random with seed 0xDEADBEEF)
    {
        std::mt19937 rng(0xDEADBEEF);
        std::normal_distribution< double > dist(0.0, 1.0);
        unsigned int n = 8, m = 3;
        MHO_linalg_matrix< double > A(n, m);
        for(unsigned int i = 0; i < n; ++i)
            for(unsigned int j = 0; j < m; ++j)
                A(i, j) = dist(rng);

        MHO_linalg_matrix< double > U(n, m);
        MHO_linalg_vector< double > S(m);
        MHO_linalg_matrix< double > V(m, m);
        hops::MHO_linalg_matrix_svd(A, U, S, V);

        // Reconstruct A_hat = U * diag(S) * V^T
        auto D = hops::MHO_linalg_diag_matrix(S);              // m x m
        auto UD = hops::MHO_linalg_matrix_multiply(U, D);      // n x m
        auto VT = hops::MHO_linalg_transpose_matrix(V);        // m x m
        auto A_hat = hops::MHO_linalg_matrix_multiply(UD, VT); // n x m

        // ||A - A_hat||_F < 1e-9
        MHO_linalg_matrix< double > diff(n, m);
        for(unsigned int i = 0; i < n; ++i)
            for(unsigned int j = 0; j < m; ++j)
                diff(i, j) = A(i, j) - A_hat(i, j);
        CHECK_CLOSE(diff.frobenius_norm(), 0.0, 1e-9);

        // Check U orthogonality: ||U^T * U - I_3||_F < 1e-9
        auto UT = hops::MHO_linalg_transpose_matrix(U);     // m x n
        auto UTU = hops::MHO_linalg_matrix_multiply(UT, U); // m x m
        MHO_linalg_matrix< double > I_m(m, m);
        I_m.set_as_identity();
        auto U_diff = UTU - I_m;
        CHECK_CLOSE(U_diff.frobenius_norm(), 0.0, 1e-9);

        // Check V orthogonality: ||V^T * V - I_3||_F < 1e-9
        auto VT2 = hops::MHO_linalg_transpose_matrix(V);     // m x m
        auto VTV = hops::MHO_linalg_matrix_multiply(VT2, V); // m x m
        auto V_diff = VTV - I_m;
        CHECK_CLOSE(V_diff.frobenius_norm(), 0.0, 1e-9);
    }

    // S2: SVD on rank-deficient matrix: A=[[1,2],[2,4],[3,6]]
    {
        MHO_linalg_matrix< double > A(3, 2);
        A(0, 0) = 1;
        A(0, 1) = 2;
        A(1, 0) = 2;
        A(1, 1) = 4;
        A(2, 0) = 3;
        A(2, 1) = 6;

        MHO_linalg_matrix< double > U(3, 2);
        MHO_linalg_vector< double > S(2);
        MHO_linalg_matrix< double > V(2, 2);
        hops::MHO_linalg_matrix_svd(A, U, S, V);

        // Exactly one singular value non-zero, other clipped to 0
        int nonZeroCount = 0;
        for(unsigned int i = 0; i < 2; ++i)
        {
            if(std::fabs(S(i)) > 1e-14)
                nonZeroCount++;
        }
        REQUIRE(nonZeroCount == 1);
    }

    // S3: SVD fat-matrix error
    {
        hops::clear_last_error();
        MHO_linalg_matrix< double > A(2, 4);
        MHO_linalg_matrix< double > U(2, 4);
        MHO_linalg_vector< double > S(4);
        MHO_linalg_matrix< double > V(4, 4);
        hops::MHO_linalg_matrix_svd(A, U, S, V);
        REQUIRE(hops::get_last_error() == MHO_linalg_error::MismatchedDimension);
    }

    // S4: svd_solve on square system
    {
        MHO_linalg_matrix< double > A(3, 3);
        A(0, 0) = 3;
        A(0, 1) = 2;
        A(0, 2) = -1;
        A(1, 0) = 2;
        A(1, 1) = -2;
        A(1, 2) = 4;
        A(2, 0) = -1;
        A(2, 1) = 0.5;
        A(2, 2) = -1;

        MHO_linalg_vector< double > x_true(3);
        x_true(0) = 1;
        x_true(1) = -2;
        x_true(2) = 3;

        MHO_linalg_vector< double > b(3);
        hops::MHO_linalg_matrix_vector_product(A, x_true, b);

        MHO_linalg_matrix< double > U(3, 3);
        MHO_linalg_vector< double > S(3);
        MHO_linalg_matrix< double > V(3, 3);
        hops::MHO_linalg_matrix_svd(A, U, S, V);

        MHO_linalg_vector< double > x(3);
        hops::MHO_linalg_matrix_svd_solve(U, S, V, b, x);

        // ||x - x_true|| < 1e-10
        MHO_linalg_vector< double > diff = x - x_true;
        CHECK_CLOSE(diff.norm(), 0.0, 1e-10);
    }

    // S5: svd_solve on rank-deficient
    {
        MHO_linalg_matrix< double > A(2, 2);
        A(0, 0) = 1;
        A(0, 1) = 1;
        A(1, 0) = 1;
        A(1, 1) = 1;

        MHO_linalg_vector< double > b(2);
        b(0) = 2;
        b(1) = 2;

        MHO_linalg_matrix< double > U(2, 2);
        MHO_linalg_vector< double > S(2);
        MHO_linalg_matrix< double > V(2, 2);
        hops::MHO_linalg_matrix_svd(A, U, S, V);

        MHO_linalg_vector< double > x(2);
        hops::MHO_linalg_matrix_svd_solve(U, S, V, b, x);

        // x should be {1,1} within 1e-9
        CHECK_CLOSE(x(0), 1.0, 1e-9);
        CHECK_CLOSE(x(1), 1.0, 1e-9);
    }

    // =========================================================================
    // Error reporting (E1)
    // =========================================================================

    // E1: Custom error handler
    {
        hops::clear_last_error();
        bool callbackFired = false;
        MHO_linalg_error capturedCode;
        std::string capturedMsg;
        hops::set_error_handler([&callbackFired, &capturedCode, &capturedMsg](MHO_linalg_error code, const std::string& msg) {
            callbackFired = true;
            capturedCode = code;
            capturedMsg = msg;
        });

        // Trigger a dimension mismatch: vector inner_product with different sizes
        MHO_linalg_vector< double > a(3);
        MHO_linalg_vector< double > b(4);
        a.inner_product(b);

        REQUIRE(callbackFired);
        REQUIRE(capturedCode == MHO_linalg_error::MismatchedDimension);

        // Reset handler
        hops::set_error_handler(nullptr);
    }

    return 0;
}
