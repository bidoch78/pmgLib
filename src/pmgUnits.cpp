#include <pmgUnits.hpp>
#include <vector>
#include <map>
#include <variant>

using namespace pmg::core;

units::units() :
    _noUnit(toUnitId(unitID::NO_UNIT), "", toTypeId(unitTYPE::UNKNOWN), "", 0) {
}

units::units(bool loaddefault) :
     _noUnit(toUnitId(unitID::NO_UNIT), "", toTypeId(unitTYPE::UNKNOWN), "", 0) {

    if (!loaddefault) return;

    _units.emplace(toUnitId(unitID::UNKNOWN), 
                    unitData(toUnitId(unitID::UNKNOWN), unitNames::UNKNOWN, toTypeId(unitTYPE::UNKNOWN) , typeNames::UNKNOWN, 0));
    _units.emplace(toUnitId(unitID::SWITCH), 
                    unitData(toUnitId(unitID::SWITCH), unitNames::SWITCH, toTypeId(unitTYPE::BOOL), typeNames::BOOL, 0));        
    _units.emplace(toUnitId(unitID::BAR), 
                    unitData(toUnitId(unitID::BAR), unitNames::BAR, toTypeId(unitTYPE::PRESSURE), typeNames::PRESSURE, 2));
    _units.emplace(toUnitId(unitID::PSI), 
                    unitData(toUnitId(unitID::PSI), unitNames::PSI, toTypeId(unitTYPE::PRESSURE), typeNames::PRESSURE, 2));
    _units.emplace(toUnitId(unitID::CELSIUS), 
                    unitData(toUnitId(unitID::CELSIUS), unitNames::CELSIUS, toTypeId(unitTYPE::TEMPERATURE), typeNames::TEMPERATURE, 0));
    _units.emplace(toUnitId(unitID::FAHRENHEIT), 
                    unitData(toUnitId(unitID::FAHRENHEIT), unitNames::FAHRENHEIT, toTypeId(unitTYPE::TEMPERATURE), typeNames::TEMPERATURE, 0));

    _units.emplace(toUnitId(unitID::PERCENTAGE), 
                    unitData(toUnitId(unitID::PERCENTAGE), unitNames::PERCENTAGE, toTypeId(unitTYPE::PERCENTAGE), typeNames::PERCENTAGE, 0));
    _units.emplace(toUnitId(unitID::LAMBDA), 
                    unitData(toUnitId(unitID::LAMBDA), unitNames::LAMBDA, toTypeId(unitTYPE::FUELMIXTURE), typeNames::FUELMIXTURE, 2));
    _units.emplace(toUnitId(unitID::AFR), 
                    unitData(toUnitId(unitID::AFR), unitNames::AFR, toTypeId(unitTYPE::FUELMIXTURE), typeNames::FUELMIXTURE, 2));
    _units.emplace(toUnitId(unitID::VIRTUAL), 
                    unitData(toUnitId(unitID::VIRTUAL), unitNames::VIRTUAL, toTypeId(unitTYPE::VIRTUAL), typeNames::VIRTUAL, 0));

}
        
void units::addUnit(const units::unitData& unit) {
    _units.emplace(unit.id, unit);
}

std::vector<units::unitData> units::getUnits(typeId utype) const {

    std::vector<unitData> retUnit;

    for (const auto& [id, unit] : _units) {
        if (unit.type == utype) retUnit.push_back(unit);
    }

    return retUnit; 

}

std::map<typeId, std::vector<units::unitData>> units::getAllUnits() const {

    std::map<typeId, std::vector<unitData>> retUnits;

    for (const auto& [id, unit] : _units) {
        retUnits[unit.type].push_back(unit);
    }

    return retUnits;

}

const units::unitData& units::getUnit(unitId uid) const {

    auto it = _units.find(uid);
    if (it == _units.end()) return _noUnit;
    return it->second;

}

const units::unitData& units::getUnit(typeId utype, unitId uid) const {

    auto it = _units.find(uid);

    if (it == _units.end()) return _noUnit;
    if (it->second.type != utype) return _noUnit;

    return it->second;

}

bool units::noUnit(const units::unitData& unit) const { return unit.id == _noUnit.id; };

unitValue units::convertValue(const unitValue& value, unitId fromUnit, unitId toUnit) const {
    return transformUnit(value, fromUnit, toUnit);
}

unitValue units::transformUnit(const unitValue& value, unitId fromUnit, unitId toUnit) const {

    if (std::holds_alternative<std::monostate>(value)) return value;

    if (fromUnit == toUnit) return value;

    const unitData& fUnit = getUnit(fromUnit);
    const unitData& tUnit = getUnit(toUnit);

    if (noUnit(fUnit) || noUnit(tUnit)) return value;
    if (fUnit.type != tUnit.type) return value;

    //For all type we need a float value
    if (!std::holds_alternative<float>(value)) return value;

    float floatValue = std::get<float>(value);

    switch(fUnit.type) {

        case toTypeId(unitTYPE::TEMPERATURE):

            if (fromUnit == toUnitId(unitID::CELSIUS) && toUnit == toUnitId(unitID::FAHRENHEIT)) {
                return (floatValue * 9.0f / 5.0f) + 32.0f;
            }
            else if (fromUnit == toUnitId(unitID::FAHRENHEIT) && toUnit == toUnitId(unitID::CELSIUS)) {
                return (floatValue - 32.0f) * 5.0f / 9.0f;
            }

            break;
        
        case toTypeId(unitTYPE::FUELMIXTURE):

            if (fromUnit == toUnitId(unitID::LAMBDA) && toUnit == toUnitId(unitID::AFR)) {
                return floatValue * 14.7f;
            }
            else if (fromUnit == toUnitId(unitID::AFR) && toUnit == toUnitId(unitID::LAMBDA)) {
                return floatValue / 14.7f;
            }

            break;

        case toTypeId(unitTYPE::PRESSURE):
    
            if (fromUnit == toUnitId(unitID::BAR) && toUnit == toUnitId(unitID::PSI)) {
                return floatValue * 14.5038f;
            }
            else if (fromUnit == toUnitId(unitID::PSI) && toUnit == toUnitId(unitID::BAR)) {
                return floatValue * 0.0689476f;
            }

            break;

    }

    return value;

}

std::string units::toString(const unitValue& value) {

    return std::visit([](const auto& v) -> std::string {
        
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
            return "";
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return v;
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(v);
        }
        
    }, value);

}