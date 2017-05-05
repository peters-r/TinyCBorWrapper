/**
 * @file main_testing.cpp
 * @date 02.05.2017
 * @author Richard Peters
 *
 * @brief TinyCBORWrapper 
 */

#include <iostream>

#include "TinyCborWrapper.hpp"


//-----------------------------------------------------------------------------
// Structs to serialize and deserialize
//-----------------------------------------------------------------------------

struct ExampleInner {
    std::string name;
    uint32_t value;
};


struct Example {
    std::vector<uint8_t> bytes;
    int32_t value;
    ExampleInner inner;
};


//-----------------------------------------------------------------------------
// Serialization overloads
//-----------------------------------------------------------------------------

/**
 * Serialize InnerSample to CBOR
 */
inline CBOR::Encoder& operator << (CBOR::Encoder& container, const ExampleInner& value)
{
    using namespace CBOR;
    
    container << startMap(2)
            << "name"  << CString(value.name)
            << "value" << CUint(value.value)
            << end;
    
    return container;
}


//-----------------------------------------------------------------------------

/**
 * DeSerialize InnerSample to CBOR
 */
inline CBOR::Decoder& operator >> (CBOR::Decoder& container, ExampleInner& value)
{
    using namespace CBOR;
    
    container 
        >> enter
            >> skip >> value.name
            >> skip >> value.value
        >> leave;
    
    return container;
}


//-----------------------------------------------------------------------------

/**
 * Serialize Sample to CBOR
 */
inline CBOR::Encoder& operator << (CBOR::Encoder& container, const Example& value)
{
    using namespace CBOR;
    
    container 
        << startArray(3) 
            << CBytes(value.bytes) << CInt(value.value) << value.inner
        << end;
    
    return container;
}


//-----------------------------------------------------------------------------

/**
 * DeSerialize Sample to CBOR
 */
inline CBOR::Decoder& operator >> (CBOR::Decoder& container, Example& value)
{
    using namespace CBOR;
    
    container 
        >> enter
            >> value.bytes >> value.value >> value.inner
        >> leave;
    
    return container;
}


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main() {

    using namespace CBOR;
    
    Example sample = {
        .bytes = { 1, 2, 3, 4, 5 },
        .value = -20,
        .inner = {
                .name = "Hello",
                .value = 10
        }
    };
    
    Example sample2;
    
    // encode sample struct
    EncoderBuffer e;
    e << sample;
    
    // decode sample struct
    DecoderBuffer d(e.getBuffer(), e.getBufferSize());
    d >> sample2;
    
    
    // print sample struct
    std::cout << "Bytes: { " <<std::endl;
    for (auto byte : sample2.bytes) {
        std::cout << byte << " ";
    }
    std::cout 
        << "}, Value: " << sample2.value 
        << " , inner.name: " << sample2.inner.name 
        << " , inner.value: " << sample2.inner.value <<std::endl;
    
    
    return 0;
}


//-----------------------------------------------------------------------------
