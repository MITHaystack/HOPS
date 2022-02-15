Converting Mk4 data to new HOPS4 format.
****************************************
Date: 01/31/22

ConvertMk4Data
--------------
To convert a single scan of mark4 data to the new data format the utility 
ConvertMk4Data is provided as a C++ application. This application is built and
installed without any special configuration flags needed in the cmake build 
setup and is available by default.

The help out output of ConvertMk4Data -h is given below::

ConvertMk4Data -i <input_directory> -o <output_directory>

where the input_directory must be a single scan worth of mark4 data, containing
the root (ovex) file, and the station and corel files to be converted. The output
directory must also be specified for where the converted files will be dumped, 
but it does not need to be pre-existing, as it will be created if needed.

Note that ConvertMk4Data only operates as a single scan of mark4 data at one time
and is not able to convert an entire experiment directory at once. This is would
be a desirable feature to implement at some point. 

As an example, consider a set of mark4 files in a scan directory such as ./No0043
listed below::

A..1E9W4X  AA..1E9W4X  AO..1E9W4X  O..1E9W4X  OO..1E9W4X  WXPSC.1E9W4X

These can be converted to the new format with the ConvertMk4Data command::

ConvertMk4Data -i ./No0043 -o ./new-No0043

This will produce a print-out to the terminal as::

ConvertMk4Data -i ./No0043/ -o ./new-No0043
INFO[main] input directory: /home/barrettj/work/projects/hops-test-data/8000/No0043
INFO[main] output directory: /home/barrettj/work/projects/hops-test-data/8000/new-No0043
INFO[mk4interface] successfully read vex file: /home/barrettj/work/projects/hops-test-data/8000/No0043/WXPSC.1E9W4X
INFO[file] Converting corel input file: /home/barrettj/work/projects/hops-test-data/8000/No0043/AO..1E9W4X
INFO[file] Converting corel input file: /home/barrettj/work/projects/hops-test-data/8000/No0043/OO..1E9W4X
INFO[file] Converting corel input file: /home/barrettj/work/projects/hops-test-data/8000/No0043/AA..1E9W4X
INFO[file] Converting station input file: /home/barrettj/work/projects/hops-test-data/8000/No0043/O..1E9W4X
INFO[file] Converting station input file: /home/barrettj/work/projects/hops-test-data/8000/No0043/A..1E9W4X
 
and will create a new directory ./new-No0043 with the contents::

A.1E9W4X.sta  AA.1E9W4X.cor  AO.1E9W4X.cor  O.1E9W4X.sta  OO.1E9W4X.cor  WXPSC.1E9W4X.json

The root (ovex) file will be converted to a .json file. Wheres the station data 
files will be given the extension ".sta", and the corel (visibility) data files 
will be given the extension ".cor". For the time being the 6-character HOPS root
code is carried over in the file names as a useful time-stamp/unique identifier.

**Note that for the time being the ".cor" files promote the visibility data from 
float to double, doubling the file size. At some point we may want to revert this
to save disk space.**

DumpFileObjectKeys
------------------

Once you have converted the mark4 data to the new format you can check that the 
objects where converted and stored properly by dumping the file object keys to
the terminal for the ".sta" and ".cor" files. This will display the contents of the
64 byte object headers the precede each object stored in the file. To dump the 
object keys for a ".cor" file, for example, run::

DumpFileObjectKeys -f ./AO.1E9W4X.cor 
key:
    sync: efbeadde
    label: ffffffff
    object uuid: 00000000f3eb4a2f82a8f68af7d44de0
    type uuid: 5eded62539d1fd31e63cee324ca65674
    class name: hops::MHO_TableContainer<std::complex<double>, hops::MHO_AxisPack<hops::MHO_Axis<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, hops::MHO_Axis<int>, hops::MHO_Axis<double>, hops::MHO_Axis<double> > >
    object name: vis
    size (bytes): 19241249
------------------------------------------------------------
key:
    sync: efbeadde
    label: ffffffff
    object uuid: 000000008e024f69a178a948ff2c7f1a
    type uuid: 65a55ef613bfa4bb2fe56a7614222faa
    class name: hops::MHO_TableContainer<double, hops::MHO_AxisPack<hops::MHO_Axis<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, hops::MHO_Axis<int>, hops::MHO_Axis<double>, hops::MHO_Axis<double> > >
    object name: weight
    size (bytes): 9625889
------------------------------------------------------------

And similarly for the station object ".sta" files, you should see something like::

DumpFileObjectKeys -f ./A.1E9W4X.sta 
key:
    sync: efbeadde
    label: ffffffff
    object uuid: 00000000c7524b13aca550bad6f61beb
    type uuid: 50c1ac0b3f9867e741af1f5353f22107
    class name: hops::MHO_TableContainer<double, hops::MHO_AxisPack<hops::MHO_Axis<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, hops::MHO_Axis<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, hops::MHO_Axis<int>, hops::MHO_Axis<int> > >
    object name: sta
    size (bytes): 1851

DumpFileObject
--------------

In order to generate a "human readable" representation of the objects stored in
a file there is a utility called DumpFileObject with the usage as follows::

DumpFileObject -f <file> -t <type> -u <uuid>

Which for the file example above, could be used to generate an json (ascii) dump 
of the visibility object using the command as follows:

DumpFileObjectKeys -f ./AO.1E9W4X.cor -t 5eded62539d1fd31e63cee324ca65674 -u 00000000f3eb4a2f82a8f68af7d44de0

The print-out of this file object is too long to show here, but is a well-formed 
json string which could be used for (very slow) conversion into other formats. This 
tool is akin to CorAsc2 for the new data containers, and is primarily intended as
a data inspection and debugging tool.








