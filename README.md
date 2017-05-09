TinyCborWrapper
===============

The TinyCborWrapper is a C++11 Header-only wrapper for the TinyCbor C-library.
Just use it as is, without any warranty.
The wrapper has been written for an IOT project, i am currently working on: http://supplets.com.

Documentation and Tests may be added in the future.
The file MainSample.cpp is an example for the usage and contains a main routine.

Don't forget to checkout the tinycbor submodule:

`git submodule update --init`

Just enter the directory and type `make` to build the sample.
The wrapper itself is just the TinyCborWrapper.hpp file,
so you can include it in your project.
Make sure the file "cbor.h" from tinycbor is available in your include path.

These three files from tinycbor must be compiled and linked with your project: 
- cborencoder.c
- cborencoder_close_container_checked.c
- cborparser.c
