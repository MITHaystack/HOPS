.. _sec:objects:

Data Object Specification
=========================

Generally speaking the data in HOPS4 is organized around roughly two
object categories. These are as follows:

#. Meta Data Containers: These serve to store small quantities of
   station and baseline meta-data associated with an observation. The types 
   of which are typically heterogeneous and may have a nested structure.
 
#. Array/Table Containers: These serve to organize large n-dimensional arrays
   of a single data type (e.g. complex visibility data and correction/calibration table data).


   
Meta-Data Containers
~~~~~~~~~~~~~~~~~~~~

To store high structured heterogeneous metadata we have chosen to exploit
the nlohmann::json library which makes it possible to store disparate
types within in the same object and which can be retrieved by the
same type of key (e.g. a name string). The process of insertion of values into
json objects is relatively trivial, however it should be noted that at
the time of retrieval, the object type must be known in order for a proper cast 
to be done before the data can be used directly.

Since we would like to avoid tight coupling to external libraries (in case they 
need to be replaced), we have chosen to wrap most of the interaction with the 
nlohmann::json library through the classes :hops:`hops::MHO_JSONWrapper` and 
:hops:`hops::MHO_ObjectTags`. We have also chosen to capture a version of this 
single-header library under the `extern` directory, so it can be distributed along 
with the rest of the HOPS4 source code. 


N-Dimensional Array Containers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following basic set of class templates can used to
construct most in-memory objects used for the manipulation of correlated
observation data and its associated station data:

#. :hops:`hops::MHO_ScalarContainer`.- encapsulates scalar-like data

#. :hops:`hops::MHO_VectorContainer` - encapsulates vector-like data

#. :hops:`hops::MHO_Axis` -  encapsulates vector-like data to be used as a coordinate axis

#. :hops:`hops::MHO_AxisPack` -  encapsulates a collection of axes

#. :hops:`hops::MHO_TableContainer` - encapsulates rank-N tensor-like data with
   associated axes collection (:hops:`hops::MHO_AxisPack`)

These template classes are to serve as a simple wrapper around the
management of the raw memory needed to store a data item and keep track
of its associated unit(s), and the values associated
with the coordinate axes along each dimension and their units. Additional 
metadata `tags` can be attached to these objects as well.

The container types listed above represent the majority of the
memory intensive data needed within HOPS and can largely be derived from
a N-dimensional array of some numeric or integral type. Therefore a
generic template class for N-dimensional arrays is needed. This is 
implemented by the template class :hops:`hops::MHO_NDArrayWrapper`. The following 
shows a stub of this class to demonstrate its structure:

.. code:: c++

   template< typename XValueType, std::size_t RANK>
   class MHO_NDArrayWrapper: public MHO_ExtensibleElement
   {
       public:
           using value_type = XValueType;
           typedef std::integral_constant< std::size_t, RANK > rank;

           MHO_NDArrayWrapper();
   	virtual ~MHO_NDArrayWrapper();

    //access operator
    template< typename... XIndexTypeS >
    typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type operator()(XIndexTypeS... idx){...};

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
three dimensional array in the following code snippet. Iterators are most commonly
utilized for efficient incremental (continuous or strided) access to the
array data as they can be computed using pointer arithmetic, while
random access is best done via indexes using the operator().

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
case of the :hops:`hops::MHO_TableContainer` it is important to associate a
coordinate axis with each dimension in order to provide various data
operators with the ability to look-up the location of a datum beyond a
simple integer-index. To enable this, we pair an N-dimensional
array with a tuple of axis objects (:hops:`hops::MHO_AxisPack`) associated with each dimension.
The following code shows an outline of the template class
structure for a :hops:`hops::MHO_TableContainer` (along with other) objects.

.. code:: c++

   template< typename XValueType >
   class MHO_ScalarContainer: public MHO_ScalarContainerBase, public MHO_NDArrayWrapper< XValueType, 0 >, public MHO_Taggable

   template< typename XValueType >
   class MHO_VectorContainer: public MHO_VectorContainerBase, public MHO_NDArrayWrapper< XValueType, 1 >, public MHO_Taggable

   template< typename XValueType >
   class MHO_Axis: public MHO_AxisBase,
                   public MHO_VectorContainer< XValueType >,
                   public MHO_IndexLabelInterface,
                   public MHO_IntervalLabelInterface {};
   
   template< typename... XAxisTypeS > 
   class MHO_AxisPack: public std::tuple< XAxisTypeS... >, virtual public MHO_Serializable {};

   template< typename XValueType, typename XAxisPackType >
   class MHO_TableContainer: public MHO_TableContainerBase,
                             public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >,
                             public XAxisPackType,
                             public MHO_Taggable {};

The axis objects themselves also inherit from 
:hops:`hops::MHO_IndexLabelInterface` and :hops:`hops::MHO_IntervalLabelInterface`.
These classes provide the to associate and index or a pair of indexes with additional 
key-value pairs. A simple example of this might be tagging a
section of the frequency axis with a particular channel ID (e.g
``[0, 32]`` :math:`\leftrightarrow` ``{"channel_id": "X17LY"}``). These two classes 
supports tagging axis locations with any type that is supported by the 
underlying json library.

A graphical representation of a :hops:`hops::MHO_TableContainer` object is shown below: 

.. figure:: ../../../_static/data-container-baseline.png

The table container class is composed of an N-dimensional array, coupled with axes to
provide coordinate values along each dimension. The axes themselves also
allow for arbitrary intervals to be labeled by key:value pairs in
order to allow for local look-up of meta data. For example, on the
the channel axis, the index labels may be channel names, while interval labels
maybe or sampler names among other possibilities.

It should be noted that the coordinate axes are present merely to label
the data, but are not meant to provide a reverse look-up capability,
(e.g example inverting the polarization-product code “LL” to infer a
0-th index location of 0). For efficiency, array access should still be
done using unsigned integer index values.

Container Dictionary and Store
------------------------------

The data containers in HOPS4 are organized by the dictionary class:
:hops:`hops::MHO_ContainerDictionary` and the storage class: :hops:`hops::MHO_ContainerStore`. 
Because the majority of the data containers in HOPS4 are instances of handful of template 
classes, the dictionary class :hops:`hops::MHO_ContainerDictionary` is responsible 
for the explicit instantiation (full specialization of all template parameters) 
of each type in use by HOPS4. It also inherits from :hops:`hops::MHO_ClassIdentityMap` which
allows for the association of each class type and its 128-bit class UUID. This class UUID is 
used to identify the object type on disk or in a byte stream, so that it can be cast to 
the appropriate type after stream. The storage class :hops:`hops::MHO_ContainerStore` is
used to store all of the in-memory containers. Objects can be retrieved from the store 
by specifying a the exact object UUID or an associated `shortname` (human readable string).
Collections of objects can be retrieved from the store by specifying the class UUID. 

For convenience the following series of tables contains the names/aliases, types, and descriptions
of the data container types which are which are explicitly instantiated by the container dictionary.
The following table contains alias for the table element types:

.. list-table:: Table Container Element Types
   :header-rows: 1

   * - name
     - type
     - description
   * - visibility_element_type
     - ``std::complex< double >``
     - the (in-memory) visibility table primitive
   * - weight_element_type
     - ``double``
     - the (in-memory) weight table primitive
   * - pcal_phasor_type
     - ``std::complex<double>`` 
     - the multi-tone pcal phasor primitive 
   * - spline_coeff_type
     - ``double``
     - coefficient used for polynomial splines
   * - flag_element_type 
     - ``char`` 
     - flag table primitive
   * - visibility_element_store_type
     - ``std::complex< float >``
     - the (on-disk) visibility table primitive
   * - weight_element_store_type
     - ``float``
     - the (on-disk) weight table primitive

The coordinate axis types are declared as follows:

.. list-table:: Coordinate Axis Types
   :header-rows: 1

   * - name
     - type
     - description
   * - polprod_axis_type
     - ``MHO_Axis< std::string >``
     - axis for polarization-product labels (XX, YX, XR, RL, RR, etc.)
   * - pol_axis_type
     - ``MHO_Axis< std::string >``
     - axis for polarization labels (X, Y, R, L, etc.)
   * - channel_axis_type
     - ``MHO_Axis< double >``
     - channels axis, with sky_frequency of each channel edge (MHz)
   * - frequency_axis_type
     - ``MHO_Axis< double >``
     - frequency axis, typically sub-channel axis (MHz)
   * - time_axis_type
     - ``MHO_Axis< double >``
     - time and/or AP axis
   * - coord_axis_type
     - ``MHO_Axis< std::string >`` 
     - station coordinate name (delay, phase, parallactic_angle, az, el, u, v, w)
   * - coeff_axis_type
     - ``MHO_Axis< int >``
     - spline coefficient index 0,1,2...(typical 6 is max)

The coordinate axis packs (collections of axes applied to each dimention of a table) are declared as follows:

.. list-table:: Axis Pack Types
   :header-rows: 1

   * - name
     - type
     - description
   * - baseline_axis_pack
     - ``MHO_AxisPack< polprod_axis_type, channel_axis_type, time_axis_type, frequency_axis_type >``
     - axis pack used for visibility and weight types
   * - station_coord_axis_pack
     - ``MHO_AxisPack< coord_axis_type, time_axis_type, coeff_axis_type >``
     - axis pack used for the station coordinate spline-model table 
   * - multitone_pcal_axis_type
     - ``MHO_AxisPack< pol_axis_type, time_axis_type, frequency_axis_type >``
     - axis pack used with multi-tone phase-cal table data

Finally, the table container types are declared by combining and element type 
with an axis pack tuple as a pair of template parameters. This allows for a 
wide variety of table types to be declared easily and with re-use of similar
coordinate axes. Some examples of these table container declarations are as
follows:

.. list-table:: Table Container Types
   :header-rows: 1

   * - name
     - type
     - description
   * - visibility_type
     - ``MHO_TableContainer< visibility_element_type, baseline_axis_pack >``
     - visibility data table (in-memory)
   * - visibility_store_type
     - ``MHO_TableContainer< visibility_element_store_type, baseline_axis_pack >``
     - visibility data table (on-disk)
   * - weight_type 
     - ``MHO_TableContainer< weight_element_type, baseline_axis_pack >``
     - weight data table (in-memory)
   * - weight_store_type 
     - ``MHO_TableContainer< weight_element_store_type, baseline_axis_pack >``
     - weight data table (on-disk)
   * - station_coord_type
     - ``MHO_TableContainer< spline_coeff_type, station_coord_axis_pack >``
     - station coordinate spline-model table 
   * - multitone_pcal_type
     - ``MHO_TableContainer< pcal_phasor_type, multitone_pcal_axis_type >``
     -  phase-cal table data

For further information on some of these concrete table container types, see the descriptions
under:

   - :doc:`visibilities <VisibilityData>`
   - :doc:`weights <WeightData>`
   - :doc:`pcal <PcalData>`
   - :doc:`station data <StationData>`


Adding Metadata to Table Containers
-----------------------------------

Adding metadata to a table container object is quite straight forward. Since all 
table container types inherit from :hops:`hops::MHO_Taggable` they permit the 
addition of nearly arbitrary key:value meta data via the json library. It is however, 
encouraged that this mechanism is primarily used for scalar (i.e. single valued) key:value pairs,
while more complex nested metadata is specified via the use of :hops:`hops::MHO_ObjectTags`.


Adding New Container Types
--------------------------

It is not expected that the current set of data container types is comprehensive for 
either current or future data processing. However, adding new data types is relatively simple.
To declare a new table container type, all that is need is that the table data element 
and axis pack be specified and added to the container dictionary. As an example, consider 
a three dimensional table consisting of integers, with three coordinate axis (the first consisting of string labels,
the second of integer labels, and the last of doubles). This object can be declared as:

.. code-block:: cpp

   using my_new_axis_pack = MHO_AxisPack< MHO_Axis<std::string>, MHO_Axis<int>, MHO_Axis<double> >;
   using my_new_table_type = MHO_TableContainer< int, my_new_axis_pack >;

Then all that is needed in order to use this new table container type (both for disk-I/O and in the container store)
is to add its definition to the container dictionary :hops:`hops::MHO_ContainerDictionary` via the ``AddClassType`` function:

.. code-block:: cpp

   //add to the constructor: MHO_ContainerDictionary::MHO_ContainerDictionary()
   AddClassType< my_new_table_type >();


Data Container Extensions
~~~~~~~~~~~~~~~~~~~~~~~~~

The primary goal of the containers is to provide a relatively simple and
efficient representation of commonly used data types that hides the
details of memory management and array indexing/access from the user.
They should not be overburdened with too much extraneous functionality
(beyond simple operator overloads like assignment, scalar multiply, etc.)
that is specific to a particular operation as this greatly over
complicates these classes and makes them brittle.

However, there are some cases where this sort of decoupling may induce a
performance cost. An example of this occurs in the case of SIMD/GPU
acceleration. In order to make use of GPU processing the data must be
copied to a buffer on the device, processed, and then the results must
be passed back to the host. However, if there are several operations to
be performed in succession on the GPU, only first and last transfer need
to occur, with intervening transfers being unnecessary as input data is
already present on the device. However, in order to eliminate the
intermediate transfers a handle to the device buffer must be kept
persistent in memory. So the questions arises, where should we keep this
device buffer object? Should it be kept as a member of the data
operator? That would be a poor choice, since if it is private it will
not accessible to other operators to make use of, and if it is public
then it will introduce the possibility of tight coupling with other
portions of the code making use of the buffer. On the other hand, a
pointer to a device buffer is too specific to belong in something as
basic as a data container. However, it is a good candidate for something
to may be stored in an extension.

In order to provide the ability to append extensions to the data
containers, they must all inherit from a base class,
:hops:`hops::MHO_ExtensibleElement`, which in turn stores a vector of
type-erased [1]_ pointers to the extensions themselves. The extensions
are templated on the the class providing the additional functionality
and must all inherit from the base class :hops:`hops::MHO_ExtendedElement` (so
they can be stored in the vector) A brief
sketch of the code that allows for this is shown below.
One draw back of this method is that requires :math:`N` ``dynamic_cast`` 
calls any time a particular extension is modified or accessed via the data container.
This is an acceptable trade off for infrequent access to expensive (to construct)
extensions, but should be used rather sparingly as ``dynamic_cast`` has
high overhead.

.. code:: cpp

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

This extensible element mechanism is made use of in :hops:`hops::MHO_ContainerJSONConverter` and 
in :hops:`hops::MHO_ContainerHDF5Converter` for data format conversion, as well as 
in :hops:`hops::MHO_OpenCLNDArrayBuffer` for OpenCL buffer construction/manipulation.
