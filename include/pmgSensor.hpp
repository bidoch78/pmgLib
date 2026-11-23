#pragma once

#include <pmgUnits.hpp>

namespace pmg::core {

    using sensorId = uint32_t;

    template<typename T>
    static constexpr sensorId toSensorId(T id) {
        return static_cast<sensorId>(id);
    };

    enum class sensorRate : uint32_t {
        LOW_RATE = 5000,
        MEDIUM_RATE = 3000,
        HIGH_RATE = 1000
    };
    
    enum class sensorID : sensorId {
        ENGINE_AIRPRESSURE = 1,
        ENGINE_COOLANTTEMP = 2,
        ENGINE_WB_LEFT = 3,
        ENGINE_WB_RIGHT = 4,
        ENGINE_OIL_PRESSURE = 5,
        FUEL_PRESSURE = 6,
        FUEL_LEVEL = 7,
        TRANS_OIL_TEMP = 8,
        TRANS_SOL_A = 9,
        TRANS_SOL_B = 10,
        TRANS_SOL_LC = 11,
        TRANS_GEARSEL_NA = 12,
        TRANS_GEARSEL_1 = 13,
        TRANS_GEARSEL_2 = 14,
        TRANS_GEARSEL_D = 15,
        TRANS_GEARSEL_R = 16,
        ENV_DAYNIGHT = 17,
        FUEL_LEVEL_LOW = 18,
        TRANS_GEARSEL = 19,
        TRANS_SELECTOR = 20
    };

    class sensor {

        private:

            uint32_t _dataLastRead = 0;
            uint32_t _dataCountRead = 0;

            sensorId _id;
            std::string _name;
            std::string _category;
            units::unitData _unit;
            
            uint32_t _rate;

            float _readVoltage;
            unitValue _readValue;
            uint32_t _srate;

        public:

            sensor(sensorId sId, const char* sName, const char* sCat, const units::unitData& sUnit, uint32_t sRate);
            ~sensor() = default;

            sensorId getId() const;
            uint32_t getSampleRate() const;
            const std::string& getName() const;
            const std::string& getCategory() const;
            const units::unitData getUnit() const;

            bool isVirtual() const;
            uint32_t getLastRead() const;
            uint32_t getCountRead() const;
            void resetCountRead();
            void setSampleRate(uint32_t sRate);
            void read(uint32_t w);

            void setValue(const float volt, const unitValue& val);
            const unitValue& getValue() const;
            std::string getStringValue();

    };

}