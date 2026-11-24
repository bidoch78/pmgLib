#pragma once

#include <vector>
#include <variant>
#include <string>
#include <map>

namespace pmg::core {

    namespace typeNames {

        inline constexpr const char* UNKNOWN     = "-";
        inline constexpr const char* PRESSURE    = "pressure";
        inline constexpr const char* TEMPERATURE = "temperature";
        inline constexpr const char* FUELMIXTURE = "fuelmixture";
        inline constexpr const char* PERCENTAGE  = "percentage";
        inline constexpr const char* BOOL        = "bool";
        inline constexpr const char* VIRTUAL     = "virtual";

    }

    namespace unitNames {

        inline constexpr const char* UNKNOWN         = "-";
        inline constexpr const char* SWITCH         = "SW";
        inline constexpr const char* BAR            = "Bar";
        inline constexpr const char* PSI            = "PSI";
        inline constexpr const char* CELSIUS        = "°C";
        inline constexpr const char* FAHRENHEIT     = "°F";
        inline constexpr const char* PERCENTAGE     = "%";
        inline constexpr const char* LAMBDA         = "λ";
        inline constexpr const char* AFR            = "AFR";
        inline constexpr const char* VIRTUAL        = "Virtual";

    }

    using unitId = uint32_t;
    using typeId = uint32_t;
    using unitValue = std::variant<std::monostate, bool, int32_t, float, std::string>;

    template<typename T>
    static constexpr unitId toUnitId(T id) { return static_cast<unitId>(id); }

    template<typename T>
    static constexpr typeId toTypeId(T id) { return static_cast<typeId>(id); }

    enum class unitTYPE: typeId {
        UNKNOWN = 0,
        PRESSURE = 1,
        TEMPERATURE = 2,
        FUELMIXTURE = 3,
        PERCENTAGE = 4,
        BOOL = 5,
        VIRTUAL = 6
    };

    enum class unitID: unitId {
        NO_UNIT = 0,
        UNKNOWN = 1,
        SWITCH = 2,
        BAR = 3,
        PSI = 4,
        CELSIUS = 5,
        FAHRENHEIT = 6,
        PERCENTAGE = 7,
        LAMBDA = 8,
        AFR = 9,
        VIRTUAL = 10
    };

    class units {

        public:

            struct unitData {
                unitId id;
                const char* unitName;
                typeId type;
                const char* typeName;
                uint8_t decimals;
                unitData(unitId uid, const char* uname, typeId utype, const char* tname, uint8_t udecimal): id(uid), unitName(uname), type(utype), typeName(tname), decimals(udecimal) {};
            };

        private:          

            unitData _noUnit;
            std::map<unitId, unitData> _units; 
 
        public:

            units();
            units(bool loaddefault);
            ~units() = default;

            void addUnit(const unitData& unit);

            std::vector<unitData> getUnits(typeId utype) const;
            std::map<typeId, std::vector<unitData>> getAllUnits() const;

            const unitData& getUnit(unitId uid) const;
            const unitData& getUnit(typeId utype, unitId uid) const;
            
            bool noUnit(const unitData& unit) const;

            unitValue convertValue(const unitValue& value, unitId fromUnit, unitId toUnit) const;
            unitValue transformUnit(const unitValue& value, unitId fromUnit, unitId toUnit) const;

            static bool hasValue(const unitValue& value) { return !std::holds_alternative<std::monostate>(value); }

            template<typename T>
            static const T* getValue(const unitValue& value) { return std::get_if<T>(&value); }

            template<typename T>
            static T getValue(const unitValue& value, T defaultValue) {
                if (const T* v = std::get_if<T>(&value)) return *v;
                return defaultValue;
            }

            static std::string toString(const unitValue& value);

    };

}