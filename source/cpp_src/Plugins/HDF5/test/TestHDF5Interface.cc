#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

#include <string>

#include "hdf5.h"
#include "hdf5_hl.h"
#include <stdio.h>

int main()
{
    std::string filename = "example.h5";
    std::string dataset_name = "/my_data";
    std::string attribute_name = "metadata";

    double data[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    hsize_t dims[1] = {5};

    //fake meta data
    std::string json_metadata = "{\"description\": \"example\", \"units\": \"m\"}";

    hid_t file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if(file_id < 0)
    {
        std::cout << "failed to create HDF5 file" << std::endl;
        return 1;
    }

    // Create a 1D dataset of doubles
    if(H5LTmake_dataset_double(file_id, dataset_name.c_str(), 1, dims, data) < 0)
    {
        std::cout << "failed to create dataset " << std::endl;
        H5Fclose(file_id);
        return 1;
    }

    // Attach JSON metadata as an attribute
    if(H5LTset_attribute_string(file_id, dataset_name.c_str(), attribute_name.c_str(), json_metadata.c_str()) < 0)
    {
        std::cout << "failed to write attribute" << std::endl;
        H5Fclose(file_id);
        return 1;
    }

    // Close file
    H5Fclose(file_id);
    std::cout << "HDF5 file created successfully with dataset and metadata." << std::endl;
    return 0;
}
