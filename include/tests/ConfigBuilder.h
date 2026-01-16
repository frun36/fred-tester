#pragma once

#include "tests/TesterConfig.h"

namespace tests {

class ConfigBuilder {
    struct BaseEntry {
        enum class Type {
            Bool,
            String,
            Double,
            OptionalString,
            Boards,
            BadChannelMap,
            Unknown
        } type;

        BaseEntry(Type type) : type(type) {}

        virtual ~BaseEntry() = default;
    };

    template<typename T>
    struct ConfigEntry: public BaseEntry {
        T val;

        ConfigEntry(Type type, T val = T()) : BaseEntry(type), val(val) {}
    };

    using Options =
        std::vector<std::pair<std::string, std::unique_ptr<BaseEntry>>>;

    template<typename T>
    struct TypeMapper {
        static constexpr BaseEntry::Type value = BaseEntry::Type::Unknown;
    };

  private:
    enum class Section { Setup, Test, Cleanup };

    template<typename T>
    ConfigBuilder& option(std::string key, Section section) {
        auto& vec = section == Section::Setup ? setupOptions
            : section == Section::Test        ? testsOptions
                                              : cleanupOptions;

        vec.push_back(
            {key, std::make_unique<ConfigEntry<T>>(TypeMapper<T>::value)}
        );
        return *this;
    }

    template<typename T>
    Result<T> get(std::string key, Section section) {
        auto& vec = section == Section::Setup ? setupOptions
            : section == Section::Test        ? testsOptions
                                              : cleanupOptions;

        auto it = std::find_if(vec.begin(), vec.end(), [&](const auto& pair) {
            return pair.first == key;
        });

        if (it == vec.end()) {
            return err("Key '{}' not found", key);
        }

        auto* entry = dynamic_cast<ConfigEntry<T>*>(it->second.get());

        if (!entry) {
            return err("Type mismatch for key '{}'", key);
        }

        return entry->val;
    }

    template<typename T>
    Result<void> set(std::string key, Section section, T value) {
        auto& vec = section == Section::Setup ? setupOptions
            : section == Section::Test        ? testsOptions
                                              : cleanupOptions;

        auto it = std::find_if(vec.begin(), vec.end(), [&](const auto& pair) {
            return pair.first == key;
        });

        if (it == vec.end()) {
            return err("Key '{}' not found", key);
        }

        auto* entry = dynamic_cast<ConfigEntry<T>*>(it->second.get());

        if (!entry) {
            return err("Type mismatch for key '{}'", key);
        }

        entry->val = std::move(value);

        return {};
    }

  public:
    template<typename T>
    ConfigBuilder& setup(std::string key) {
        return option<T>(std::move(key), Section::Setup);
    }

    template<typename T>
    ConfigBuilder& test(std::string key) {
        return option<T>(std::move(key), Section::Test);
    }

    template<typename T>
    ConfigBuilder& cleanup(std::string key) {
        return option<T>(std::move(key), Section::Cleanup);
    }

    template<typename T>
    Result<T> setupTryGet(std::string key) {
        return get<T>(key, Section::Setup);
    }

    template<typename T>
    Result<T> testTryGet(std::string key) {
        return get<T>(key, Section::Test);
    }

    template<typename T>
    Result<T> cleanupTryGet(std::string key) {
        return get<T>(key, Section::Cleanup);
    }

    template<typename T>
    T setupGetOr(std::string key, T defaultValue) {
        auto result = get<T>(key, Section::Setup);
        return result ? result.value() : defaultValue;
    }

    template<typename T>
    T testGetOr(std::string key, T defaultValue) {
        auto result = get<T>(key, Section::Test);
        return result ? result.value() : defaultValue;
    }

    template<typename T>
    T cleanupGetOr(std::string key, T defaultValue) {
        auto result = get<T>(key, Section::Cleanup);
        return result ? result.value() : defaultValue;
    }

    template<typename T>
    void setupSet(std::string key, T value) {
        set<T>(key, Section::Setup, std::move(value));
    }

    template<typename T>
    void testSet(std::string key, T value) {
        set<T>(key, Section::Test, std::move(value));
    }

    template<typename T>
    void cleanupSet(std::string key, T value) {
        set<T>(key, Section::Cleanup, std::move(value));
    }

    template<typename T>
    Result<std::optional<T>> parseOptional(
        const toml::table& t,
        std::string name
    ) {
        auto* node = t.get(name);
        if (!node)
            return std::optional<T> {};
        if (node->is_boolean()) {
            if (auto b = node->value<bool>(); b && !*b)
                return std::optional<T> {};
        } else if (auto str = node->value<T>()) {
            return *str;
        } else {
            return err("{} must be {} or false", name, typeid(T).name());
        }
        return std::optional<T> {};
    }

    template<typename T>
    Result<T> parse(const toml::table& tbl, std::string key) {
        auto val = tbl.get_as<T>(key);
        if (!val)
            return err("{} must be {}", key, typeid(T).name());
        return **val;
    }

    Result<TesterConfig::Boards> parseBoards(
        const toml::table& t,
        std::string name
    );

    Result<std::optional<TesterConfig::BadChannelMapConfig>> parseBadChannelMap(
        const toml::table& t,
        std::string name
    );

    Result<void> handleTable(const toml::table& t, Options& o);

    Result<TesterConfig> parseToml(const toml::table& t);

  private:
    Options setupOptions;
    Options testsOptions;
    Options cleanupOptions;
};

template<>
struct ConfigBuilder::TypeMapper<bool> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::Bool;
};

template<>
struct ConfigBuilder::TypeMapper<double> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::Double;
};

template<>
struct ConfigBuilder::TypeMapper<std::string> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::String;
};

template<>
struct ConfigBuilder::TypeMapper<std::optional<std::string>> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::OptionalString;
};

template<>
struct ConfigBuilder::TypeMapper<TesterConfig::Boards> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::Boards;
};

template<>
struct ConfigBuilder::TypeMapper<std::optional<TesterConfig::BadChannelMapConfig>> {
    static constexpr BaseEntry::Type value = BaseEntry::Type::BadChannelMap;
};

}; // namespace tests
