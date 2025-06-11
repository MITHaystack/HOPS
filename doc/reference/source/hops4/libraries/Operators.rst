..  _Operators:

Operators
=========



Data operators
--------------

The data operator classes are meant to organize the mathematic
manipulations which are to be performed on the data containers. For
example, many of the operations performed in the existing HOPS3
code-base (such as the application of a priori phase calibration) are
relatively trivial linear transformations applied to the visibility
data. However, they are currently intertwined with a large amount of
control logic which obscures the basic data pathway (e.g see
postproc/fourfit/norm_fx.c)

Most unary or binary operations that are to applied to visibility or
other data residing in an ``HO_TableContainers`` such as scaling,
multiplication, transposition, summation, Fourier transformation, etc.
will be made available as individual classes inheriting from the same
interface. A uniform class interface will allow these data operators to
be composed or modified to create more complicated composite operators
or strung together and called in an ordered fashion in order to
accomplish data pipelines of arbitrary complexity. An additional
advantage of encapsulating individual operations is that (coupled with
the data container extensions) any SIMD parallel-processing extension
used to accelerate data processing can be made opaque to the user.

Listing `[lst:operators] <#lst:operators>`__ gives a brief sketch of the
class templates generalizing the data operators. The common inheritance
from the base class ``MHO_Operator`` allows them all to be stored in an
common container (e.g. ``std::vector<MHO_Operator*>``) so that once they
are constructed and configured they may be retrieved and
intialized/executed in the appropriate order. Listing
`[lst:operator-use] <#lst:operator-use>`__ shows a brief code sketch
demonstrating how two simple operators would be constructed, assigned
arguments and then initialized and executed in order.

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

It is expected that the vast majority of the data operators will be
unary or binary, requiring only their own configuration parameters along
with one or two data containers upon which they operate as inputs.
However, any number of arguments is possible so long as the underlying
implementation provides the appropriate overload.

One aspect of the data operators which is not yet detailed here is a
notion of what pieces of meta-data each operator may need in order to
complete its function. Some of the more primitive operations (e.g.
complex conjugation) may not need any meta-data, while certain specific
calibration routines may need station related meta-data (e.g. channel
specific phase-cal). While some meta-data items could be exposed
directly via external setter/getters, a possibly preferable option which
would preserve encapsulation might be for each operator to define an
internal schema, listing the keys and type of the parameters it needs to
retrieve from a single meta-data container (populated from the vex), or
what sort of labels it expects to be attached to the data containers on
which it operates. In addition, a mechanism for filtering operations
(e.g. if station = Xx, then apply this operator) also needs to be
established independent of the previous control-block structure of
HOPS3.

.. code:: c++

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

Specific data operations
~~~~~~~~~~~~~~~~~~~~~~~~

Below is an incomplete list of various data operations. A full
specification of each operation is detailed in the subsequent pages.

#. MHO_ComplexConjugator: Apply a complex conjugation to all elements of
   an ND-array.

#. MHO_CyclicRotator: Apply a cyclic rotation to the selected axes of an
   ND-array.

#. MHO_FastFourierTransform: Apply a Fourier transform to a one
   dimensional array.

#. MHO_FunctorBroadcaster: Apply a specified unary function to each
   element of an ND-array.

#. MHO_MultidimensionalFastFourierTransform: Apply a Fourier transform
   to the selected axes of an ND-array using native libary.

#. MHO_MultidimensionalFastFourierTransformFFTW: Apply a Fourier
   transform to the selected axes of an ND-array using FFTW library.

#. MHO_MultidimensionalPaddedFastFourierTransform: Apply a zero-padded
   Fourier transform to the selected axes of an ND-array.

#. MHO_Reducer: Apply a reduction (e.g. sum all elements) along the
   selected axis of an ND-array.

#. MHO_SubSample: Skip select every n-th element of an ND-array for a
   specified axis of a ND-array.

| **Name:** MHO_ComplexConjugator
| **Type:** Unary, in-place and out-of-place (requires copy).
| **Template Parameters:** The specific N dimensional array type.
| **Configuration Parameters:** None.
| **Inputs:** A N dimensional array with complex double/float value
  type.
| **Outputs:** A N dimensional array with complex double/float value
  type.
| **Description:** Iterates over all values in an N dimensional array
  and applies the operation ``std::conj()`` to each element, according
  to algorithm `[algo:complex-conjugator] <#algo:complex-conjugator>`__.

[algo:complex-conjugator]

| **Name:** MHO_CyclicRotator
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** Requires the integer index of the axis
  to be rotated, and the integer offset specifying the size of the
  rotation. A positive value of the rotation offset results in a right
  shift cyclic rotation, while a negative value results in a left shift
  cyclic rotation.
| **Inputs:** A N dimensional array with arbitrary trivially copyable
  type.
| **Outputs:** A N dimensional array with arbitrary trivially copyable
  type.
| **Description:** Performs cyclic rotation upon the requested axis for
  the specified offset, according to algorithm
  `[algo:cyclic-rot] <#algo:cyclic-rot>`__.

[algo:cyclic-rot]

| **Name:** MHO_FastFourierTransform
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** Requires the direction of the transform
  to be specified (forward/backward), the direction follows the
  convention of FFTW.
| **Inputs:** A one dimensional array with complex double/float value
  type.
| **Outputs:** A one dimensional array with complex double/float value
  type.
| **Description:** This operator performs an Fourier transform (or
  inverse transform) on the input array using an FFT algorithm. If the
  array size is a power of two, then either a Cooley-Tukey or
  Gentleman-Sande radix-2 algorithm will be applied. For all other
  sizes, the Bluestein/Chirp-Z algorithm is used.
| **Name:** MHO_FunctorBroadcaster
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** The unary functor class to be applied to
  each element of the array (this is a template parameter).
| **Inputs:** A N dimensional array with any value type (must be
  acceptable to the functor)
| **Outputs:** A N dimensional array with any value type (must be
  acceptable to the functor)
| **Description:** For every element in the array the functor operation
  will be applied. In the case of an out-of-place operation a copy will
  take place.
| **Name:** MHO_MultidimensionalFastFourierTransform
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** The indices of the dimensions which are
  to undergo transformation(default is all), as well as direction of the
  transform to be specified (forward/backward), the direction follows
  the convention of FFTW.
| **Inputs:** A N dimensional array with complex double/float value type
| **Outputs:** A N dimensional array with complex double/float value
  type
| **Description:** Executes a Fourier transform on the selected
  dimensions of the array using the native FFT calculator.
| **Name:** MHO_MultidimensionalFastFourierTransformFFTW
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** The indices of the dimensions which are
  to undergo transformation (default is all), as well as direction of
  the transform to be specified (forward/backward), the direction
  follows the convention of FFTW.
| **Inputs:** A N dimensional array with complex double/float value type
| **Outputs:** A N dimensional array with complex double/float value
  type
| **Description:** Executes a Fourier transform on the selected
  dimensions of the array using the FFTW library, the precise algorithm
  selected is determined by FFTW.
| **Name:** MHO_MultidimensionalPaddedFastFourierTransform
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** The indices of the dimensions which are
  to undergo transformation (default is all), the padding factor
  :math:`M` and the direction of the transform (forward/backward). The
  zero padding can be specified as either symmetrically center padded
  (zeros place in middle of array), or end-padded.
| **Inputs:** A N dimensional array with complex double/float value type
  with even lengths in each dimension to be transformed.
| **Outputs:** A N dimensional array with complex double/float value
  type with even lengths in each dimension to be transformed.
| **Description:** For each selected dimension of length :math:`n`, the
  array will be padded with zeros, such that the new length will be
  :math:`nM`. The zeros will either be placed in the center of the
  re-sized array (center-padded), or at the end (end-padded). The
  resulting padded array will then be transformed using the native FFT
  calculator. The primary use case of this padded FFT is for
  interpolation.
| **Name:** MHO_Reducer
| **Type:** Unary, both in-place (requires copy and resize) and
  out-of-place.
| **Configuration Parameters:** The indices of the dimensions which are
  to undergo reduction, and the operation which is to execute the
  reduction (addition or multiplication).
| **Inputs:** A N dimensional array with numerical value type
| **Outputs:** A N dimensional array with numerical value type
| **Description:** The input array will be reduced along the selected
  axes, and depending on the operation (addition or multiplication), the
  contents will be resized and replaced by the sum or product of the
  elements along that axis.
| **Name:** MHO_SubSample
| **Type:** Unary, both in-place and out-of-place.
| **Configuration Parameters:** The index of the dimension along which
  the sub-sampling operation should take place, and the stride at which
  elements are re-sampled.
| **Inputs:** A N dimensional array with any value type
| **Outputs:** A N dimensional array with any value type
| **Description:** For a stride value of :math:`k`, and dimension index
  :math:`j`, the output array will be resized and populated in such a
  way that only every :math:`k`-th element (along the :math:`j`-th
  dimension) from the original array will remain.

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
