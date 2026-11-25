#pragma once

#include <cstdint>
#include <cstddef>
#include <pmgUnits.hpp>
#include <pmgSensor.hpp>

namespace pmg::core {

    class binaryWriter {
        
        private:

            uint8_t* _buffer;
            size_t _capacity;
            size_t _position;

            bool _bufferIsOk;

            bool hasEnoughSpace(size_t length);

        public:

            binaryWriter(uint8_t* buffer, size_t capacity);

            void reset();
            
            size_t position() const;
            size_t size() const;
            size_t capacity() const;
            size_t remaining() const;

            // Raw bytes
            bool writeBytes(const uint8_t* data, size_t length);

            // ---------------------------------------------------------
            // Integer primitives
            // Little Endian
            // ---------------------------------------------------------
            bool writeUInt8(uint8_t value);
            bool writeUInt16(uint16_t value);
            bool writeUInt32(uint32_t value);
            bool writeInt32(int32_t value);
            bool writeFloat(float value); // IEEE-754 / Little Endian
            bool writeBool(bool value);

            // ---------------------------------------------------------
            // String => write [length:uint16][UTF-8/raw bytes...]
            // ---------------------------------------------------------
            bool writeString(const char* value, size_t length);

            static size_t sizeOf(const unitValue& uValue);
            static size_t sizeOf(const std::vector<std::pair<unitId, unitValue>>& uValues, bool addcountfield = false);
            static size_t sizeOf(const std::vector<sensor::sensorData>& uSensors, bool addcountfield = false);

    };    

}