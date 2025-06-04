..  _Utilities:

Utilities
=========

Lets's use chapter to add some C++ code here:
With ``.. code-block:: cpp`` we can add C++ snippets:

.. code-block:: cpp 

    int main()
    {
    std::cout << "hello sphinx!\n";
    return 0;
    }


Source Code documentation
-------------------------

.. doxygenclass:: hops::MHO_DirectoryInterface
    :project: hops
    :members:
    :private-members:


.. doxygenclass:: hops::MHO_FileKey
    :project: hops
    :members:
    :private-members:

.. doxygenclass:: hops::MHO_TimeStampConverter
    :project: hops
    :members:
    :private-members:
