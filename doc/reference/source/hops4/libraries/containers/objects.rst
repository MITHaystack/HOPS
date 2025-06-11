.. _sec:objects:

Object specification
====================

Generally speaking the code will be organized around roughly three
object type categories involved in the structure of the new HOPS. These
are as follows:

#. Meta Data Containers: These serve to store small quantities of
   station and baseline metadata associated with an observation of
   disparate types.

#. Array Containers: These serve to organize large n-dimensional arrays
   of a single data type (e.g. visibility data and
   correction/calibration table data).

#. Data Operators: These evaluate a function or perform some
   transformation on a given data container. Their operation is
   configurable via a set of externally defined parameters, while their
   application to any particular data set can be made conditional by a
   set of filters.).

Data Containers
---------------

The existing HOPS3 code base relies on a fixed number of ``C`` structs
to organize and present the data related to an observation. The strict
memory layout of these structures has the advantage of making them
cross-machine compatible, which is necessary since these structures are
also used as the core components of the Mark-4 I/O library. However, a
notable disadvantage of this rigid design is the degree of difficulty
encountered in making changes to the existing data structures, or adding
new data types in order to accommodate additional information which was
not originally envisioned at the time the library was written. To make
the data structures more flexible we intend to decouple the in-memory
data layout from the file I/O, so they do not necessarily need to be
byte-for-byte copies.

Meta-Data Containers
~~~~~~~~~~~~~~~~~~~~

In a strictly typed language such as ``C``, flexible data structures
have a high degree of code overhead, not only in the management of
dynamic memory allocation, but more severely in the conversion of data
types and typecasting. To ameliorate this we propose to exploit
``C++11``\ ’s variadic template mechanism, which allows for the
transformation of type-agnostic class lists into concrete class types or
hierarchies at compile time. This makes it possible to store disparate
types (so long as the complete set of types is known at compile time)
within in the same object that are indexed and can be retrieved by the
same type of key (e.g. a name string).
Listing `[lst:metaobjects] <#lst:metaobjects>`__ gives a condensed
example of the preliminary version of the template base class for a
meta-data container (with detailed functionality removed).

.. code:: c++

   #include <map>

   //implemenation omitted between {}
   template< typename XKeyType, typename XValueType >
   class MHO_SingleTypeMap
   {
       //...impl. omitted...
       private:
           std::map< XKeyType, XValueType > fMap;
   };

   //declare a multi-type map which takes a key type, and variadic template parameter for the types to be stored
   template <typename XKeyType, typename... XValueTypeS>
   class MHO_MultiTypeMap;

   //declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
   template <typename  XKeyType, typename XValueType>
   class MHO_MultiTypeMap< XKeyType, XValueType >: public MHO_SingleTypeMap< XKeyType, XValueType > {};

   //now set up the recursion
   template< typename XKeyType, typename XValueType, typename... XValueTypeS >
   class MHO_MultiTypeMap< XKeyType, XValueType, XValueTypeS...>:
           public MHO_MultiTypeMap< XKeyType, XValueType >,
           public MHO_MultiTypeMap< XKeyType, XValueTypeS... > {};

To the extent possible, the in-memory meta-data structures should be
classes which provide access via a key-value pair mechanism so as to
avoid exposing the private internal storage layout to the routines
needing access to subsets of the data. This retrieval mechanism also has
the benefit of completely decoupling the compile time structure of the
data containers from the data they need to hold at runtime. A key:value
interface is trivially available via the STL std::map template class, so
there is no need to expend effort on a native implementation. Moreover
this sort of interface should also make conversion of these data
structures into widely accessible formats such as JSON or python
dictionaries possible for data export to external software.

N-Dimensional Array Containers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We propose the following basic set of class templates be used to
construct most in-memory objects used for the manipulation of correlated
observation data and its associated station data:

#. ``MHO_ScalarContainer`` - encapsulates scalar-like data

#. ``MHO_VectorContainer`` - encapsulates vector-like data

#. ``MHO_TableContainer`` - encapsulates rank-N tensor-like data with
   associated axes (vector)

These template classes are to serve as a simple wrapper around the
management of the raw memory needed to store a data item and keep track
of its associated unit(s), and (if applicable) the values associated
with the axes along each dimension and their units.

The three container types listed above represent the majority of the
memory intensive data needed within HOPS and can largely be derived from
a N-dimensional array of some numeric or integral type. Therefore a
generic template class for N-dimensional arrays is needed, listing
`[lst:ndarray] <#lst:ndarray>`__ shows a stub of this class.

.. code:: c++

   template< typename XValueType, std::size_t RANK>
   class MHO_NDArrayWrapper: public MHO_ExtensibleElement
   {
       public:
           using value_type = XValueType;
           typedef std::integral_constant< std::size_t, RANK > rank;

           MHO_NDArrayWrapper();
   	virtual ~MHO_NDArrayWrapper();

   	class iterator {...};
   	class strided_iterator {...};

       protected:
           XValueType* fDataPtr;
           bool fExternallyManaged;
           std::vector< XValueType > fData; //internally managed data
           std::size_t fDimensions[RANK]; //size of each dimension
           std::size_t fTotalArraySize; //total size of array
   };

The underlying storage of the N-dimensional array data is done as a
single contiguous chunk of memory which can either be a piece of
externally or internally managed memory. Indexing into this chunk of
memory is done using C-like row-major order, where for an array of rank
D, with dimension sizes :math:`\{N_0, N_1, \cdots N_{D-1}\}`, the
location of the data specified by the indexes
:math:`\{n_0, n_1, \cdots, n_{D-1}\}` can found at an offset from the
start, :math:`z`, that is given by:

.. math:: z = \sum_{k=0}^{D-1} \left ( \prod_{j=k+1}^{D-1} N_j \right) n_k

Access to the underlying data stored within a class of this type can
then proceed in two main ways. The first is through the aforementioned
row-major order indexing operation, and the second is through the use of
iterators. An example of several of the provided methods is shown for a
three dimensional array in listing
`[lst:array-usage] <#lst:array-usage>`__. Iterators are most commonly
utilized for efficient incremental (continuous or strided) access to the
array data as they can be computed using pointer arithmetic, while
random access is best done via indexes.

.. code:: c++

   MHO_NDArrayWrapper< double, 3> ex;
   //declare the dimensions of the 3d array
   ex.Resize(10,10,10);

   //access via un-checked index tuples
   ex(0,0,0) = 1;
   //access via bounds-checked index tuples
   ex.at(9,9,9) = 1;
   //access via un-checked offset from start 
   ex[999] = 1;
   //access to underlying raw memory
   double* ptr = ex.GetData();
   ptr[3] = 1;
   //access via iterator 
   auto it = ex.begin();
   *it = 1;
   //skip-by-10 access via strided iterator 
   auto sit = ex.stride_begin(10)
   *(++sit) = 1;  //access to ex(0,1,0);

In addition to the raw data stored in the N-dimensional array, in the
case of the ``MHO_TableContainer`` it is important to associate a
coordinate axis with each dimension in order to provide various data
operators with the ability to look-up the location of a datum beyond a
simple integer-index. To enable this, we will pair an N-dimensional
array with a tuple of axis objects associated with each dimension.
Listing `[lst:objects] <#lst:objects>`__ shows the template class
structure for a TableContainer (along with other) objects.

.. code:: c++

   template< typename XValueType >
   class MHO_ScalarContainer: public MHO_NDArrayWrapper< XValueType, 0>, public MHO_Named {};

   template< typename XValueType >
   class MHO_VectorContainer: public MHO_NDArrayWrapper< XValueType, 1>, public MHO_Named{};

   template< typename XValueType >
   class MHO_Axis: public MHO_VectorContainer< XValueType >, public MHO_IntervalLabelTree {};

   template< typename...XAxisTypeS >
   class MHO_AxisPack:  public std::tuple< XAxisTypeS... > {};

   template< typename XValueType, typename XAxisPackType >
   class MHO_TableContainer: public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>,
                             public XAxisPackType, public MHO_Named {};

The axis objects themselves also inherit from ``MHO_IntervalLabelTree``,
which provides the ability to associate a pair of indexes with a key
value pair of several types. A simple example of this would be tagging a
section of the frequency axis with a particular channel ID (e.g
``[0, 32]`` :math:`\leftrightarrow` ``{"channel_id": "X17LY"}``). The
class ``MHO_IntervalLabelTree`` will support tagging axis intervals with
values of at least the following types: char, bool, int, double, and
string, using strings as keys, and allow for the bi-directional look up
of a key:value pairs with assoiated intervals.

As a concrete example of the ``MHO_TableContainer`` template class,
listing `[lst:visib] <#lst:visib>`__ gives a simple example of what
template declaration of an object storing channelized visibility data
from a single-baseline observation might look like.

.. code:: c++

   //definitions of a channelized set of single-baseline visbility data
   using visibility_type = std::complex<double>;

   using ch_polprod_axis_type = MHO_Axis<std::string>;
   using ch_channel_axis_type = MHO_Axis<int>; //channels are simply numbered here
   using ch_frequency_axis_type = MHO_Axis<double>;
   using ch_time_axis_type = MHO_Axis<double>;

   using ch_baseline_axis_pack = MHO_AxisPack< ch_polprod_axis_type, ch_channel_axis_type, ch_time_axis_type, ch_frequency_axis_type >;
   using ch_baseline_data_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;
   using ch_baseline_weight_type = MHO_TableContainer< weight_type, ch_baseline_axis_pack >;
   using ch_baseline_sbd_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;

For example, in the case of channelized visibilities, the axes of each
of the four dimensions would be:

#. Axis 0: Polarization-product axis, labelled by a short string
   specifying the reference and remote stations’ polarizations for data
   associated with that column (e.g “XX” or “RR” or “RX”).

#. Axis 1: Channel axis, labelled by a character or numerical value
   (e.g. “A” or 1).

#. Axis 2: Time axis, labelled by the time since start of a scan in
   seconds.

#. Axis 3: Frequency axis, labelled by the frequency offset from the
   edge of the channel (MHz).

It should be noted that these coordinate axes are there merely to label
the data, but are not meant to provide a reverse look-up capability,
(e.g example inverting the polarization-product code “LL” to infer a
0-th index location of 0). For efficiency array access should still be
done using unsigned integer index values. A graphical representation of
a ``MHO_TableContainer`` is shown in
Figure `1 <#fig:table-container>`__.

.. figure:: fig/data-container-baseline.png
   :alt: A graphical representation of a ``MHO_TableContainer``. This
   class is composed of an N-dimensional array, coupled with axes to
   provide coordinate values along each dimension. The axes themselves
   allow for arbitrary intervals to be labelled by key:value pairs in
   order to allow for local look-up of filter data. For example, along
   the frequency axis, the interval labels may be channel or sampler
   names among other possibilities. Furthermore, the interval and
   associated labels will be stored in an interval-tree structure to
   allow for fast bi-directional lookup of data indices
   :math:`\leftrightarrow` data labels.
   :name: fig:table-container
   :width: 75.0%

   A graphical representation of a ``MHO_TableContainer``. This class is
   composed of an N-dimensional array, coupled with axes to provide
   coordinate values along each dimension. The axes themselves allow for
   arbitrary intervals to be labelled by key:value pairs in order to
   allow for local look-up of filter data. For example, along the
   frequency axis, the interval labels may be channel or sampler names
   among other possibilities. Furthermore, the interval and associated
   labels will be stored in an interval-tree structure to allow for fast
   bi-directional lookup of data indices :math:`\leftrightarrow` data
   labels.

Specific data types
~~~~~~~~~~~~~~~~~~~

Below is an incomplete table (`[tab:data-types] <#tab:data-types>`__) of
the various data objects that are constructed from
``MHO_TableContainer``, along with their data value type and axis names.
These may be subject to change.


Output data objects
-------------------

The output data types of the fringe-fitter need to be able to summarize
all of the information that is currently present in the ``type_2XX``
data types, with the option to be extended. The ``type_2XX``\ ’s are
primarily concerned with storing the fringe solution (delay, delay rate,
etc.), and a data summary (per-channel phase/amplitudes, data
selection/flagging, etc.) along with a subset of station meta data. A
table listing the data items that are present in the ``type_2XX``\ ’s
which need to be re-mapped onto the new data structure is shown in table
`[tab:type2xx] <#tab:type2xx>`__.

Data Container Extensions
~~~~~~~~~~~~~~~~~~~~~~~~~

The primary goal of the containers is to provide a relatively simple and
efficient representation of commonly used data types that hides the
details of memory management and array indexing/access from the user.
They should not be overburdened with too much extraneous functionality
(beyond simple operator overloads like assignment, scalar multiply, etc.
) that is specific to a particular operation as this greatly over
complicates these classes and makes them brittle.

However, there are some cases where this sort of decoupling may induce a
performance cost. An example of this occurs in the case of SIMD/GPU
acceleration. In order to make use of GPU processing the data must be
copied to a buffer on the device, processed, and then the results must
be passed back to the host. However, if there are several operations to
be performed in succession on the GPU, only first and last transfer need
to occur, with intervening transfers being unecessary as input data is
already present on the device. However, in order to eliminate the
intermediate transfers a handle to the device buffer must be kept
persistent in memory. So the questions arises, where should we keep this
device buffer object? Should it be kept as a member of the data
operator? That would be a poor choice, since if it is private it will
not accesible to other operators to make use of, and if it is public
then it will introduce the possibility of tight coupling with other
portions of the code making use of the buffer. On the other hand, a
pointer to a device buffer is too specific to belong in something as
basic as a data container. However, it is a good candidate for something
to may be stored in an extension.

In order to provide the ability to append extensions to the data
containers, they must all inherit from a base class,
``MHO_ExtensbleElement``, which in turn stores a vector of
type-erased [1]_ pointers to the extensions themselves. The extensions
are templated on the the class providing the additional functionality
and must all inherit from the base class ``MHO_ExtendedElement`` (so
they can be stored in the vector owned ``MHO_ExtensbleElement``) A brief
sketch of the code that allows for this is shown in listing
`[lst:extend] <#lst:extend>`__. One draw back of this method is that
requires :math:`N` ``dynamic_cast`` calls any time a particular
extension is modified or accessed via the data container. This is an
acceptable trade off for infrequent access to expensive (to construct)
extensions, but should be used rather sparingly as ``dynamic_cast`` has
high overhead.

.. code:: c++

   #include <vector>

   //forward declare these types
   class MHO_Element;
   class MHO_ExtensibleElement;
   template<class XExtensionType> class MHO_ExtendedElement;

   class MHO_Element{
       public:
           MHO_Element();
           virtual ~MHO_Element();
   };

   class MHO_ExtensibleElement{
       public:
           MHO_ExtensibleElement();
           virtual ~MHO_ExtensibleElement();
           template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* MakeExtension();
           template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* AsExtension();

       protected:
           std::vector< MHO_Element* > fExtensions;
   };

   template<class XExtensionType>
   inline MHO_ExtendedElement<XExtensionType>*
   MHO_ExtensibleElement::MakeExtension()
   {
       MHO_ExtendedElement<XExtensionType>* extention;
       for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
       {
           extention = dynamic_cast<MHO_ExtendedElement<XExtensionType>*>( *it );
           if(extention != nullptr){delete extention; fExtensions.erase(it); break; }
       }
       extention = new MHO_ExtendedElement<XExtensionType>(this);
       fExtensions.push_back(extention);
       return extention;
   }

   template<class XExtensionType>
   inline MHO_ExtendedElement<XExtensionType>*
   MHO_ExtensibleElement::AsExtension()
   {
       MHO_ExtendedElement<XExtensionType>* extention;
       for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
       {
           extention = dynamic_cast<MHO_ExtendedElement<XExtensionType>*>( *it );
           if (extention != nullptr){return extention;};
       }
       return nullptr;
   }

   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////

   template<class XExtensionType >
   class MHO_ExtendedElement: public MHO_Element, public XExtensionType
   {
       public:
           MHO_ExtendedElement(MHO_ExtensibleElement* parent):
               XExtensionType(parent),
               fParent(parent)
           {};
           virtual ~MHO_ExtendedElement(){};

       protected:

           MHO_ExtensibleElement* fParent;
   };

.. [1]
   https://davekilian.com/cpp-type-erasure.html
