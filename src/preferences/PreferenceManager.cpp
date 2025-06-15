#include "PreferenceManager.h"
#include <fstream>

PreferenceManager& PreferenceManager::getInstance() {
    static PreferenceManager instance;
    return instance;
}

void PreferenceManager::setFileName(const std::string& filename) {
    fileName = filename;
}

void PreferenceManager::load() {
    if (std::ifstream file(fileName); file.is_open()) {
        file >> preferences;
    }
}

void PreferenceManager::save() const {
    if (std::ofstream file(fileName); file.is_open()) {
        file << preferences.dump(4);  // joli format JSON :)
    }
}

void PreferenceManager::set(const std::string& key, const nlohmann::json& value) {
    preferences[key] = value;
}

nlohmann::json PreferenceManager::get(const std::string& key, const nlohmann::json& defaultValue) const {
    if (preferences.contains(key)) {
        return preferences.at(key);
    }
    return defaultValue;
}

bool PreferenceManager::has(const std::string& key) const {
    return preferences.contains(key);
}

void PreferenceManager::remove(const std::string& key) {
    preferences.erase(key);
}

std::ostream& operator<<(std::ostream& os, const PreferenceManager& pm) {
    os << pm.preferences.dump(4);
    return os;
}