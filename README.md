TinyCborWrapper
===============

The TinyCborWrapper is a C++11 Wrapper for the TinyCbor C-library.
Use it as is, without any warranty.
The wrapper was written for a project were i am currently working on: http://supplets.com.

Documentation and Tests may be added in the future.
The file main_testing.cpp is an example for the usage.

Don't forget to clone it with the tinycbor submodule:
`git clone <URL>`
`git submodule update --init`

Just enter the directory and type `make` to build the sample.
The wrapper itself is just the TinyCborWrapper.hpp file,
so you can include it in your project.
Make sure the file "cbor.h" from tinycbor is available in your include path.

These three files from tinycbor must be compiled and linked with your project: 
- cborencoder.c
- cborencoder_close_container_checked.c
- cborparser.c
