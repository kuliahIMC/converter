#pragma once

#include "services/settings_service.h"

#include <memory>

class AppContext {
public:
    static AppContext &instance();

    SettingsService &settings();
    const SettingsService &settings() const;

private:
    AppContext();
    ~AppContext() = default;

    AppContext(const AppContext &) = delete;
    AppContext &operator=(const AppContext &) = delete;

    std::unique_ptr<SettingsService> settingsService_;
};

