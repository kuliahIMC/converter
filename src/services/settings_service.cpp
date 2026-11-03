#include "settings_service.h"
#include "../utils/app_paths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

QJsonObject SettingsService::defaultSettings()
{
    QJsonObject outputObj;
    outputObj["folder"] = ""; // kompatibilitas pengaturan lama
    outputObj["image_folder"] = "";
    outputObj["video_folder"] = "";
    outputObj["remember_last_folder"] = true;

    QJsonObject conversionObj;
    conversionObj["image_quality"] = 90;
    conversionObj["video_quality"] = "medium";
    conversionObj["audio_quality"] = "high";

    QJsonObject root;
    root["theme"] = "Dark";
    root["output"] = outputObj;
    root["conversion"] = conversionObj;

    return root;
}

SettingsService::SettingsService()
{
    const QString dataDir = AppPaths::getDataDir();
    settingsPath_ = QDir(dataDir).filePath("settings.json");
    legacySettingsPath_ = QDir(AppPaths::getBundleDataDir()).filePath("settings.json");

    load();
}

QJsonObject SettingsService::mergeDefaults(const QJsonObject &defaults, const QJsonObject &loaded)
{
    QJsonObject result = defaults;

    for (auto it = loaded.begin(); it != loaded.end(); ++it) {
        const QString key = it.key();
        const QJsonValue val = it.value();

        if (val.isObject() && result.contains(key) && result[key].isObject()) {
            result[key] = mergeDefaults(result[key].toObject(), val.toObject());
        } else {
            result[key] = val;
        }
    }

    return result;
}

void SettingsService::load()
{
    QString sourcePath = settingsPath_;

    if (!QFileInfo::exists(sourcePath)) {
        if (QFileInfo::exists(legacySettingsPath_) && legacySettingsPath_ != settingsPath_) {
            sourcePath = legacySettingsPath_;
        } else {
            settings_ = defaultSettings();
            save();
            return;
        }
    }

    QFile file(sourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        settings_ = defaultSettings();
        save();
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        settings_ = defaultSettings();
        save();
        return;
    }

    settings_ = mergeDefaults(defaultSettings(), doc.object());

    // Jika dibaca dari bundle/legacy, simpan salinan ke writable user data path
    if (sourcePath != settingsPath_) {
        save();
    }
}

void SettingsService::save()
{
    const QFileInfo fi(settingsPath_);
    QDir().mkpath(fi.absolutePath());

    QFile file(settingsPath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        const QJsonDocument doc(settings_);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

QJsonValue SettingsService::get(const QString &key, const QJsonValue &defaultValue) const
{
    if (settings_.contains(key)) {
        return settings_.value(key);
    }
    return defaultValue;
}

void SettingsService::set(const QString &key, const QJsonValue &value)
{
    settings_[key] = value;
    save();
}

QJsonObject SettingsService::getSection(const QString &section) const
{
    if (settings_.contains(section) && settings_[section].isObject()) {
        return settings_[section].toObject();
    }
    return QJsonObject();
}

void SettingsService::setSection(const QString &section, const QJsonObject &value)
{
    settings_[section] = value;
    save();
}

QString SettingsService::getTheme() const
{
    return get("theme", "Dark").toString();
}

void SettingsService::setTheme(const QString &theme)
{
    set("theme", theme);
}

QString SettingsService::getOutputFolder() const
{
    return getSection("output").value("folder").toString();
}

void SettingsService::setOutputFolder(const QString &folder)
{
    QJsonObject section = getSection("output");
    section["folder"] = folder;
    setSection("output", section);
}

QString SettingsService::getImageOutputFolder() const
{
    const QJsonObject output = getSection("output");
    const QString folder = output.value("image_folder").toString();

    // Gunakan folder lama jika pengaturan baru belum pernah diisi.
    return folder.isEmpty()
        ? output.value("folder").toString()
        : folder;
}

void SettingsService::setImageOutputFolder(const QString &folder)
{
    QJsonObject output = getSection("output");
    output["image_folder"] = folder;
    setSection("output", output);
}

QString SettingsService::getVideoOutputFolder() const
{
    const QJsonObject output = getSection("output");
    const QString folder = output.value("video_folder").toString();

    return folder.isEmpty()
        ? output.value("folder").toString()
        : folder;
}

void SettingsService::setVideoOutputFolder(const QString &folder)
{
    QJsonObject output = getSection("output");
    output["video_folder"] = folder;
    setSection("output", output);
}

bool SettingsService::getRememberLastFolder() const
{
    return getSection("output").value("remember_last_folder").toBool(true);
}

void SettingsService::setRememberLastFolder(bool remember)
{
    QJsonObject section = getSection("output");
    section["remember_last_folder"] = remember;
    setSection("output", section);
}

int SettingsService::getImageQuality() const
{
    return getSection("conversion").value("image_quality").toInt(90);
}

void SettingsService::setImageQuality(int quality)
{
    QJsonObject section = getSection("conversion");
    section["image_quality"] = quality;
    setSection("conversion", section);
}

QString SettingsService::getVideoQuality() const
{
    return getSection("conversion").value("video_quality").toString("medium");
}

void SettingsService::setVideoQuality(const QString &quality)
{
    QJsonObject section = getSection("conversion");
    section["video_quality"] = quality;
    setSection("conversion", section);
}

QString SettingsService::getAudioQuality() const
{
    return getSection("conversion").value("audio_quality").toString("high");
}

void SettingsService::setAudioQuality(const QString &quality)
{
    QJsonObject section = getSection("conversion");
    section["audio_quality"] = quality;
    setSection("conversion", section);
}
