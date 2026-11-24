#include <unity.h>

#include <pmgSensor.hpp>
#include <pmgUnits.hpp>
#include <iostream>

using namespace pmg::core;

void testUnitConversion()
{

    units allUnits(true);

    unitValue value = allUnits.convertValue(1.0f, toUnitId(unitID::LAMBDA), toUnitId(unitID::AFR));

    std::string message = "Lambda: 1 - AFR: " + units::toString(value);
    TEST_MESSAGE(message.c_str());

    TEST_ASSERT_TRUE(std::get<float>(value) == 14.7f);
    
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(testUnitConversion);

    return UNITY_END();
    
}