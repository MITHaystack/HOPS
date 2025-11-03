#ifndef MHO_LinearAlgebraUtilities_HH__
#define MHO_LinearAlgebraUtilities_HH__

#include <cmath>
#include <cstddef>
#include <complex>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <functional>

#include "MHO_Message.hh"

#define MHO_LINALG_PRINT_ERROR

#define MHO_CHECK_ARRAY_OVERRUN

namespace hops
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Error Handling 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//error codes
enum class MHO_linalg_error 
{
    Success = 0,
    DomainError, //1
    Overflow, //2
    Underflow, //3
    MismatchedDimension, //4
    DivideByZero, //5
    FailedToConverge, //6
    ArrayOverrun, //7
    Unknown
};

//make these globals thread local
thread_local MHO_linalg_error last_error = MHO_linalg_error::Success;
thread_local std::string last_error_msg;

//provide and optional callback method (defaults to null)
using MHO_linalg_error_handler = std::function<void(MHO_linalg_error, const std::string&)>;
thread_local MHO_linalg_error_handler error_handler = nullptr;

inline void report_error(MHO_linalg_error code, const std::string& msg) 
{
    last_error = code;
    last_error_msg = msg;
    if (error_handler) 
    {
        error_handler(code, msg);
    }
    #ifdef MHO_LINALG_PRINT_ERROR
    if( code != MHO_linalg_error::Success)
    {
        msg_warn("math", msg << eom);
    }
    #endif
}

//query functions
inline MHO_linalg_error get_last_error() noexcept { return last_error; }
inline const std::string& get_last_error_message() noexcept { return last_error_msg; }
inline void set_error_handler(MHO_linalg_error_handler handler) { error_handler = std::move(handler); }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//vector class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename XValueType = double >
class MHO_linalg_vector
{
    public:

        MHO_linalg_vector():
            fSize(0), 
            fData()
        {};

        MHO_linalg_vector(unsigned int sz): 
            fSize(sz)
        {
            
            fData.resize(fSize);
            zero();
        };

        MHO_linalg_vector(MHO_linalg_vector&& other) noexcept: 
            fSize(other.fSize), 
            fData(std::move(other.fData)) 
        {}

        MHO_linalg_vector(const MHO_linalg_vector& copy) noexcept: 
            fSize(copy.fSize),
            fData(copy.fData)
        {}

        //no virtual destructor, since we do not plan on inheritance
        //virtual ~MHO_linalg_vector() {};

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
        XValueType& operator()(unsigned int i) 
        {
            #ifndef MHO_CHECK_ARRAY_OVERRUN
                return fData[i];
            #else
                if(i < fSize)
                {
                    return fData[i]; 
                }
                else
                {
                    std::stringstream ss;
                    ss << "MHO_linalg_vector::operator() error, out of range: " << i << " !< " << fSize;
                    report_error(MHO_linalg_error::ArrayOverrun, ss.str());
                    return fDummy;
                }
            #endif
        }

        const XValueType& operator()(unsigned int i) const 
        { 
            #ifndef MHO_CHECK_ARRAY_OVERRUN
                return fData[i];
            #else
                if(i < fSize)
                {
                    return fData[i]; 
                }
                else
                {
                    std::stringstream ss;
                    ss << "MHO_linalg_vector::operator() error, out of range: " << i << " !< " << fSize;
                    report_error(MHO_linalg_error::ArrayOverrun, ss.str());
                    return fDummy;
                }
            #endif
        }

        // assignment
        inline MHO_linalg_vector& operator=(const MHO_linalg_vector& other) noexcept
        {
            if(this != &other)
            {
                this->fData = other.fData;
            }
            return *this;
        }

        //move assignment
        inline MHO_linalg_vector& operator=(MHO_linalg_vector&& other) noexcept 
        {
            if (this != &other) 
            {
                fSize = other.fSize;
                fData = std::move(other.fData);
            }
            return *this;
        }

        inline MHO_linalg_vector& operator+=(const MHO_linalg_vector& other)
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
                report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_vector::operator+= error, vectors have difference sizes.");
            }
            return *this;
        }

        inline MHO_linalg_vector& operator-=(const MHO_linalg_vector& other)
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
                report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_vector::operator-= error, vectors have difference sizes.");
            }
            return *this;
        }


        inline MHO_linalg_vector operator+(const MHO_linalg_vector& other)
        {
            MHO_linalg_vector tmp = *this;
            tmp += other;
            return tmp;
        }

        inline MHO_linalg_vector operator-(const MHO_linalg_vector& other)
        {
            MHO_linalg_vector tmp = *this;
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

        inline XValueType inner_product(const MHO_linalg_vector& b) const
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
                ss << "MHO_linalg_inner_product: error, vectors have different sizes: " << fSize << " != " << b.size() << "." << "\n";
                report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
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
                report_error(MHO_linalg_error::DivideByZero, "MHO_linalg_vector::normalize: error, vector has zero norm.");
            }
        }

        inline MHO_linalg_vector cross_product(const MHO_linalg_vector& b)
        {
            MHO_linalg_vector c(3);
            if(b.size() == 3)
            {
                c(0) = fData[1] * b(2) - fData[2] * b(1);
                c(1) = fData[2] * b(0) - fData[0] * b(2);
                c(2) = fData[0] * b(1) - fData[1] * b(0);
            }
            else
            {
                report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_vector_cross_product: error, requires vectors of size 3" );
            }
            return c;
        }

    private:

        unsigned int fSize;
        std::vector<XValueType> fData;
        XValueType fDummy;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//matrix class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename XValueType = double >
class MHO_linalg_matrix
{
    public:

        MHO_linalg_matrix() : fNRows(0), fNCols(0), fTotalSize(0), fData() {}

        MHO_linalg_matrix(unsigned int nrows, unsigned int ncols):
            fNRows(nrows),
            fNCols(ncols),
            fTotalSize(nrows*ncols),
            fData(nrows*ncols, 0.0)
        {}

        //copy cons.
        MHO_linalg_matrix(const MHO_linalg_matrix& copy) : 
            fNRows(copy.fNRows), 
            fNCols(copy.fNCols), 
            fTotalSize(copy.fTotalSize),
            fData(copy.fData)
        {}

        //move constructor
        MHO_linalg_matrix(MHO_linalg_matrix&& other) :
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
        //virtual ~MHO_linalg_matrix(){};
 
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
        XValueType& operator()(unsigned int i, unsigned int j) 
        {
            #ifndef MHO_CHECK_ARRAY_OVERRUN
                return fData[i * fNCols + j];
            #else 
                if(i < fNRows && j < fNCols)
                {
                    return fData[i * fNCols + j];
                }
                else 
                {
                    std::stringstream ss;
                    ss << "MHO_linalg_matrix::operator() error, out of range, row: " << i << " !< " << fNRows << " or col: " << j << " !< " << fNCols;
                    report_error(MHO_linalg_error::ArrayOverrun, ss.str());
                    return fDummy;
                }
            #endif
        }

        const XValueType& operator()(unsigned int i, unsigned int j) const 
        {
            #ifndef MHO_CHECK_ARRAY_OVERRUN
                return fData[i * fNCols + j];
            #else 
                if(i < fNRows && j < fNCols)
                {
                    return fData[i * fNCols + j];
                }
                else 
                {
                    std::stringstream ss;
                    ss << "MHO_linalg_matrix::operator() error, out of range, row: " << i << " !< " << fNRows << " or col: " << j << " !< " << fNCols;
                    report_error(MHO_linalg_error::ArrayOverrun, ss.str());
                    return fDummy;
                }
            #endif
        }

        // assignment
        inline MHO_linalg_matrix& operator=(const MHO_linalg_matrix& other)
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
        inline MHO_linalg_matrix& operator=(MHO_linalg_matrix&& other) noexcept 
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
        inline MHO_linalg_matrix& operator+=(const MHO_linalg_matrix& b)
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
                report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_matrix::operator+: error, matrices have difference sizes.");     
            }
            return *this;
        }

        inline MHO_linalg_matrix& operator-=(const MHO_linalg_matrix& b)
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
                report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_matrix::operator-: error, matrices have difference sizes.");     
            }
            return *this;
        }

        inline MHO_linalg_matrix operator+(const MHO_linalg_matrix& other)
        {
            MHO_linalg_matrix tmp = *this;
            tmp += other;
            return tmp;
        }

        inline MHO_linalg_matrix operator-(const MHO_linalg_matrix& other)
        {
            MHO_linalg_matrix tmp = *this;
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
        
        //Frobenius norm of matrix
        XValueType frobenius_norm()
        {
            XValueType sum = 0;
            for(unsigned int i = 0; i < fTotalSize; ++i)
            {
                sum += fData[i]*fData[i]; // TODO FIXME FOR COMPLEX DATA
            }
            return std::sqrt(sum);
        }


    private:
        unsigned int fNRows;
        unsigned int fNCols;
        unsigned int fTotalSize;
        std::vector<XValueType> fData;
        XValueType fDummy;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//matrix-vector operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< typename XValueType = double >
void MHO_linalg_matrix_vector_product(const MHO_linalg_matrix<XValueType>& m, const MHO_linalg_vector<XValueType>& in, MHO_linalg_vector<XValueType>& out)
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
        ss << "MHO_linalg_matrix_vector_product: error, matrix/vector sizes are mismatched." << "\n";
        ss << "matrix m is " << m.n_rows() << " by " << m.n_cols() << "." << "\n";
        ss << "input vector has size " << in.size() << "."  << "\n";
        ss << "output vector has size " << out.size() << "."  << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void MHO_linalg_matrix_transpose_vector_product(const MHO_linalg_matrix<XValueType>& m, const MHO_linalg_vector<XValueType>& in, MHO_linalg_vector<XValueType>& out)
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
        ss << "MHO_linalg_matrix_transpose_vector_product: error, matrix/vector sizes are mismatched." << "\n";
        ss << "transpose of matrix m is " << m.n_cols() << " by " << m.n_rows() << "."  << "\n";
        ss << "input vector has size " << in.size() << "."  << "\n";
        ss << "output vector has size " << out.size() << "."  << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
    }
}

template< typename XValueType = double >
void MHO_linalg_vector_outer_product(const MHO_linalg_vector<XValueType>& a, const MHO_linalg_vector<XValueType>& b, MHO_linalg_matrix<XValueType>& p)
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

//C = A x B
template< typename XValueType = double >
void MHO_linalg_matrix_multiply(const MHO_linalg_matrix<XValueType>& A, const MHO_linalg_matrix<XValueType>& B, MHO_linalg_matrix<XValueType>& C)
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
        ss << "MHO_linalg_matrix_multiply: error, matrices have different sizes." << "\n";
        ss << "matrix a is " << A.n_rows() << " by " << A.n_cols() << "." << "\n";
        ss << "matrix b is " << B.n_rows() << " by " << B.n_cols() << "." << "\n";
        ss << "matrix c is " << C.n_rows() << " by " << C.n_cols() << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
    }
}

//matrix multiply which constructs return matrix
template< typename XValueType = double >
MHO_linalg_matrix<XValueType> 
MHO_linalg_matrix_multiply(const MHO_linalg_matrix<XValueType>& A, const MHO_linalg_matrix<XValueType>& B)
{
    MHO_linalg_matrix<XValueType> C;
    C.resize(A.n_rows(), B.n_cols());
    MHO_linalg_matrix_multiply(A,B,C);
    return C;
}

template< typename XValueType = double >
void MHO_linalg_matrix_multiply_with_transpose(bool transposeA, bool transposeB, const MHO_linalg_matrix<XValueType>& A, const MHO_linalg_matrix<XValueType>& B,
                                         MHO_linalg_matrix<XValueType>& C)
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
        ss << "MHO_linalg_matrix_multiply_with_transpose: error, matrices have different sizes." << "\n";
        ss << "matrix a is " << A.n_rows() << " by " << A.n_cols() << "." << "\n";
        ss << "matrix b is " << B.n_rows() << " by " << B.n_cols() << "." << "\n";
        ss << "matrix c is " << C.n_rows() << " by " << C.n_cols() << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
    }
}

//matrix multiply with transpose which constructs return matrix
template< typename XValueType = double >
MHO_linalg_matrix<XValueType> 
MHO_linalg_matrix_multiply_with_transpose(bool transposeA, bool transposeB, const MHO_linalg_matrix<XValueType>& A, const MHO_linalg_matrix<XValueType>& B)
{
    MHO_linalg_matrix<XValueType> C;
    unsigned int c_row = A.n_rows();
    unsigned int c_col = B.n_cols();
    if(transposeA){c_row = A.n_cols();}
    if(transposeB){c_col = B.n_rows();}
    C.resize(c_row, c_col);
    MHO_linalg_matrix_multiply_with_transpose(transposeA,transposeB,A,B,C);
    return C;
}


template< typename XValueType = double >
void MHO_linalg_matrix_svd(const MHO_linalg_matrix<XValueType>& A, MHO_linalg_matrix<XValueType>& U, MHO_linalg_vector<XValueType>& S, MHO_linalg_matrix<XValueType>& V)
{
    // this function uses the slower but more accurate one-sided jacobi svd
    // as defined in the paper:
    // Jacobi's method is more accurate than QR by J. Demmel and K. Veselic
    // SIAM. J. Matrix Anal. & Appl., 13(4), 1204-1245.
    // www.netlib.org/lapack/lawnspdf/lawn15.pdf
    // assume that A is n x m with n >= m (tall or square); fat matrices (m>n) are not supported
    // then U is an n x m
    // V is m x m
    // S is length m

    unsigned int n = A.n_rows();
    unsigned int m = A.n_cols();
    
    if (n < m)
    {
        std::stringstream ss;
        ss << "MHO_linalg_matrix_svd: error, only tall (n_row >= m_cols) or square matrices supported." << "\n";
        ss << "matrix A is " << n << " by " << m << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
    }
    
    if(U.n_rows() != n || U.n_cols() != m)
    {
        std::stringstream ss;
        ss << "MHO_linalg_matrix_svd: error, matrices A and U have different sizes, will resize U." << "\n";
        ss << "matrix A is " << n << " by " << m << "." << "\n";
        ss << "matrix U was " << U.n_rows() << " by " << U.n_cols() << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
        U.resize(n,m);
    }
    
    if(V.n_rows() != m || V.n_cols() != m)
    {
        std::stringstream ss;
        ss << "MHO_linalg_matrix_svd: error, matrix V has wrong size, will resize." << "\n";
        ss << "matrix V is " << V.n_rows() << " by " << V.n_cols() << "." << "\n";
        ss << "matrix V should be " << m << " by " << m << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
        V.resize(m,m);
    }
    
    if(S.size() != m)
    {
        std::stringstream ss;
        ss << "MHO_linalg_matrix_svd: error, vector S has wrong size, will resize" << "\n";
        ss << "vector S is length " << S.size() << "\n";
        ss << "vector S should be length " << m << "." << "\n";
        report_error(MHO_linalg_error::MismatchedDimension, ss.str() );
        S.resize(m);
    }

    // copy A into U
    U = A;
    // set V to the identity
    V.set_as_identity();
    // put some limits on the number of iterations, this is completely arbitrary
    int n_iter = 0;
    int n_max_iter = 10 * m;
    // scratch space
    XValueType a, b, c, g1, g2, cs, sn, t, psi, sign;
    // make a very rough empirical estimate of the appropriate tolerance
    XValueType tol = 0;
    for(unsigned int i = 0; i < n; i++)
    {
        for(unsigned int j = 0; j < m; j++)
        {
            g1 = U(i, j);
            tol += g1 * g1;
        }
    }
    tol = tol * n * m * std::numeric_limits<XValueType>::epsilon() * std::numeric_limits<XValueType>::epsilon();

    // convergence count
    int count = 0;
    do
    {
        count = 0;
        // for all column pairs i < j < m
        for(unsigned int i = 0; i < m - 1; i++)
        {
            for(unsigned int j = i + 1; j < m; j++)
            {
                // add to the convergence count
                count++;
                // compute the a,b,c submatrix
                a = 0;
                b = 0;
                c = 0;
                for(unsigned int k = 0; k < n; k++)
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
                    t = (sign) / (std::fabs(psi) + std::sqrt(1.0 + psi * psi));
                    cs = 1.0 / std::sqrt(1.0 + t * t);
                    sn = cs * t;
                    // apply the rotation to U and V
                    for(unsigned int k = 0; k < n; k++)
                    {
                        // apply to U
                        g1 = U(k, i);
                        g2 = U(k, j);
                        U(k, i) = cs * g1 - sn * g2;
                        U(k, j) = sn * g1 + cs * g2;
                    }
                    for(unsigned int k = 0; k < m; k++)
                    {
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
            ss << "MHO_linalg_matrix_svd: error, SVD failed to converge within " << n_max_iter << " iterations, " << "\n";
            report_error(MHO_linalg_error::FailedToConverge, ss.str() );
            break;
        }
    }
    while(count > 0);

    // now we compute the singular values, they are the norms of the columns of U
    // we also compute the norm of all the singular values
    XValueType norm_s = 0.0;
    for(unsigned int i = 0; i < m; i++)
    {
        a = 0;
        for(unsigned int j = 0; j < n; j++)
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
    for(unsigned int i = 0; i < m; i++)
    {
        if(S(i) < tol)
        {
            S(i) = 0.0;
        }
    }
    // now we fix U by post multiplying with the inverse of diag(S)
    for(unsigned int i = 0; i < n; i++) // rows
    {
        for(unsigned int j = 0; j < m; j++) // cols
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
void MHO_linalg_matrix_transpose(const MHO_linalg_matrix<XValueType>& in, MHO_linalg_matrix<XValueType>& out)
{

    if((in.n_rows() != out.n_cols()) || (in.n_cols() != out.n_rows()))
    {
        report_error(MHO_linalg_error::MismatchedDimension, "MHO_linalg_matrix_transpose: error, dimension mismatch." );
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
void
MHO_linalg_matrix_svd_solve(const MHO_linalg_matrix<XValueType>& U, const MHO_linalg_vector<XValueType>& S, const MHO_linalg_matrix<XValueType>& V, const MHO_linalg_vector<XValueType>& b, MHO_linalg_vector<XValueType>& x)
{
    //the solution is given by:
    //x = [V*diag(S)^{-1}*U^{T}]b
    //workspace
    MHO_linalg_vector<XValueType> work(x.size());
    //first we apply U^T to b
    x.zero();
    work.zero();
    MHO_linalg_matrix_transpose_vector_product(U, b, work);
    x = work;
    
    //now we apply the inverse of diag(S) to x
    //with the exception that if a singular value is zero, then we apply zero
    //we assume anything less than EPSILON*norm(S) to be essentially zero (singular values should all be positive)
    double s, elem;
    double norm_s = S.norm();
    for(unsigned int i=0; i<S.size(); i++)
    {
        s = S(i);
        if(s > std::numeric_limits<XValueType>::epsilon()*norm_s)
        {
            //multiply 1/s against the i'th element of x
            elem = (1.0/s)*x(i); //MHO_linalg_vector_get(x,i);
            x(i) = elem;
        }
        else
        {
            x(i) = 0; //truncate small s
        }
    }

    //finally we apply the matrix V to the vector x
    MHO_linalg_matrix_vector_product(V, x, work);
    x = work;
}

template< typename XValueType = double >
void MHO_linalg_matrix_print(const MHO_linalg_matrix<XValueType>& m)
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
void MHO_linalg_vector_print(const MHO_linalg_vector<XValueType>& m)
{
    for(unsigned int j = 0; j < m.size() - 1 ; j++)
    {
        std::cout << m(j) << ", ";
    }
    std::cout << m( m.size() - 1 );
    std::cout << std::endl;
}

//build diagonal matrix from a vector
template<typename XValueType>
MHO_linalg_matrix<XValueType> MHO_linalg_diag_matrix(const MHO_linalg_vector<XValueType>& s)
{
    unsigned int n = s.size();
    MHO_linalg_matrix<XValueType> D(n, n);
    D.zero();
    for(unsigned int i = 0; i < n; ++i)
    {
        D(i, i) = s(i);
    }
    return D;
}

//construct the transpose of a matrix
template<typename XValueType>
MHO_linalg_matrix<XValueType> MHO_linalg_transpose_matrix(const MHO_linalg_matrix<XValueType>& A)
{
    MHO_linalg_matrix<XValueType> AT(A.n_cols(), A.n_rows());
    for(unsigned int i = 0; i < A.n_rows(); ++i)
    {
        for(unsigned int j = 0; j < A.n_cols(); ++j)
        {
            AT(j, i) = A(i, j);
        }
    }
    return AT;
}



} //end namespace

#endif /*! end of include guard: MHO_LinearAlgebraUtilities_HH__ */
