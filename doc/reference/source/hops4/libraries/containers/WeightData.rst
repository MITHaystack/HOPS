
Data Object: weights
====================

.. list-table:: Table Container Metadata
   :widths: 15 15 20 50
   :header-rows: 1

   * - Axis
     - Type
     - Shape
     - Metadata
   * - axis_0
     - ``MHO_Axis<string>``
     - ``[4]``
     - name: polarization_product
   * - axis_1
     - ``MHO_Axis<double>``
     - ``[32]``
     - name: channel  
       units: MHz  
       index_labels: 32 labels from 0..31  
       each label contains:  
       
       - bandwidth  
       - channel  
       - difx_freqindex  
       - frequency_band  
       - index  
       - mk4_channel_id  
       - net_sideband  
       - sky_freq
   * - axis_2
     - ``MHO_Axis<double>``
     - ``[30]``
     - name: time  
       units: s
   * - axis_3
     - ``MHO_Axis<double>``
     - ``[1]``
     - name: frequency  
       units: MHz

.. list-table:: Table Container Info
   :widths: 20 80
   :header-rows: 1

   * - Property
     - Value
   * - class_name
     - ``MHO_TableContainer<float,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>``
   * - class_uuid
     - f05838a616aa848562a57d5ace23e8d1
   * - rank
     - 4
   * - dimensions
     - ``[4, 32, 30, 1]``
   * - strides
     - ``[960, 30, 1, 1]``
   * - total_size
     - 3840

.. list-table:: Table Tags
   :widths: 25 75
   :header-rows: 1

   * - Tag
     - Value
   * - name
     - weights
   * - origin
     - difx
   * - root_code
     - 3SU576
   * - start
     - 2019y105d18h00m00s
   * - correlation_date
     - 2023y061d02h05m31s
   * - baseline
     - Gs-K2
   * - baseline_shortname
     - GH
   * - difx_baseline_index
     - 258
   * - reference_station
     - Gs
   * - reference_station_name
     - GGAO12M
   * - reference_station_mk4id
     - G
   * - remote_station
     - K2
   * - remote_station_name
     - KOKEE12M
   * - remote_station_mk4id
     - H
