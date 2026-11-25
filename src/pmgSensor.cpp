#include <pmgSensor.hpp>
#include <pmgUnits.hpp>
// #include <Arduino.h>

using namespace pmg::core;

sensor::sensor(sensorId sId, const char* sName, const char* sCat, const units::unitData& sUnit, uint32_t sRate)
          : _id(sId), _name(sName), _category(sCat), _unit(sUnit), _rate(sRate) {

};

sensorId sensor::getId() const { return _id; };
uint32_t sensor::getSampleRate() const { return _rate; };
const std::string& sensor::getName() const { return _name; };
const std::string& sensor::getCategory() const { return _category; };

const units::unitData sensor::getUnit() const { return _unit; };

bool sensor::isVirtual() const { return _unit.type == typeId(unitTYPE::VIRTUAL); };

uint32_t sensor::getLastRead() const { return _dataLastRead; };
uint32_t sensor::getCountRead() const { return _dataCountRead; };
void sensor::resetCountRead() { _dataCountRead = 0; };
void sensor::setSampleRate(uint32_t sRate) { _srate = sRate; };
void sensor::read(uint32_t w) { _dataLastRead = w; _dataCountRead++; };

void sensor::setValue(const float volt, const unitValue& val) {

    _dataCountRead++;
    _readVoltage = volt;
    _readValue = val;

};

const unitValue& sensor::getValue() const { return _readValue; }

std::string sensor::getStringValue() { return units::toString(_readValue); }

sensor::sensorData sensor::getSensorData() const {

    sensorData data;

    data.id = _id;
    data.rate = _srate;
    data.voltage = _readVoltage;
    data.values.emplace_back(_unit.id, _readValue);   
   
    return data;

}

sensor::sensorData sensor::getSensorData(const units& uref, const std::vector<unitId>& convert) const {
    
    sensorData data;

    data.id = _id;
    data.rate = _srate;
    data.voltage = _readVoltage;

    if (convert.size() == 0) {
        data.values.emplace_back(_unit.id, _readValue);   
    }
    else {
        
        for (const auto& convToUnit : convert) {
            data.values.emplace_back(convToUnit, uref.convertValue( _readValue, _unit.id, convToUnit));   
        }

    }

    return data;

}