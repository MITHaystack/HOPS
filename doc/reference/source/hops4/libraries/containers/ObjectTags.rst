Data Object: `object_tags`
==========================

The :hops:`hops::MHO_ObjectTags` class is used as a means to attach arbitrary information to a separate object or collection of objects.
The object collection associated with a particular object tag object is specified by a list of object unique identifiers (UUIDs) stored as strings.
The objects with the associated UUIDs are generally contained in the same file, or if in memory, in the same :hops:`hops::MHO_ContainerStore`.
The underlying type of the ``MHO_ObjectTags`` class is a ``json`` object provided by the external ``nlohmann::json`` library.

General Information
-------------------
- **Class**: :hops:`hops::MHO_ObjectTags`
- **Class UUID**: 330c5f9889eaa350f8955c6e709a536c

Tags
----

The object collection is referenced by a list of 128 bit object UUIDs. The object UUIDs are stored as hex-strings (e.g. "c159826f284e416985db47c926080274")
under a list object called ``object_uuids``. The UUIDs stored here are also present in the object 
file keys (:hops:`hops::MHO_FileKey`) that precede the binary data of each respective object in a file. In memory objects 
can also be retrieved from the container store (:hops:`hops::MHO_ContainerStore`) using the same unique object UUID as a key.

The meta-data tags (key:value pairs) that are associated with the object collection, can take on any
nested structure that is available to the ``json`` format. However, the top level object which contains
all such information is called ``tags``. For example, in the ``fourfit4`` fringe output, 
much of the fringe-fitting parameters, and plotting data are stored in an ``MHO_ObjectTags`` object, under "/tags/parameters" and "/tags/plot_data".
A example portion of which looks like:

.. code-block:: json

    "class_name": "MHO_ObjectTags",
        "class_uuid": "330c5f9889eaa350f8955c6e709a536c",
        "object_uuids": [
            "c159826f284e416985db47c926080274"
        ],
        "tags": {
            "parameters": {
                "cmdline": {
                    "accounting": false,
                    "ap_per_seg": 0,
                    "args": [
                        "fourfit4",
                        "-c",
                        "../../cf_3686_GEHSVY_pstokes",
                        "-P",
                        "XX",
                        "-b",
                        "GE",
                        "-m",
                        "-1",
                        "./"
                    ],
                    "baseline": "GE",
                    "control_file": "../../cf_3686_GEHSVY_pstokes",
                    "directory": "./",
                    "disk_file": "",
                    "exclude_autos": false,
                    "first_plot_channel": 0,
                    "frequency_group": "?",
                    "message_level": -1,
                    "polprod": "XX",
                    ...
                    ...
                    ...
            "plot_data": {
                "AP(sec)": 1.0,
                "Amp": 4.555523059968371,
                "AprioriAccel(us/s/s)": -2.351127226975069e-06,
                "AprioriClock(usec)": 96.58727777,
                "AprioriClockrate(us/s)": 1.238e-06,
                "AprioriDelay(usec)": -426.7657403524763,
                "AprioriRate(us/s)": 0.01546835364977025,
                "BuildTime": "2025:154:194416",
                "ChannelsPlotted": [
                    "a",
                    "b",
                    "c",
                    ...
                    ...
                    ...
