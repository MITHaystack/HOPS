#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding
#include "MHO_FringeFitter.hh"
#include "MHO_FringePlotVisitor.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_FringePlotVisitorFactory.hh"

//for control intialization
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_FringeControlInitialization.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "pybind11_json/pybind11_json.hpp"
    #include <pybind11/embed.h>
    #include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
    #include "MHO_DefaultPythonPlotVisitor.hh"
    #include "MHO_PyConfigurePath.hh"
    #include "MHO_PyFringeDataInterface.hh"
    #include "MHO_PythonOperatorBuilder.hh"
#endif

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"

//wraps the MPI interface (in case it is not enabled)
#include "MHO_MPIInterfaceWrapper.hh"

//set build timestamp, for fourfit plots (legacy behavior)
#ifdef HOPS_BUILD_TIME
    #define HOPS_BUILD_TIMESTAMP STRING(HOPS_BUILD_TIME)
#else
    //no build time defined...default
    #define HOPS_BUILD_TIMESTAMP "2000-01-01T00:00:00.0Z"
#endif





#include <cmath>
#include <cstddef>
#include <vector>

#include <string>
#include <limits>
#include <sstream>
#include <functional>

namespace shsh
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Error Handling 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//error codes
enum class shsh_error 
{
    Success = 0,
    DomainError, //1
    Overflow, //2
    Underflow, //3
    MismatchedDimension, //4
    DivideByZero, //5
    FailedToConverge, //6
    Unknown
};

//make these globals thread local
thread_local shsh_error last_error = shsh_error::Success;
thread_local std::string last_error_msg;

//provide and optional callback method (defaults to null)
using shsh_error_handler = std::function<void(shsh_error, const std::string&)>;
thread_local shsh_error_handler error_handler = nullptr;

inline void report_error(shsh_error code, const std::string& msg) 
{
    last_error = code;
    last_error_msg = msg;
    if (error_handler) 
    {
        error_handler(code, msg);
    }
}

//query functions
inline shsh_error get_last_error() noexcept { return last_error; }
inline const std::string& get_last_error_message() noexcept { return last_error_msg; }
inline void set_error_handler(shsh_error_handler handler) { error_handler = std::move(handler); }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//vector class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename XValueType = double >
class shsh_vector
{
    public:

        shsh_vector():
            fSize(0), 
            fData()
        {};

        shsh_vector(unsigned int sz): 
            fSize(sz)
        {
            
            fData.resize(fSize);
            zero();
        };

        shsh_vector(shsh_vector&& other) noexcept: 
            fSize(other.fSize), 
            fData(std::move(other.fData)) 
        {}

        shsh_vector(const shsh_vector& copy) noexcept: 
            fSize(copy.fSize),
            fData(copy.fData)
        {}

        //no virtual destructor, since we do not plan on inheritance
        //virtual ~shsh_vector() {};

        unsigned int size() const {return fSize;};

        void resize(unsigned int sz)
        {
            if(sz == fSize)
            {
                return;
            }
            fSize = sz;
            fData.resize(fSize);
        }

        void zero()
        {
            std::fill(fData.begin(), fData.end(), 0.0);
        }

        // access (no safety checks)
        XValueType& operator()(unsigned int i) { return fData[i]; }
        const XValueType& operator()(unsigned int i) const { return fData[i]; }

        // assignment
        inline shsh_vector& operator=(const shsh_vector& other) noexcept
        {
            if(this != &other)
            {
                this->fData = other.fData;
            }
            return *this;
        }

        //move assignment
        inline shsh_vector& operator=(shsh_vector&& other) noexcept 
        {
            if (this != &other) 
            {
                fSize = other.fSize;
                fData = std::move(other.fData);
            }
            return *this;
        }

        inline shsh_vector& operator+=(const shsh_vector& other)
        {
            if(this->fSize == other.size() )
            {
                for(unsigned int i = 0; i < fSize; ++i)
                {
                    this->fData[i] += other(i);
                }
            }
            else
            {
                report_error(shsh_error::MismatchedDimension, "shsh_vector::operator+= error, vectors have difference sizes.");
                
            }
            return *this;
        }

        inline shsh_vector& operator-=(const shsh_vector& other)
        {
            if(this->fSize == other.size() )
            {
                for(unsigned int i = 0; i < fSize; ++i)
                {
                    this->fData[i] -= other(i);
                }
            }
            else
            {
                report_error(shsh_error::MismatchedDimension, "shsh_vector::operator-= error, vectors have difference sizes.");
            }
            return *this;
        }


        inline shsh_vector operator+(const shsh_vector& other)
        {
            shsh_vector tmp = *this;
            tmp += other;
            return tmp;
        }

        inline shsh_vector operator-(const shsh_vector& other)
        {
            shsh_vector tmp = *this;
            tmp -= other;
            return tmp;
        }

        //scale by a scalar constant
        inline void scale(const XValueType& scale_factor)
        {
            for(unsigned int i = 0; i < fSize; ++i)
            {
                fData[i] *= scale_factor;
            }
        }

        inline XValueType inner_product(const shsh_vector& b) const
        {
            XValueType val = 0;
            if( fSize == b.size() )
            {
                for(unsigned int i = 0; i < fSize; i++)
                {
                    val += fData[i] * b(i);
                }
            }
            else
            {
                std::stringstream ss;
                ss << "shsh_inner_product: error, vectors have different sizes: " << fSize << " != " << b.size() << "." << "\n";
                report_error(shsh_error::MismatchedDimension, ss.str() );
            }
            return val;
        }

        inline XValueType norm() const
        {
            XValueType val = 0.0;
            for(unsigned int i = 0; i < fSize; i++)
            {
                val += fData[i] * fData[i];
            }
            return std::sqrt(val);
        }

        inline void normalize()
        {
            XValueType norm = this->norm();
            if( norm != 0)
            {
                this->scale(1.0 / norm);
            }
            else 
            {
                report_error(shsh_error::DivideByZero, "shsh_vector::normalize: error, vector has zero norm.");
            }
        }

        inline shsh_vector cross_product(const shsh_vector& b)
        {
            shsh_vector c(3);
            if(b.size() == 3)
            {
                c(0) = fData[1] * b(2) - fData[2] * b(1);
                c(1) = fData[2] * b(0) - fData[0] * b(2);
                c(2) = fData[0] * b(1) - fData[1] * b(0);
            }
            else
            {
                report_error(shsh_error::MismatchedDimension, "shsh_vector_cross_product: error, requires vectors of size 3" );
            }
            return c;
        }

    private:

        unsigned int fSize;
        std::vector<XValueType> fData;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//matrix class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename XValueType = double >
class shsh_matrix
{
    public:

        shsh_matrix() : fNRows(0), fNCols(0), fTotalSize(0), fData() {}

        shsh_matrix(unsigned int nrows, unsigned int ncols):
            fNRows(nrows),
            fNCols(ncols),
            fTotalSize(nrows*ncols),
            fData(nrows*ncols, 0.0)
        {}

        //copy cons.
        shsh_matrix(const shsh_matrix& copy) : 
            fNRows(copy.fNRows), 
            fNCols(copy.fNCols), 
            fTotalSize(copy.fTotalSize),
            fData(copy.fData)
        {}

        //move constructor
        shsh_matrix(shsh_matrix&& other) :
            fNRows(other.fNRows), 
            fNCols(other.fNCols), 
            fTotalSize(other.fTotalSize),
            fData(std::move(other.fData))
        {
            //reset the moved-from object
            other.fNRows = 0;
            other.fNCols = 0;
            other.fTotalSize = 0;
        }

        //no inheritance or virtual functions
        //virtual ~shsh_matrix(){};
 
        void resize(unsigned int nrows, unsigned int ncols)
        {
            if(fTotalSize != nrows*ncols)
            {
                fData.resize(nrows*ncols);
            }
            fNRows = nrows;
            fNCols = ncols;
            fTotalSize = fNRows * fNCols;
        }

        unsigned int n_rows() const {return fNRows;};
        unsigned int n_cols() const {return fNCols;};

        void zero()
        {
            for(unsigned int i = 0; i < fNRows * fNCols; i++)
            {
                fData[i] = 0.0;
            }
        }

        void set_as_identity()
        {
            this->zero();
            unsigned int min;
            if(fNRows < fNCols)
            {
                min = fNRows;
            }
            else
            {
                min = fNCols;
            }
            for(unsigned int i = 0; i < min; i++)
            {
                (*this)(i, i) = 1.0;
            }
        }

        // access (no safety checks)
        XValueType& operator()(unsigned int i, unsigned int j) { return fData[i * fNCols + j]; }
        const XValueType& operator()(unsigned int i, unsigned int j) const { return fData[i * fNCols + j]; }

        // assignment
        inline shsh_matrix& operator=(const shsh_matrix& other)
        {
            if(this == &other)
            {
                return *this;
            }
            this->resize(other.fNRows, other.fNCols);
            this->fData = other.fData;
            return *this;
        }

        //move assignment
        inline shsh_matrix& operator=(shsh_matrix&& other) noexcept 
        {
            if(this != &other) 
            {
                this->fNRows = other.fNRows;
                this->fNCols = other.fNCols;
                this->fTotalSize = other.fTotalSize;
                this->fData = std::move(other.fData);
            }
            //reset the moved-from object
            other.fNRows = 0;
            other.fNCols = 0;
            other.fTotalSize = 0;
            return *this;
        }

        // math
        inline shsh_matrix& operator+=(const shsh_matrix& b)
        {
            if(this->fNRows == b.fNRows && this->fNCols == b.fNCols)
            {
                //same memory layout, so only a single 1D loop is needed
                for(unsigned int i = 0; i < this->fTotalSize; ++i)
                {
                    this->fData[i] += b.fData[i];
                }
            }
            else
            {
                report_error(shsh_error::MismatchedDimension, "shsh_matrix::operator+: error, matrices have difference sizes.");     
            }
            return *this;
        }

        inline shsh_matrix& operator-=(const shsh_matrix& b)
        {
            if(this->fNRows == b.fNRows && this->fNCols == b.fNCols)
            {
                //same memory layout, so only a single 1D loop is needed
                for(unsigned int i = 0; i < this->fTotalSize; ++i)
                {
                    this->fData[i] -= b.fData[i];
                }
            }
            else
            {
                report_error(shsh_error::MismatchedDimension, "shsh_matrix::operator-: error, matrices have difference sizes.");     
            }
            return *this;
        }

        inline shsh_matrix operator+(const shsh_matrix& other)
        {
            shsh_matrix tmp = *this;
            tmp += other;
            return tmp;
        }

        inline shsh_matrix operator-(const shsh_matrix& other)
        {
            shsh_matrix tmp = *this;
            tmp -= other;
            return tmp;
        }

        //scale by a scalar constant
        void scale(const XValueType& scale_factor)
        {
            for(unsigned int i = 0; i < this->fTotalSize; ++i)
            {
                this->fData[i] *= scale_factor;
            }
        }


    private:
        unsigned int fNRows;
        unsigned int fNCols;
        unsigned int fTotalSize;
        std::vector<XValueType> fData;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//matrix-vector operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< typename XValueType = double >
void shsh_matrix_vector_product(const shsh_matrix<XValueType>& m, const shsh_vector<XValueType>& in, shsh_vector<XValueType>& out)
{
    // check sizes
    if((in.size() == m.n_cols()) && (out.size() == m.n_rows()))
    {
        XValueType elem;
        for(unsigned int i = 0; i < m.n_rows(); i++)
        {
            elem = 0.0;
            for(unsigned int j = 0; j < m.n_cols(); j++)
            {
                elem += m(i, j) * in(j);
            }
            out(i) = elem;
        }
    }
    else
    {
        std::stringstream ss;
        ss << "shsh_matrix_vector_product: error, matrix/vector sizes are mismatched." << "\n";
        ss << "matrix m is " << m.n_rows() << " by " << m.n_cols() << "." << "\n";
        ss << "input vector has size " << in.size() << "."  << "\n";
        ss << "output vector has size " << out.size() << "."  << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void shsh_matrix_transpose_vector_product(const shsh_matrix<XValueType>& m, const shsh_vector<XValueType>& in, shsh_vector<XValueType>& out)
{
    // check sizes
    if((in.size() == m.n_rows()) && (out.size() == m.n_cols()))
    {

        XValueType elem;
        for(unsigned int i = 0; i < m.n_cols(); i++)
        {
            elem = 0.0;

            for(unsigned int j = 0; j < m.n_rows(); j++)
            {
                elem += m(j, i) * in(j);
            }
            out(i) = elem;
        }
    }
    else
    {
        std::stringstream ss;
        ss << "shsh_matrix_transpose_vector_product: error, matrix/vector sizes are mismatched." << "\n";
        ss << "transpose of matrix m is " << m.n_cols() << " by " << m.n_rows() << "."  << "\n";
        ss << "input vector has size " << in.size() << "."  << "\n";
        ss << "output vector has size " << out.size() << "."  << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void shsh_vector_outer_product(const shsh_vector<XValueType>& a, const shsh_vector<XValueType>& b, shsh_matrix<XValueType>& p)
{
    //resize if necessary
    if( (a.size() == p.n_rows()) && (b.size() == p.n_cols()) )
    {
        p.resize(a.size(), b.size());
    }

    for(unsigned int i = 0; i < p.n_rows(); i++)
    {
        for(unsigned int j = 0; j < p.n_cols(); j++)
        {
            p(i, j) = a(i) * b(j);
        }
    }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//matrix-matrix operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< typename XValueType = double >
void shsh_matrix_multiply(const shsh_matrix<XValueType>& A, const shsh_matrix<XValueType>& B, shsh_matrix<XValueType>& C)
{
    // check that sizes are valid
    unsigned int a_row = A.n_rows();
    unsigned int a_col = A.n_cols();

    unsigned int b_row = B.n_rows();
    unsigned int b_col = B.n_cols();

    unsigned int c_row = C.n_rows();
    unsigned int c_col = C.n_cols();

    if((a_col == b_row) && (c_row == a_row) && (c_col == b_col))
    {
        // perform super slow naive direct O(N^3) matrix multiplication
        for(unsigned int i = 0; i < c_row; i++)
        {
            for(unsigned int j = 0; j < c_col; j++)
            {
                XValueType elem = 0.0;

                for(unsigned int offset = 0; offset < b_row; offset++)
                {
                    elem += (A(i, offset)) * (B(offset, j));
                }
                C(i, j) = elem;
            }
        }
    }
    else
    {
        std::stringstream ss;
        ss << "shsh_matrix_multiply: error, matrices have different sizes." << "\n";
        ss << "matrix a is " << A.n_rows() << " by " << A.n_cols() << "." << "\n";
        ss << "matrix b is " << B.n_rows() << " by " << B.n_cols() << "." << "\n";
        ss << "matrix c is " << C.n_rows() << " by " << C.n_cols() << "." << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void shsh_matrix_multiply_with_transpose(bool transposeA, bool transposeB, const shsh_matrix<XValueType>& A, const shsh_matrix<XValueType>& B,
                                         shsh_matrix<XValueType>& C)
{
    // check that sizes are valid
    unsigned int a_row = A.n_rows();
    unsigned int a_col = A.n_cols();

    if(transposeA)
    {
        unsigned int swap = a_col;
        a_col = a_row;
        a_row = swap;
    }

    unsigned int b_row = B.n_rows();
    unsigned int b_col = B.n_cols();

    if(transposeB)
    {
        unsigned int swap = b_col;
        b_col = b_row;
        b_row = swap;
    }

    unsigned int c_row = C.n_rows();
    unsigned int c_col = C.n_cols();

    unsigned int ai, aj, bi, bj;

    if((a_col == b_row) && (c_row == a_row) && (c_col == b_col))
    {
        // perform super slow naive direct O(N^3) matrix multiplication
        for(unsigned int i = 0; i < c_row; i++)
        {
            for(unsigned int j = 0; j < c_col; j++)
            {
                XValueType elem = 0.0;

                for(unsigned int offset = 0; offset < b_row; offset++)
                {
                    if(transposeA)
                    {
                        ai = offset;
                        aj = i;
                    }
                    else
                    {
                        ai = i;
                        aj = offset;
                    }

                    if(transposeB)
                    {
                        bi = j;
                        bj = offset;
                    }
                    else
                    {
                        bi = offset;
                        bj = j;
                    }

                    elem += (A(ai, aj)) * (B(bi, bj));
                }

                C(i, j) = elem;
            }
        }
    }
    else
    {
        std::stringstream ss;
        ss << "shsh_matrix_multiply_with_transpose: error, matrices have different sizes." << "\n";
        ss << "matrix a is " << A.n_rows() << " by " << A.n_cols() << "." << "\n";
        ss << "matrix b is " << B.n_rows() << " by " << B.n_cols() << "." << "\n";
        ss << "matrix c is " << C.n_rows() << " by " << C.n_cols() << "." << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void shsh_matrix_svd(const shsh_matrix<XValueType>& A, shsh_matrix<XValueType>& U, shsh_vector<XValueType>& S, shsh_matrix<XValueType>& V)
{
    // this function uses the slower but more accurate one-sided jacobi svd
    // as defined in the paper:
    // Jacobi's method is more accurate than QR by J. Demmel and K. Veselic
    // SIAM. J. Matrix Anal. & Appl., 13(4), 1204-1245.
    // www.netlib.org/lapack/lawnspdf/lawn15.pdf

    // assume that A is m x n
    // then U is an m x n
    // V is n x n
    // S is length n

    unsigned int n = A.n_rows();
    unsigned int m = A.n_cols();

    if(U.n_rows() != n || U.n_cols() != m)
    {
        std::stringstream ss;
        ss << "shsh_matrix_svd: error, matrices A and U have different sizes." << "\n";
        ss << "matrix A is " << A.n_rows() << " by " << A.n_cols() << "." << "\n";
        ss << "matrix U is " << U.n_rows() << " by " << U.n_cols() << "." << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }

    if(n != V.n_rows() || n != V.n_rows())
    {
        std::stringstream ss;
        ss << "shsh_matrix_svd: error, matrix V has wrong size." << "\n";
        ss << "matrix V is " << V.n_rows() << " by " << V.n_cols() << "." << "\n";
        ss << "matrix V should be " << n << " by " << n << "." << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }

    if(S.size() != n)
    {
        std::stringstream ss;
        ss << "shsh_matrix_svd: error, vector S has wrong size." << "\n";
        ss << "vector S is length " << S.size()  << "\n";
        ss << "vector S should be length " << n  << "." << "\n";
        report_error(shsh_error::MismatchedDimension, ss.str() );
    }

    // copy A into U
    U = A;
    // set V to the identity
    //shsh_matrix_set_identity(V);
    V.set_as_identity();

    // put some limits on the number of iterations, this is completely arbitrary
    int n_iter = 0;
    int n_max_iter = 10 * n;

    // scratch space
    XValueType a, b, c, g1, g2, cs, sn, t, psi, sign;

    // make a very rough empirical estimate of the appropriate tolerance
    XValueType tol = 0;
    for(unsigned int i = 0; i < n; i++)
    {
        for(unsigned int j = 0; j < n; j++)
        {
            g1 = U(i, j);
            tol += g1 * g1;
        }
    }
    tol = tol * n * m * std::numeric_limits<XValueType>::epsilon() *  std::numeric_limits<XValueType>::epsilon();

    // convergence count
    int count = 0;
    do
    {
        count = 0;

        // for all column pairs i < j < n
        for(unsigned int i = 0; i < n - 1; i++)
        {
            for(unsigned int j = i + 1; j < n; j++)
            {
                // add to the convergence count
                count++;

                // compute the a,b,c submatrix
                a = 0;
                b = 0;
                c = 0;

                for(unsigned int k = 0; k < m; k++)
                {
                    g1 = U(k, i);
                    g2 = U(k, j);
                    a += g1 * g1;
                    b += g2 * g2;
                    c += g1 * g2;
                }

                if((c * c) / (a * b) > tol)
                {
                    // compute the sine/cosine of the Given's rotation
                    psi = (b - a) / (2.0 * c);

                    sign = 1.0;
                    if(psi < 0)
                    {
                        sign = -1.0;
                    }
                    else
                    {
                        sign = 1.0;
                    }

                    t = (sign) / (std::fabs(psi) + std::sqrt(1.0 + psi * psi));
                    cs = 1.0 / std::sqrt(1.0 + t * t);
                    sn = cs * t;

                    // apply the rotation to U and V
                    for(unsigned int k = 0; k < m; k++)
                    {
                        // apply to U
                        g1 = U(k, i);
                        g2 = U(k, j);
                        U(k, i) = cs * g1 - sn * g2;
                        U(k, j) = sn * g1 + cs * g2;

                        // apply to V
                        g1 = V(k, i);
                        g2 = V(k, j);
                        V(k, i) = cs * g1 - sn * g2;
                        V(k, j) = sn * g1 + cs * g2;
                    }
                }
                else
                {
                    // subtract from convergence count
                    count--;
                }
            }
        }

        n_iter++;

        if(n_iter >= n_max_iter)
        {
            std::stringstream ss;
            ss << "shsh_matrix_svd: error, SVD failed to converge within " << n_max_iter << " iterations, " << "\n";
            report_error(shsh_error::FailedToConverge, ss.str() );
            break;
        }
    }
    while(count > 0);

    // now we compute the singluar values, they are the norms of the columns of U
    // we also compute the norm of all the singular values
    XValueType norm_s = 0.0;
    for(unsigned int i = 0; i < n; i++)
    {
        a = 0;
        for(unsigned int j = 0; j < m; j++)
        {
            g1 = U(j, i);
            a += g1 * g1;
        }
        norm_s += a;
        a = std::sqrt(a);
        S(i) = a;
    }

    norm_s = std::sqrt(norm_s);

    tol = std::numeric_limits<XValueType>::epsilon() * norm_s;

    // eliminate all those singular values which are below the tolerance
    for(unsigned int i = 0; i < n; i++)
    {
        if(S(i) < tol)
        {
            S(i) = 0.0;
        }
    }

    // now we fix U by post multiplying with the inverse of diag(S)
    for(unsigned int i = 0; i < m; i++) // rows
    {
        for(unsigned int j = 0; j < n; j++) // col
        {
            g1 = U(i, j);
            g2 = S(j);
            if(g2 == 0.0)
            {
                U(i, j) = 0.0; // set to zero
            }
            else
            {
                U(i, j) = g1 / g2;
            }
        }
    }
}
template< typename XValueType = double >
void shsh_matrix_transpose(const shsh_matrix<XValueType>& in, shsh_matrix<XValueType>& out)
{

    if((in.n_rows() != out.n_cols()) || (in.n_cols() != out.n_rows()))
    {
        report_error(shsh_error::MismatchedDimension, "shsh_matrix_transpose: error, dimension mismatch." );
    }

    unsigned int nrows = in.n_rows();
    unsigned int ncols = in.n_cols();
    XValueType temp;

    for(unsigned int row = 0; row < nrows; row++)
    {
        for(unsigned int col = 0; col < ncols; col++)
        {
            temp = in(row, col);
            out(col, row) = temp;
        }
    }
}


template< typename XValueType = double >
void shsh_matrix_print(const shsh_matrix<XValueType>& m)
{
    for(unsigned int i = 0; i < m.n_rows(); i++) // rows
    {
        for(unsigned int j = 0; j < (m.n_cols() - 1); j++) // col
        {
            std::cout << m(i, j) << ", ";
        }

        std::cout << m(i, m.n_cols() - 1);

        std::cout << std::endl;
    }
}

template< typename XValueType = double >
void shsh_vector_print(const shsh_vector<XValueType>& m)
{
    for(unsigned int j = 0; j < m.size() ; j++)
    {
        std::cout << m(j) << ", ";
    }
    std::cout << std::endl;
}

} // namespace shsh









using namespace hops;
using namespace shsh;

int main(int argc, char** argv)
{
    int process_id = 0;
    int local_id = 0;
    int n_processes = 1;

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //true -> run with no local even/odd split
    process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
    local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
    n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();
#endif

#ifdef USE_PYBIND11
    //start the interpreter and keep it alive, need this or we segfault
    //each process has its own interpreter
    py::scoped_interpreter guard{};
    configure_pypath();
#endif

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("blingfit"));

    MHO_ParameterStore cmdline_params;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &cmdline_params);
    if(parse_status != 0)
    {
        msg_fatal("main", "could not parse command line options." << eom);
        std::exit(1);
    }

    //flattened pass-info parameters (these are flattened into a single string primarily for MPI)
    std::string concat_delim = ",";
    std::string cscans, croots, cbaselines, cfgroups, cpolprods;

    MPI_SINGLE_PROCESS
    {
        msg_debug("main", "determining the data passes" << eom);
        MHO_BasicFringeDataConfiguration::determine_passes(&cmdline_params, cscans, croots, cbaselines, cfgroups, cpolprods);
        msg_debug("main", "done determining the data passes" << eom);
    }

//use MPI bcast to send all of the pass information to the worker processes
#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->BroadcastString(cscans);
    MHO_MPIInterface::GetInstance()->BroadcastString(croots);
    MHO_MPIInterface::GetInstance()->BroadcastString(cbaselines);
    MHO_MPIInterface::GetInstance()->BroadcastString(cfgroups);
    MHO_MPIInterface::GetInstance()->BroadcastString(cpolprods);
#endif

    std::vector< mho_json > pass_vector;
    MHO_BasicFringeDataConfiguration::split_passes(pass_vector, cscans, croots, cbaselines, cfgroups, cpolprods);
    
    //construct the scan/directory set
    std::set< std::string > scan_set;
    for(std::size_t pass_index = 0; pass_index < pass_vector.size(); pass_index++)
    {
        mho_json pass = pass_vector[pass_index];
        std::string sdir = pass["directory"].get<std::string>();
        scan_set.insert(sdir);
    }

    std::size_t n_scans = scan_set.size();
    MPI_SINGLE_PROCESS
    {
        msg_info("main", "blingfit will fringe " << n_scans << " n_scans of data" << eom);
    }


    //loop over all scans
    for(auto scan_dir_it = scan_set.begin(); scan_dir_it != scan_set.end(); scan_dir_it++)
    {
        //collect all of the passes (baseline, fgroup, pol-prod) associated with a specific scan
        std::string scan_dir = *scan_dir_it;
        std::vector< mho_json > scan_pass_vector;
        for(std::size_t pass_index = 0; pass_index < pass_vector.size(); pass_index++)
        {
            mho_json pass = pass_vector[pass_index];
            std::string sdir = pass["directory"].get<std::string>();
            std::cout<<sdir<<" =? "<<scan_dir<<std::endl;
            if(sdir == scan_dir)
            {
                scan_pass_vector.push_back(pass);
            }
        }

        msg_info("main", "scan " << scan_dir << " contains "<< scan_pass_vector.size()<< " passes of data" << eom);

        //map to store all the fringe data for this scan (this only works with n_processes == 1)!!!
        std::map< std::string, MHO_ParameterStore > scanFringeParameters;
        std::set< std::string > scanStations;

        //this loop could be trivially parallelized (with the exception of plotting)
        for(std::size_t pass_index = 0; pass_index < scan_pass_vector.size(); pass_index++)
        {
            if(pass_index % n_processes == process_id)
            {
                // profiler_start();
                //populate a few necessary parameters and  initialize the fringe/scan data

                //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
                mho_json pass = scan_pass_vector[pass_index];
                pass["build_time"] = HOPS_BUILD_TIMESTAMP; //set the build time stamp in the pass info
                //construct the scan-pass-key from baseline-polprod-fgroup
                std::string bline = pass["baseline"].get<std::string>();
                std::string pprod = pass["polprod"].get<std::string>();
                std::string fg = pass["frequency_group"].get<std::string>();
                std::string pkey = bline + "|" + pprod + "|" + fg; 
                
                std::cout<<"pkey = "<<pkey<<std::endl;
                
                //fringeData = MHO_FringeData();
                MHO_FringeData fringeData;
                fringeData.GetParameterStore()->CopyFrom(cmdline_params); //copy in command line info
                fringeData.GetParameterStore()->Set("/pass", pass);

                //initializes the scan data store, reads the ovex file and sets the value of '/pass/source'
                bool scan_dir_ok = MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(),
                                                                                          fringeData.GetScanDataStore());
                MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(),
                                                                              fringeData.GetScanDataStore());
                                                                              

                //parse the control file and form the control statements
                MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), 
                                                                      fringeData.GetControlFormat(),
                                                                      fringeData.GetControlStatements());
        
                //build the fringe fitter based on the input (only 2 choices currently -- basic and ionospheric)
                MHO_FringeFitterFactory ff_factory(&(fringeData));
                MHO_FringeFitter* ffit = ff_factory.ConstructFringeFitter(); //configuration is done here
        
                //initialize and perform run loop
                while(!ffit->IsFinished())
                {
                    ffit->Initialize();
                    ffit->PreRun();
                    ffit->Run();
                    ffit->PostRun();
                }
                ffit->Finalize();
                
                //keep track of the station codes
                std::string ref_station;
                std::string rem_station;
                fringeData.GetParameterStore()->Get("/config/reference_station", ref_station);
                fringeData.GetParameterStore()->Get("/config/remote_station", rem_station);
                std::cout<<"ref_station = "<<ref_station<<std::endl;
                std::cout<<"rem_station = "<<rem_station<<std::endl;
                scanStations.insert(ref_station);
                scanStations.insert(rem_station);

        
                // //flush profile events
                // profiler_stop();
                // std::vector< MHO_ProfileEvent > events;
                // MHO_Profiler::GetInstance().GetEvents(events);
                // 
                // //convert and dump the events into the parameter store for now (will be empty unless enabled)
                // mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
                // fringeData.GetParameterStore()->Set("/profile/events", event_list);
        
                //determine if this pass was skipped or is in test-mode
                bool is_skipped = fringeData.GetParameterStore()->GetAs< bool >("/status/skipped");
                bool test_mode = fringeData.GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
                
                //copy the fringe parameters 
                scanFringeParameters[pkey].CopyFrom( *(fringeData.GetParameterStore() ) );
        
                //OUTPUT
                // //open and dump to file -- should we profile this as well?
                // if(!test_mode && !is_skipped)
                // {
                //     bool use_mk4_output = false;
                //     fringeData.GetParameterStore()->Get("/cmdline/mk4format_output", use_mk4_output);
                // 
                //     if(!use_mk4_output)
                //     {
                //         fringeData.WriteOutput();
                //     }
                //     else
                //     {
                //         MHO_MK4FringeExport fexporter;
                //         fexporter.SetParameterStore(fringeData.GetParameterStore());
                //         fexporter.SetPlotData(fringeData.GetPlotData());
                //         fexporter.SetContainerStore(fringeData.GetContainerStore());
                //         fexporter.ExportFringeFile();
                //     }
                // }
        
                //use the plotter factory to construct one of the available plotting backends
                if(!is_skipped)
                {
                    //currently we only have two fringe plotting options (gnuplot or matplotlib)
                    std::string plot_backend;
                    fringeData.GetParameterStore()->Get("/control/config/plot_backend", plot_backend);
                    MHO_FringePlotVisitorFactory plotter_factory;
                    MHO_FringePlotVisitor* plotter = plotter_factory.ConstructPlotter(plot_backend);
                    if(plotter != nullptr)
                    {
                        ffit->Accept(plotter);
                    }
                }
            }
        } //end of pass loop
        
        //map stations to integers 
        std::map<std::string, int> station2int;
        std::map<int, std::string> int2station;
        int count = 0;
        for(auto sit = scanStations.begin(); sit != scanStations.end(); sit++)
        {
            std::string st = *sit;
            station2int[st] = count;
            int2station[count] = st;
            std::cout<<count<<": "<<st<<std::endl;
            count++;
        }
        
        std::cout<<"fringe-fitted: "<<scanFringeParameters.size()<<std::endl;
        int nstations = scanStations.size();
        std::cout<<"nstations = "<<nstations<<std::endl;
        shsh_matrix mbd_delta_mx(nstations, nstations);
        shsh_matrix drate_delta_mx(nstations, nstations);
        
        for(auto it = scanFringeParameters.begin(); it != scanFringeParameters.end(); it++)
        {
            std::string pkey = it->first;
            
            std::string ref_station;
            std::string rem_station;
            it->second.Get("/config/reference_station", ref_station);
            it->second.Get("/config/remote_station", rem_station);
            
            int row = station2int[ref_station];
            int col = station2int[rem_station];

            double mbd;
            double drate;
            it->second.Get("/fringe/mbdelay", mbd);
            it->second.Get("/fringe/drate", drate);
            std::cout<<ref_station<<":"<<rem_station<<", "<<"pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<std::endl;

            mbd_delta_mx(row,col) = mbd;
            drate_delta_mx(row,col) = drate;
            
            //make sure we include the opposite entries for skew-symmetric matrix 
            mbd_delta_mx(col, row) = -1.0*mbd;
            drate_delta_mx(col, row) = -1.0*drate;
            
        }
        
        std::cout<<"MBD deltas: "<<std::endl;
        shsh_matrix_print(mbd_delta_mx);
        
        std::cout<<"drate deltas:"<<std::endl;
        shsh_matrix_print(drate_delta_mx);
        
        
        //now solve for the station delays with SVD decomp;
        shsh_matrix U(nstations, nstations);
        shsh_matrix V(nstations, nstations);
        shsh_vector S(nstations);
        shsh_matrix_svd(mbd_delta_mx, U,S,V);
        
        shsh_vector col1(nstations);
        for(int i=0; i<nstations; i++)
        {
            col1(i) = U(i,0)*S(0);
        }
        
        std::cout<<"station delays:"<<std::endl;
        shsh_vector_print(col1);


    }//end of scan loop



#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    return 0;
}
