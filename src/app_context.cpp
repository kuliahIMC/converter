#include "app_context.h"

AppContext &AppContext::instance()
{
    static AppContext ctx;
    return ctx;
}

AppContext::AppContext()
    : settingsService_(std::make_unique<SettingsService>())
{
}

SettingsService &AppContext::settings()
{
    return *settingsService_;
}

const SettingsService &AppContext::settings() const
{
    return *settingsService_;
}

