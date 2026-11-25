#include <pmgBinaryWriter.hpp>
#include <cstdint>
#include <cstring>

using namespace pmg::core;

binaryWriter::binaryWriter(uint8_t* buffer, size_t capacity): _buffer(buffer), _capacity(capacity), _position(0) {
    reset();
}

void binaryWriter::reset() {
    _position = 0;
    _bufferIsOk = (_buffer != nullptr && _capacity > 0);
}

bool binaryWriter::hasEnoughSpace(size_t length) {
    if (!_bufferIsOk || length > (_capacity - _position)) return false;
    return true;
}

size_t binaryWriter::position() const { return _position; }
size_t binaryWriter::size() const { return _position; }
size_t binaryWriter::capacity() const { return _capacity; }
size_t binaryWriter::remaining() const { return _capacity - _position; }

// ---------------------------------------------------------
// Raw bytes
// ---------------------------------------------------------

bool binaryWriter::writeBytes(const uint8_t* data, size_t length) {
    
    if (data == nullptr && length != 0) return false;

    if (!hasEnoughSpace(length)) return false;

    if (length != 0) {
        std::memcpy(_buffer + _position, data, length);
        _position += length;
    }

    return true;

}

// ---------------------------------------------------------
// Integer primitives
// Little Endian
// ---------------------------------------------------------

bool binaryWriter::writeUInt8(uint8_t value) {
    if (!hasEnoughSpace(1)) return false;
    _buffer[_position++] = value;
    return true;

}

bool binaryWriter::writeUInt16(uint16_t value) {
    if (!hasEnoughSpace(2)) return false;
    _buffer[_position++] = static_cast<uint8_t>(value);
    _buffer[_position++] = static_cast<uint8_t>(value >> 8);
    return true;
}

bool binaryWriter::writeUInt32(uint32_t value) {
    if (!hasEnoughSpace(4)) return false;
    _buffer[_position++] = static_cast<uint8_t>(value);
    _buffer[_position++] = static_cast<uint8_t>(value >> 8);
    _buffer[_position++] = static_cast<uint8_t>(value >> 16);
    _buffer[_position++] = static_cast<uint8_t>(value >> 24);
    return true;
}

bool binaryWriter::writeInt32(int32_t value) { return writeUInt32(static_cast<uint32_t>(value)); }

bool binaryWriter::writeBool(bool value) { return writeUInt8(value ? 1 : 0); }

// IEEE-754 / Little Endian
bool binaryWriter::writeFloat(float value) {
    //Just in case => normally we must have 4 octets = float = uint32_t
    static_assert(sizeof(float) == sizeof(uint32_t), "BinaryWriter requires 32-bit float");
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return writeUInt32(bits);
}

////////////////////////////
////////////////////////////

// ---------------------------------------------------------
// String => MAX SIZE 65 535
//
// Format:[length:uint16][UTF-8/raw bytes...]
// ---------------------------------------------------------

bool binaryWriter::writeString(const char* value, size_t length) {
    if (length > UINT16_MAX || !hasEnoughSpace(sizeof(uint16_t) + length)) return false;
    writeUInt16(static_cast<uint16_t>(length));
    return writeBytes(reinterpret_cast<const uint8_t*>(value), length);
}

////////////////////////////
////////////////////////////

size_t binaryWriter::sizeOf(const unitValue& uValue) {
    return std::visit([](const auto& arg) -> size_t {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg.size() + 1; // [length][value...octet]
        } else {
            return sizeof(T); // 1 for bool, 4 for int32_t / float
        }
    }, uValue);
}

size_t binaryWriter::sizeOf(const std::vector<std::pair<unitId, unitValue>>& uValues, bool addcountfield) {

    size_t s = 0;

    for (const auto& [uId, uValue] : uValues) {
        s += binaryWriter::sizeOf(uValue) + 2; // sizeof value + 1 for unitId & 1 for unitType
    }

    return s + (addcountfield ? 1: 0);

}

size_t binaryWriter::sizeOf(const std::vector<sensor::sensorData>& uSensors, bool addcountfield) {

    size_t s = 0;
    
    return s + (addcountfield ? 1: 0);

}