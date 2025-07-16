..  _Operators:

Operators
=========

The data operator classes are meant to organize the various mathematic
manipulations that are performed on the data containers. For
example, many of the operations performed in the existing HOPS3
code-base (such as the application of *a priori* phase calibration) are
relatively trivial linear transformations applied to the visibility
data. However, in the HOPS3 code base these simple operations have 
become intertwined with a large amount of control logic which obscures 
the basic data pathway (e.g see source/c_src/fourfit_libs/ffsearch/src/norm_fx.c)

Most unary or binary operations that are to applied to visibility or
other data residing in a ``MHO_TableContainers`` object such as scaling,
multiplication, transposition, summation, Fourier transformation, etc.
have in HOPS4 been reorganized into individual classes inheriting from the same
operator interface. A uniform class interface allows these data operators to
be composed or modified to create more complicated composite operators
or strung together and called in an ordered fashion in order to
accomplish data pipelines of arbitrary complexity. An additional
advantage of encapsulating individual operations is that (coupled with
the data container extensions) most SIMD parallel-processing extensions 
(e.g. CUDA/OpenCL plugins) used to accelerate data processing can be made 
opaque to the caller.

The following code gives a brief sketch of the class templates generalizing 
the data operators.

.. code:: cpp

   class MHO_Operator
   {
       public:
           MHO_Operator();
           virtual ~MHO_Operator();
           virtual bool Initialize() = 0;
           virtual bool Execute() = 0;
   };

   template<class XArgType>
   class MHO_UnaryOperator: public MHO_Operator
   {
       public:
           MHO_UnaryOperator();
           virtual ~MHO_UnaryOperator();
           virtual void SetArgs(XArgType* in); //in-place
           virtual void SetArgs(const XArgType* in, XArgType* out); //out-of-place
   };

   template<class XArgType1, class XArgType2 = XArgType1, class XArgType3 = XArgType2>
   class MHO_BinaryOperator: public MHO_Operator
   {
      public:
          MHO_BinaryOperator();
          virtual ~MHO_BinaryOperator();
          virtual void SetArgs(const XArgType1* in1, const XArgType2* in2, XArgType3* out) //out-of-place
   };


The common inheritance from the base class :hops:`hops::MHO_Operator` allows 
operators of various types to all be stored in an common container (
e.g. ``std::vector<MHO_Operator*>``) so that once they are constructed they may 
be retrieved and intialized/executed in the appropriate order. The following code 
shows a brief code sketch demonstrating how two simple operators would be 
constructed, assigned arguments, initialized, and then executed in order.

.. code:: c++

   //prexisting pointers to a particular data container
   //data_type* array1;
   //data_type* array2;

   //storage for an ordered list of operators
   std::vector< MHO_Operator* > operators;

   //construct and configure two operations, and insert in list
   MHO_ScalarMultiply mult;
   mult.SetFactor(2);
   mult.SetArgs(array1);

   MHO_SubSample sub;
   sub.SetDimensionStride(0, 4);
   sub.SetArgs(array1, array2);

   operators.push_back(&mult);
   operators.push_back(&sub);

   //chained initialization of the operators
   for(auto it = operators.begin(); it != operators.end(); it++)
   {
       it->Initialize();
   }

   { //some possible outer loop here...

       //chained execution of the operators
       for(auto it = operators.begin(); it != operators.end(); it++)
       {
           it->Execute();
       }

   }
   //array2 now contains the contents of array1, multiplied by 2
   //but sampled only every 4th element in the 0-th dimension

The vast majority of the data operators are either unary or binary. Unary 
operators accepting either single input data container (to operate in-place), or
both an input and output data container (of the same type) to operate out-of-place.
Binary operators require two input data containers, and a single output data 
container (all not necessarily of the same type) to carry out an
out-of-place execution. There are two additional categories of operator beyond unary/binary.
These are 'transforming' and 'inspecting' operators. Transforming operators are 
specific form of unary operator which accept an input and output data container, that 
are not of the same type, and change the input into the output. While, inspecting 
operators do not modifying the input data container at all, but are used to compute
some information from it (e.g. finding a maximum in an array).
While the aforementioned cases cover all of the use cases in HOPS4, the operator 
interface is very generic and for future use cases any number of input/output 
arguments is possible, as long as the underlying implementation provides the 
appropriate overloads.


Specific data operations
~~~~~~~~~~~~~~~~~~~~~~~~

Below is an incomplete list of various data operations. A full
specification of each operation is detailed in the subsequent pages.


MHO_ComplexConjugator
---------------------

========================== ====================================================================
Class                      :hops:`MHO_ComplexConjugator`
Operator Type              Unary
Template Parameters        A N dimensional array type
Argument Data Type         N dimensional array with complex double/float value type
========================== ====================================================================

Iterates over all values in an N dimensional array and applies the operation 
``std::conj()`` to each element. This is a template class supporting both in-place and 
out-of-place operations.

MHO_CyclicRotator
-----------------

========================== ====================================================================
Class                      :hops:`MHO_CyclicRotator`
Operator Type              Unary
Template Parameters        The specific N dimensional array type
Configuration Parameters   | Integer index of axis to rotate and integer offset 
                           | for rotation size
Argument Data Type         N dimensional array with arbitrary trivially copyable type
========================== ====================================================================

Performs cyclic rotation upon the requested axis for the specified offset. 
A positive value of the rotation offset results in a right shift cyclic 
rotation, while a negative value results in a left shift cyclic rotation.
This is a template class supporting both in-place and out-of-place operations.

MHO_FastFourierTransform
------------------------

========================== ====================================================================
Class                      :hops:`MHO_FastFourierTransform`
Operator Type              Unary
Template Parameters        XFloatType (floating point type)
Configuration Parameters   | Direction of transform (forward/backward, follows FFTW 
                           | convention)
Argument Data Type         One dimensional array with complex double/float value type
========================== ====================================================================

Performs a Fourier transform (or inverse transform) on the input array using 
an FFT algorithm. If the array size is a power of two, then either a Cooley-Tukey 
or Gentleman-Sande radix-2 algorithm will be applied. For all other sizes, the 
Bluestein/Chirp-Z algorithm is used. This is a template class supporting both in-place and 
out-of-place operations.

MHO_FunctorBroadcaster
----------------------

========================== ====================================================================
Class                      :hops:`MHO_FunctorBroadcaster`
Operator Type              Unary
Template Parameters        XArrayType (array type), XFunctorType (functor type)
Configuration Parameters   The unary functor class to be applied to each element
Argument Data Type         | N dimensional array with any value type (must be 
                           | acceptable to functor)
========================== ====================================================================

For every element in the array the functor operation will be applied. In the 
case of an out-of-place operation a copy will take place. This is a template class 
supporting both in-place and out-of-place operations.


MHO_MultidimensionalFastFourierTransform
----------------------------------------

========================== ====================================================================
Class                      :hops:`MHO_MultidimensionalFastFourierTransform`
Operator Type              Unary
Template Parameters        XArgType (N dimensional array type)
Configuration Parameters   | Indices of dimensions to transform (default all), 
                           | direction (forward/backward)
Argument Data Type         N dimensional array with complex double/float value type
========================== ====================================================================

Executes a Fourier transform on the selected dimensions of the array using 
the native FFT calculator. The direction follows the convention of FFTW.
This is a template class supporting both in-place and out-of-place operations.

MHO_MultidimensionalFastFourierTransformFFTW
--------------------------------------------

========================== ====================================================================
Class                      :hops:`MHO_MultidimensionalFastFourierTransformFFTW`
Operator Type              Unary
Template Parameters        XArgType (N dimensional array type)
Configuration Parameters   | Indices of dimensions to transform (default all), 
                           | direction (forward/backward)
Argument Data Type         N dimensional array with complex double/float value type
========================== ====================================================================

Executes a Fourier transform on the selected dimensions of the array using 
the FFTW library. The precise algorithm selected is determined by FFTW. The 
direction follows the convention of FFTW. This is a template class supporting both 
in-place and out-of-place operations.


MHO_Reducer
-----------

========================== ====================================================================
Class                      :hops:`MHO_Reducer`
Operator Type              Unary
Template Parameters        XArrayType (array type), XFunctorType (reduction operation)
Configuration Parameters   | Indices of dimensions to reduce, operation type 
                           | (addition/multiplication)
Argument Data Type         N dimensional array with numerical value type
========================== ====================================================================

The input array will be reduced along the selected axes, and depending on 
the operation (addition or multiplication), the contents will be resized 
and replaced by the sum or product of the elements along that axis. Template 
class supporting both in-place (requires copy and resize) and out-of-place 
operations.


MHO_SubSample
-------------

========================== ====================================================================
Class                      :hops:`MHO_SubSample`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   | Dimension index for sub-sampling, stride for element 
                           | selection
Argument Data Type         N dimensional array with any value type
========================== ====================================================================

For a stride value of k, and dimension index j, the output array will be 
resized and populated in such a way that only every k-th element (along the 
j-th dimension) from the original array will remain. This is a template class supporting 
both in-place and out-of-place operations.

MHO_AbsoluteValue
-----------------

========================== ====================================================================
Class                      :hops:`MHO_AbsoluteValue`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   None
Argument Data Type         N dimensional array with numerical value type
========================== ====================================================================

Applies the absolute value operation to every element of the input array.
This is a template class supporting both in-place and out-of-place operations.

MHO_ElementTypeCaster
---------------------

========================== ====================================================================
Class                      :hops:`MHO_ElementTypeCaster`
Operator Type              Unary
Template Parameters        XArrayType (array type), XNewType (target type)
Configuration Parameters   None
Argument Data Type         N dimensional array with any value type
========================== ====================================================================

Casts the elements of the input array from one type to another. Template 
class supporting both in-place and out-of-place operations.

MHO_EndZeroPadder
-----------------

========================== ====================================================================
Class                      :hops:`MHO_EndZeroPadder`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   Dimension index and padding size
Argument Data Type         N dimensional array with numerical value type
========================== ====================================================================

Pads the end of the specified dimension with zeros. This is a template class supporting 
both in-place and out-of-place operations.

MHO_ExtremaSearch
-----------------

========================== ====================================================================
Class                      :hops:`MHO_ExtremaSearch`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   Search type (minimum/maximum)
Argument Data Type         N dimensional array with numerical value type
========================== ====================================================================

Searches for the minimum or maximum value in the input array and returns 
the value and its location. This is a template class for inspecting array contents.

MHO_NaNMasker
-------------

========================== ====================================================================
Class                      :hops:`MHO_NaNMasker`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   Mask value to replace NaN with
Argument Data Type         N dimensional array with floating point value type
========================== ====================================================================

Replaces NaN values in the input array with a specified mask value. Template 
class supporting both in-place and out-of-place operations.

MHO_SelectRepack
----------------

========================== ====================================================================
Class                      :hops:`MHO_SelectRepack`
Operator Type              Unary
Template Parameters        XArrayType (array type)
Configuration Parameters   Selection criteria and packing parameters
Argument Data Type         N dimensional array with any value type
========================== ====================================================================

Selects elements from the input array based on specified criteria and repacks 
them into a new array structure. This is a template class supporting data reorganization.

Compound data operations
~~~~~~~~~~~~~~~~~~~~~~~~

On their own each of the specific data operations listed in the previous
section are of limited utility. However, they can be composed to produce
more useful manipulations of the data (e.g. ``norm_fx.c``). The
advantage of composing complex operations via a series of simple
operators is that more fine grained testing can be done at each sub-step
to ensure it is operating correctly without involving the much more
complicated process.

Let us consider the data manipulation done by the fourfit function
``norm_fx.c``. This function is responsible for a large number of
changes to the data, but at its core is largely concerned with
transforming the visibility data from frequency-space to delay-space, so
that a peak in delay-space can be found. However, in the process of
executing this function, several other modifications are introduced to
the data, such as: the application of phase and delay calibration
corrections, the summation of the visibilities of different
polarization-products, the application of delta-parallatic angle
corrections, and the excision of data due to low correlator weights or
ad-hoc flagging. A brief sketch of the operations performed on the set
of visibilties by ``norm_fx.c`` is summarized with minimal detail below.

[algo:normfx]

Once ``norm_fx.c`` has been applied to the visibility data, what was
originally the frequency axis of the input array, is now the
(single-band) delay axis, and a search function to locate the maximum
delay value can be executed. Once a maximum is found, an additional
interpolation step is executed to fine tune the delay value.

Following the application of ``norm_fx.c``, the resulting output data
can then be Fourier transformed along the time-axis, in order to search
for the maximum delay rate. This process is handled by the fourfit
function ``delay_rate.c``.
