.. _PluginsLibrary:

Plugins
=======

The plugin libraries provides a collection of (optional) extensions for the HOPS4 framework. This
section documents a subset of these plugins: GPU acceleration via CUDA, HDF5 data export, and
Python integration (bindings). Other optional plugins (e.g. OpenCL, OpenMP, Matplot++-based
plotting, Python subprocess plotting) exist in ``source/cpp_src/Plugins/`` but are not yet
documented here.

The python bindings plugin depends only on python (python3-dev) and an integrated copy of the pybind11 header-only library 
and does not introduce any other external dependencies. 

The HDF5 export library requires the HDF5 development library (libhdf5-dev) in order to be compiled, and provides
the ability to export HOPS4 data containers to the HDF5 format.

The CUDA and OpenCL extensions are currently experimental and require a GPU with the associated development libraries and drivers.

.. toctree::
   :maxdepth: 2

   python_bindings
   hdf5_data_export
   cuda_acceleration
