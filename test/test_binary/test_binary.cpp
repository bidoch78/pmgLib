#include <unity.h>

#include <pmgSensor.hpp>
#include <pmgUnits.hpp>
#include <pmgBinaryWriter.hpp>
#include <iostream>

using namespace pmg::core;

void testsizeof_unitValue()
{

    units allUnits(true);

    std::vector<std::pair<unitValue, size_t>> values = {
        { unitValue(1.0456f), 4 },
        { unitValue(42),      4 },
        { unitValue(true),    1 },
        { unitValue("demain"), 7}
    };
    
    for (const auto& [value, expectedSize] : values)
    {
        size_t size = binaryWriter::sizeOf(value);
        
        std::string message = units::toString(value) + " - Expected: " + std::to_string(expectedSize) + ", actual: " + std::to_string(size);
        
        TEST_MESSAGE(message.c_str());
        TEST_ASSERT_EQUAL_size_t(expectedSize, size);
    }

}

void testsizeof_unitValues() {

    std::vector<std::pair<unitId, unitValue>> values = {
        { toUnitId(unitID::CELSIUS), unitValue(80) },
        { toUnitId(unitID::FAHRENHEIT), unitValue(100) }
    };

    size_t size = binaryWriter::sizeOf(values, true); // (4+2)*2 + 1 

    std::string message = "double(int) - Expected: " + std::to_string(size);

    TEST_MESSAGE(message.c_str());
    TEST_ASSERT_EQUAL_size_t(size, 13);

    //Impossible but for testing purpose
    values = {
        { toUnitId(unitID::CELSIUS), unitValue(true) },
        { toUnitId(unitID::FAHRENHEIT), unitValue(false) },
        { toUnitId(unitID::VIRTUAL), unitValue("demain") }
    };

    size = binaryWriter::sizeOf(values, true); // (1+2)*2 + (7+2) + 1 
    
    message = "double(bool) - Expected: " + std::to_string(size);

    TEST_MESSAGE(message.c_str());
    TEST_ASSERT_EQUAL_size_t(size, 16);

}

void testsizeof_sensorValues() {

    units allUnits(true);
    sensor s(toSensorId(sensorID::ENGINE_AIRPRESSURE), "", "", allUnits.getUnit(toUnitId(unitID::BAR)), 1000);
    s.setValue(2, 1.0f);

    sensor::sensorData sd = s.getSensorData();

    std::string message = "ID: " + std::to_string(sd.id);

    for (const auto& [uid, uvalue] : sd.values) {
        message += " u:" + std::to_string(uid) + ", v:" + units::toString(uvalue);    
    }

    TEST_MESSAGE(message.c_str());

    sd = s.getSensorData(allUnits, { toUnitId(unitID::BAR), toUnitId(unitID::PSI) });

    message = "ID: " + std::to_string(sd.id);

    for (const auto& [uid, uvalue] : sd.values) {
        message += "| u:" + std::to_string(uid) + ", v:" + units::toString(uvalue);    
    }

    TEST_MESSAGE(message.c_str());

    TEST_ASSERT_TRUE(true);

}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(testsizeof_unitValue);
    RUN_TEST(testsizeof_unitValues);
    RUN_TEST(testsizeof_sensorValues);

    return UNITY_END();
    
}