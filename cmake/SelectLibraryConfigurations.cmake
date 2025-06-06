# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Copyright 2000-2025 Kitware, Inc. and `Contributors <CONTRIBUTORS.rst>`_
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 
# * Neither the name of Kitware, Inc. nor the names of Contributors
#   may be used to endorse or promote products derived from this
#   software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
SelectLibraryConfigurations
---------------------------

This module is intended to be used in :ref:`Find Modules` when finding packages
that are available with multiple :ref:`Build Configurations`.  It provides a
macro that automatically sets and adjusts library variables.  Supported library
build configurations are ``Release`` and ``Debug`` as these are the most common
ones in such packages.

.. note::

  This module has been available since early versions of CMake, when the
  ``<PackageName>_LIBRARIES`` result variable was used for linking found
  packages.  When writing standard find modules, :ref:`Imported Targets` should
  be preferred.  In addition to or as an alternative to this module, imported
  targets provide finer control over linking through the
  :prop_tgt:`IMPORTED_CONFIGURATIONS` property.

.. command:: select_library_configurations

  .. code-block:: cmake

    select_library_configurations(<basename>)

  This macro is a helper for setting the ``<basename>_LIBRARY`` and
  ``<basename>_LIBRARIES`` result variables when a library might be provided
  with multiple build configurations.

  The argument is:

  ``<basename>``
    The base name of the library, used as a prefix for variable names.  This is
    the name of the package as used in the ``Find<PackageName>.cmake`` module
    filename, or the component name, when find module provides them.

  Prior to calling this macro the following cache variables should be set in the
  find module (for example, by the :command:`find_library` command):

  ``<basename>_LIBRARY_RELEASE``
    A cache variable storing the full path to the ``Release`` build of the
    library.  If not set or found, this macro will set its value to
    ``<basename>_LIBRARY_RELEASE-NOTFOUND``.

  ``<basename>_LIBRARY_DEBUG``
    A cache variable storing the full path to the ``Debug`` build of the
    library.  If not set or found, this macro will set its value to
    ``<basename>_LIBRARY_DEBUG-NOTFOUND``.

  This macro then sets the following local result variables:

  ``<basename>_LIBRARY``
    A result variable that is set to the value of
    ``<basename>_LIBRARY_RELEASE`` variable if found, otherwise it is set to the
    value of ``<basename>_LIBRARY_DEBUG`` variable if found.  If both are found,
    the release library value takes precedence. If both are not found, it is set
    to value ``<basename>_LIBRARY-NOTFOUND``.

    If the :manual:`CMake Generator <cmake-generators(7)>` in use supports
    build configurations, then this variable will be a list of found libraries
    each prepended with the ``optimized`` or ``debug`` keywords specifying which
    library should be linked for the given configuration.  These keywords are
    used by the :command:`target_link_libraries` command.  If a build
    configuration has not been set or the generator in use does not support
    build configurations, then this variable value will not contain these
    keywords.

  ``<basename>_LIBRARIES``
    A result variable that is set to the same value as the
    ``<basename>_LIBRARY`` variable.

  .. note::

    The ``select_library_configurations()`` macro should be called before
    handling standard find module arguments with
    :module:`find_package_handle_standard_args()
    <FindPackageHandleStandardArgs>` to ensure that the ``<PackageName>_FOUND``
    result variable is correctly set based on ``<basename>_LIBRARY`` or other
    related variables.

Examples
^^^^^^^^

Setting library variables based on the build configuration inside a find module
file:

.. code-block:: cmake
  :caption: FindFoo.cmake

  # Find release and debug build of the library
  find_library(Foo_LIBRARY_RELEASE ...)
  find_library(Foo_LIBRARY_DEBUG ...)

  # Set Foo_LIBRARY and Foo_LIBRARIES result variables
  include(SelectLibraryConfigurations)
  select_library_configurations(Foo)

  # Set Foo_FOUND variable and print result message.
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    Foo
    REQUIRED_VARS Foo_LIBRARY ...
  )

When find module provides components with multiple build configurations:

.. code-block:: cmake
  :caption: FindFoo.cmake

  include(SelectLibraryConfigurations)
  foreach(component IN LISTS Foo_FIND_COMPONENTS)
    # ...
    select_library_configurations(Foo_${component})
    # ...
  endforeach()

A project can then use this find module as follows:

.. code-block:: cmake
  :caption: CMakeLists.txt

  find_package(Foo)
  target_link_libraries(project_target PRIVATE ${Foo_LIBRARIES})
  # ...
#]=======================================================================]

# This macro was adapted from the FindQt4 CMake module and is maintained by Will
# Dicharry <wdicharry@stellarscience.com>.

macro(select_library_configurations basename)
    if(NOT ${basename}_LIBRARY_RELEASE)
        set(${basename}_LIBRARY_RELEASE "${basename}_LIBRARY_RELEASE-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()
    if(NOT ${basename}_LIBRARY_DEBUG)
        set(${basename}_LIBRARY_DEBUG "${basename}_LIBRARY_DEBUG-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()

    get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if( ${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE AND
           NOT ${basename}_LIBRARY_DEBUG STREQUAL ${basename}_LIBRARY_RELEASE AND
           ( _isMultiConfig OR CMAKE_BUILD_TYPE ) )
        # if the generator is multi-config or if CMAKE_BUILD_TYPE is set for
        # single-config generators, set optimized and debug libraries
        set( ${basename}_LIBRARY "" )
        foreach( _libname IN LISTS ${basename}_LIBRARY_RELEASE )
            list( APPEND ${basename}_LIBRARY optimized "${_libname}" )
        endforeach()
        foreach( _libname IN LISTS ${basename}_LIBRARY_DEBUG )
            list( APPEND ${basename}_LIBRARY debug "${_libname}" )
        endforeach()
    elseif( ${basename}_LIBRARY_RELEASE )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_RELEASE} )
    elseif( ${basename}_LIBRARY_DEBUG )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_DEBUG} )
    else()
        set( ${basename}_LIBRARY "${basename}_LIBRARY-NOTFOUND")
    endif()

    set( ${basename}_LIBRARIES "${${basename}_LIBRARY}" )

    if( ${basename}_LIBRARY )
        set( ${basename}_FOUND TRUE )
    endif()

    mark_as_advanced( ${basename}_LIBRARY_RELEASE
        ${basename}_LIBRARY_DEBUG
    )
endmacro()
