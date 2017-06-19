  /**
 * @file CBOREncoder.h
 * @date 02.05.2017
 * @author Richard Peters
 *
 * @brief TinyCBORWrapper 
 */

#ifndef TINYCBORWRAPPER_HPP_
#define TINYCBORWRAPPER_HPP_

#include <cbor.h>
#include <string>
#include <vector>
#include <initializer_list>
#include <functional>

namespace CBOR {


//-----------------------------------------------------------------------------
// CBOR Types
//-----------------------------------------------------------------------------

template <class TSrc, class TDst>
struct CBORValue
{
    explicit CBORValue(TDst& val) : value(val) { }
    operator TDst() { return value; }
    
    void set(const TSrc& val) { value = val; }
    
    TDst& value;
};

template <class T>
struct CBORConstValue
{
    explicit CBORConstValue(const T& val) : value(val) { }
    operator T() { return value; }
    T value;
};


//-----------------------------------------------------------------------------

struct CNull { };
struct CUndefined { };

typedef CBORConstValue<std::string> CString;
typedef CBORConstValue< std::vector<uint8_t> > CBytes;
typedef CBORConstValue<int64_t> CInt;
typedef CBORConstValue<uint64_t> CUint;
typedef CBORConstValue<bool> CBool;
typedef CBORConstValue<float> CFloat;
typedef CBORConstValue<double> CDouble;


//-----------------------------------------------------------------------------

template<typename T> using TString = CBORValue<std::string, T>;
template<typename T> using TBytes = CBORValue<std::vector<uint8_t>, T>;
template<typename T> using TInt = CBORValue<int64_t, T>;
template<typename T> using TUint = CBORValue<uint64_t, T>;
template<typename T> using TBool = CBORValue<bool, T>;
template<typename T> using TFloat = CBORValue<float, T>;
template<typename T> using TDouble = CBORValue<double, T>;

//-----------------------------------------------------------------------------

class Encoder;
class Decoder;
typedef std::function<Encoder&(Encoder&)> tEncoderFn;
typedef std::function<Decoder&(Decoder&)> tDecoderFn;


//-----------------------------------------------------------------------------
// Exceptions
//-----------------------------------------------------------------------------

class EncoderException
{
public:

    EncoderException(CborError err) : m_err(err) {} 
    
    CborError getErrorCode() { return m_err; }
    
private:
    CborError m_err;
};


//-----------------------------------------------------------------------------

class DecoderException
{
public:

    DecoderException(CborError err) : m_err(err) {} 
    
    CborError getErrorCode() { return m_err; }
    
private:
    CborError m_err;
};


//-----------------------------------------------------------------------------
// CBorEncoder
//-----------------------------------------------------------------------------

class Encoder
{

public:
    
    Encoder(CborEncoder& rEncoder) : m_rEncoder(rEncoder) { }

    virtual ~Encoder() { }
    
    Encoder& encode(const CUint& value) 
    {   
        CborError err = cbor_encode_uint(&m_rEncoder, value.value);
        if (err != CborNoError) 
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encode(const CInt& value) 
    {   
        CborError err = cbor_encode_int(&m_rEncoder, value.value);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encode(const CString& value)
    {
        CborError err = cbor_encode_text_string(
                &m_rEncoder, value.value.c_str(), value.value.length());
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encode(const CBytes& value)
    {
        CborError err = cbor_encode_byte_string(
                &m_rEncoder, value.value.data(), value.value.size());
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encode(const CBool& value)
    {
        CborError err = cbor_encode_boolean(&m_rEncoder, value.value);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encode(const CFloat& value)
    {
        CborError err = cbor_encode_float(&m_rEncoder, value.value);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }

    Encoder& encode(const CDouble& value)
    {
        CborError err = cbor_encode_double(&m_rEncoder, value.value);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }
    
    Encoder& encodeNull()
    {
        CborError err = cbor_encode_null(&m_rEncoder);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }

    Encoder& encodeUndefined()
    {
        CborError err = cbor_encode_undefined(&m_rEncoder);
        if (err != CborNoError)
            throw EncoderException(err);
        
        return *this;
    }

    CborEncoder& getEncoder() { return m_rEncoder; }
    
    
protected:

    CborEncoder& m_rEncoder;
    friend Encoder& operator<<(Encoder&, tEncoderFn);
    friend Encoder& createArray(Encoder& container, size_t size);
    friend Encoder& createMap(Encoder& container, size_t size);
};


//-----------------------------------------------------------------------------

class EncoderBuffer : public Encoder
{

public:
    
    EncoderBuffer(size_t buffer_size = 4096) 
        : Encoder(m_rEncoder), m_pBuffer(new uint8_t[buffer_size]), m_bufferSize(buffer_size)
    {
        cbor_encoder_init(&m_rEncoder, m_pBuffer, buffer_size, 0);
    }

    virtual ~EncoderBuffer() { delete[] m_pBuffer; }

    size_t size() {
        return cbor_encoder_get_buffer_size(&m_rEncoder, m_pBuffer);
    }
    
    uint8_t* getBuffer() { return m_pBuffer; }
    
    size_t getBufferSize() { return m_bufferSize; }
    
private:

    CborEncoder m_rEncoder;
    uint8_t* m_pBuffer;
    size_t m_bufferSize;
};


//-----------------------------------------------------------------------------

class InnerEncoder : public Encoder
{
public:
    
    InnerEncoder(Encoder& rOuterEncoder) 
        : Encoder(m_encoder), m_rOuter(rOuterEncoder) { }
    
    virtual ~InnerEncoder() {
        cbor_encoder_close_container(&m_rOuter.getEncoder(), &m_encoder);
    }

    CborEncoder& getEncoder() { return m_encoder; }

    /* friend functions */
    friend Encoder& end(Encoder& container);
    friend Encoder& createMap(Encoder& container, size_t size);
    friend Encoder& createArray(Encoder& container, size_t size);


protected:

    CborEncoder m_encoder;
    Encoder& m_rOuter;

};


//-----------------------------------------------------------------------------

inline Encoder& end(Encoder& container)
{
    if (InnerEncoder* s = dynamic_cast<InnerEncoder*>(&container)) {
        Encoder& outer = s->m_rOuter;
        delete s;
        return outer;
    }

    return container;
}


//-----------------------------------------------------------------------------

inline Encoder& createMap(Encoder& container, size_t size)
{
    InnerEncoder* pInner = new InnerEncoder(container);
    
    CborError err = cbor_encoder_create_map(
            &container.m_rEncoder, &pInner->getEncoder(), size);
    
    if (err != CborNoError)
        throw EncoderException(err);
    
    return *pInner;
}



//-----------------------------------------------------------------------------

inline Encoder& createArray(Encoder& container, size_t size)
{
    InnerEncoder* pInner = new InnerEncoder(container);
    
    CborError err = cbor_encoder_create_array(
            &container.m_rEncoder, &pInner->getEncoder(), size);
    
    if (err != CborNoError)
        throw EncoderException(err);
    
    return *pInner;
}


//-----------------------------------------------------------------------------

inline tEncoderFn startMap(size_t size = CborIndefiniteLength)
{
    return std::bind(createMap, std::placeholders::_1, size);
}


//-----------------------------------------------------------------------------

inline tEncoderFn startArray(size_t size = CborIndefiniteLength)
{
    return std::bind(createArray, std::placeholders::_1, size);
}


//-----------------------------------------------------------------------------

inline Encoder& operator << (Encoder& container, const char* str)
{
    return container.encode(CString(str));
}


//-----------------------------------------------------------------------------

template <typename T>
inline Encoder& operator << (Encoder& container, const T& value)
{
    return container.encode(value);
}


//-----------------------------------------------------------------------------

template <>
inline Encoder& operator<< <decltype(end)>(Encoder& container, decltype(end)& op)
{
    return op(container);
}


//-----------------------------------------------------------------------------

inline Encoder& operator << (Encoder& container, tEncoderFn op)
{
    return op(container);
}


//-----------------------------------------------------------------------------
// CBOR Decoder
//-----------------------------------------------------------------------------

class Decoder
{

public:
    
    Decoder(CborValue& position) : m_it(position) { }

    virtual ~Decoder() { }
    
    uint64_t decodeUint() 
    {   
        CborError err;
        uint64_t value_buffer;
        
        if (cbor_value_get_uint64(&m_it, &value_buffer) != CborNoError)
            throw DecoderException(err);
        
        next();
        
        return value_buffer;
    }
    
    
    int64_t decodeInt() 
    {   
        CborError err;
        int64_t value_buffer;
        
        if ((err = cbor_value_get_int64(&m_it, &value_buffer)) != CborNoError)
            throw DecoderException(err);
        
        next();
        
        return value_buffer;
    }
    
    
    std::string decodeString() 
    {   
        CborError err;
        size_t len;
        
        err = cbor_value_calculate_string_length(&m_it, &len);
        if (err != CborNoError)
            throw DecoderException(err);
        
        char* str_val = new char[++len];
        err = cbor_value_copy_text_string(&m_it, str_val, &len, &m_it);
        if (err != CborNoError)
            throw DecoderException(err);
        
        std::string ret(str_val);
        delete[] str_val;
        
        return ret;
    }
    
    
    std::vector<uint8_t> decodeBytes()
    {
        CborError err;
        size_t len;

        err = cbor_value_calculate_string_length(&m_it, &len);
        if (err != CborNoError)
            throw DecoderException(err);
        
        uint8_t* bytes = new uint8_t[len];
        err = cbor_value_copy_byte_string(&m_it, bytes, &len, &m_it);
        if (err != CborNoError)
            throw DecoderException(err);
        
        std::vector<uint8_t> ret(bytes, bytes+len);
        delete[] bytes;
        
        return ret;
    }
    
    
    bool decodeBool() 
    {   
        CborError err;
        bool value_buffer;
        
        err = cbor_value_get_boolean(&m_it, &value_buffer);
        if (err != CborNoError)
            throw DecoderException(err);
        
        next();
        
        return value_buffer;
    }
    
    
    float decodeFloat() 
    {   
        CborError err;
        float value_buffer;
        
        err = cbor_value_get_float(&m_it, &value_buffer);
        if (err != CborNoError)
            throw DecoderException(err);
        
        next();
        
        return value_buffer;
    }
    
    
    double decodeDouble() 
    {   
        CborError err;
        double value_buffer;
        
        err = cbor_value_get_double(&m_it, &value_buffer);
        if (err != CborNoError)
            throw DecoderException(err);
        
        next();
        
        return value_buffer;
    }
    
    
    template <typename T>
    uint64_t decode(TUint<T>& value) 
    {   
        value.set(decodeUint());
        return value.value;
    }

    template <typename T>
    int64_t decode(TInt<T>& value) 
    {   
        value.set(decodeInt());
        return value.value;
    }

    template <typename T>
    std::string decode(TString<T>& value) 
    {   
        value.set(decodeString());
        return value.value;
    }
    
    std::vector<uint8_t> decode(TBytes< std::vector<uint8_t> >& value) 
    {   
        value.set(decodeBytes());
        return value.value;
    }
    
    template <typename T>
    bool decode(TBool<T>& value) 
    {   
        value.set(decodeBool());
        return value.value;
    }
    
    template <typename T>
    float decode(TFloat<T>& value) 
    {   
        value.set(decodeFloat());
        return value.value;
    }
    
    template <typename T>
    double decode(TDouble<T>& value) 
    {   
        value.set(decodeDouble());
        return value.value;
    }
    
    
    bool isMap() { return cbor_value_is_map(&m_it); }
    
    bool isArray() { return cbor_value_is_array(&m_it); }
    
    bool isString() { return cbor_value_is_text_string(&m_it); }
    
    bool isBytes() { return cbor_value_is_byte_string(&m_it); }
    
    bool isInt() { return cbor_value_is_integer(&m_it); }
    
    bool isUint() { return cbor_value_is_unsigned_integer(&m_it); }
    
    bool isBool() { return cbor_value_is_boolean(&m_it); }
    
    bool isFloat() { return cbor_value_is_float(&m_it); }
    
    bool isDouble() { return cbor_value_is_double(&m_it); }
    
    bool isUndefined() { return cbor_value_is_undefined(&m_it); }
    
    bool isNull() { return cbor_value_is_null(&m_it); }
    
    size_t getArrayLength() 
    { 
        size_t len; 
        CborError err = cbor_value_get_array_length(&m_it, &len);
        if (err != CborNoError)
            throw DecoderException(err);
        return len;
    }
    
    size_t getMapLength() 
    { 
        size_t len; 
        CborError err = cbor_value_get_map_length(&m_it, &len);
        if (err != CborNoError)
            throw DecoderException(err);
        return len;
    }
    
    void next() 
    { 
        CborError err = cbor_value_advance(&m_it);
        if (err != CborNoError)
            throw DecoderException(err);
    }

    CborValue& getIterator() { return m_it; }
    
protected:

    CborValue& m_it;

};


//-----------------------------------------------------------------------------

class DecoderBuffer : public Decoder
{

public:
    
    DecoderBuffer(uint8_t* pBuffer, size_t buffer_size) 
        : Decoder(m_it), m_pBuffer(pBuffer)
    { 
        cbor_parser_init(pBuffer, buffer_size, 0, &m_parser, &m_it);
    }

    virtual ~DecoderBuffer() { }    
    
private:
    uint8_t* m_pBuffer;
    CborParser m_parser;
    CborValue m_it;
};


//-----------------------------------------------------------------------------

class InnerDecoder : public Decoder
{

public:
    
    InnerDecoder(Decoder& rOuter) : Decoder(m_it), m_rOuter(rOuter) {
        if (!cbor_value_is_container(&m_rOuter.getIterator()))
            throw DecoderException(CborErrorUnknownType);
        cbor_value_enter_container(&m_rOuter.getIterator(), &m_it);
    }
    
    virtual ~InnerDecoder() {
        cbor_value_leave_container(&m_rOuter.getIterator(), &m_it);
    }    
    
    Decoder& getOuter() { return m_rOuter; }
    
private:

    Decoder& m_rOuter;
    CborValue m_it;
};


//-----------------------------------------------------------------------------

inline Decoder& leave(Decoder& container)
{
    if (InnerDecoder* s = dynamic_cast<InnerDecoder*>(&container)) {
        Decoder& outer = s->getOuter();
        delete s;
        return outer;
    }

    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& enter(Decoder& container)
{
    InnerDecoder* pInner = new InnerDecoder(container);
    return *pInner;
}


//-----------------------------------------------------------------------------

inline Decoder& skip(Decoder& container)
{
    container.next();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, tDecoderFn op)
{
    return op(container);
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, std::string& value)
{
    value = container.decodeString();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, uint8_t& value)
{
    value = container.decodeUint();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, uint16_t& value)
{
    value = container.decodeUint();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, uint32_t& value)
{
    value = container.decodeUint();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, uint64_t& value)
{
    value = container.decodeUint();
    return container;
}

//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, int8_t& value)
{
    value = container.decodeInt();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, int16_t& value)
{
    value = container.decodeInt();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, int32_t& value)
{
    value = container.decodeInt();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, int64_t& value)
{
    value = container.decodeInt();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, bool& value)
{
    value = container.decodeBool();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, std::vector<uint8_t>& value)
{
    value = container.decodeBytes();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, float& value)
{
    value = container.decodeFloat();
    return container;
}


//-----------------------------------------------------------------------------

inline Decoder& operator >> (Decoder& container, double& value)
{
    value = container.decodeDouble();
    return container;
}

//-----------------------------------------------------------------------------

template <typename T>
inline Decoder& readValue(Decoder& container, T& value)
{
    container.decode(value);
    return container;
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RString(T& value)
{
    TString<T> target(value);
    return std::bind(readValue< TString<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RBytes(T& value)
{
    TBytes<T> target(value);
    return std::bind(readValue< TBytes<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RUint(T& value)
{
    TUint<T> target(value);
    return std::bind(readValue< TUint<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RInt(T& value)
{
    TInt<T> target(value);
    return std::bind(readValue< TInt<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RBool(T& value)
{
    TBool<T> target(value);
    return std::bind(readValue< TBool<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RFloat(T& value)
{
    TFloat<T> target(value);
    return std::bind(readValue< TFloat<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------

template <typename T>
inline tDecoderFn RDouble(T& value)
{
    TDouble<T> target(value);
    return std::bind(readValue< TFloat<T> >, 
            std::placeholders::_1, std::ref(target));
}


//-----------------------------------------------------------------------------


}

#endif /* TINYCBORWRAPPER_HPP_ */
