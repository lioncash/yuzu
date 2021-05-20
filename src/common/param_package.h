// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <initializer_list>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace Common {

/// A string-based key-value container supporting serializing to and deserializing from a string
class ParamPackage {
public:
    struct StringHash {
        using is_transparent = std::true_type;

        size_t operator()(std::string_view txt) const noexcept {
            return std::hash<std::string_view>{}(txt);
        }
    };
    struct StringEqual {
        using is_transparent = std::true_type;

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
            return lhs == rhs;
        }
    };
    using DataType = std::unordered_map<std::string, std::string, StringHash, StringEqual>;

    ParamPackage() = default;
    explicit ParamPackage(const std::string& serialized);
    ParamPackage(std::initializer_list<DataType::value_type> list);
    ParamPackage(const ParamPackage& other) = default;
    ParamPackage(ParamPackage&& other) noexcept = default;

    ParamPackage& operator=(const ParamPackage& other) = default;
    ParamPackage& operator=(ParamPackage&& other) = default;

    [[nodiscard]] std::string Serialize() const;
    [[nodiscard]] std::string Get(std::string_view key, std::string_view default_value) const;
    [[nodiscard]] int Get(std::string_view key, int default_value) const;
    [[nodiscard]] float Get(std::string_view key, float default_value) const;
    void Set(const std::string& key, std::string value);
    void Set(const std::string& key, int value);
    void Set(const std::string& key, float value);
    [[nodiscard]] bool Has(std::string_view key) const;
    void Erase(const std::string& key);
    void Clear();

private:
    DataType data;
};

} // namespace Common
