#pragma once

#include <string>
#include <../json.hpp>

class PreferenceManager {
public:
    static PreferenceManager& getInstance();

    void load();
    void save() const;

    void set(const std::string& key, const nlohmann::json& value);
    [[nodiscard]] nlohmann::json get(const std::string& key, const nlohmann::json& defaultValue = nullptr) const;
    [[nodiscard]] bool has(const std::string& key) const;
    void remove(const std::string& key);

    void setFileName(const std::string& filename);

    PreferenceManager(const PreferenceManager&) = delete;
    PreferenceManager& operator=(const PreferenceManager&) = delete;

    friend std::ostream& operator<<(std::ostream& os, const PreferenceManager& pm);

private:
    PreferenceManager() = default;

    std::string fileName = "preferences.json";
    nlohmann::json preferences;
};
