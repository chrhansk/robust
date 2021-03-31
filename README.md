# A robust shortest path implementation

This code supplements the paper

[Hansknecht, Christoph, Alexander Richter, and Sebastian Stiller. "Fast robust shortest path computations."](https://drops.dagstuhl.de/opus/volltexte/2018/9710/)

## Dependencies

This code depends on the following packages:

- [METIS](http://glaros.dtc.umn.edu/gkhome/views/metis)
- [GoogleTest](https://github.com/google/googletest)
- [Intel TBB](https://www.threadingbuildingblocks.org/)
- [Google Protobuf](https://developers.google.com/protocol-buffers)

## Building and testing

To build and test the code, run

- mkdir build
- cd build
- cmake .. <CMAKE_OPTIONS>
- make
- [make test]

## Running benchmarks

This code includes several benchmarks, which are build
automatically. To execute all benchmarks and collect their respective
results, use the command

- make collect

This should produce several CSV files containing running times. Beware
to build the code in release mode to get accurate benchmarks results.

## Dataset

The dataset was derived from [OpenStreetMap](https://www.openstreetmap.org) data (© OpenStreetMap contributors).
It is therefore licensed under the [Open Data Commons Open Database License (ODbL)](https://opendatacommons.org/licenses/odbl/), 
see the related [Copyright](https://www.openstreetmap.org/copyright).
