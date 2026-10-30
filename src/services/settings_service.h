#pragma once

#include <QJsonObject>
#include <QString>

class SettingsService {
public:
    SettingsService();

    void load();
    void save();

    // Nilai generic
    QJsonValue get(const QString &key, const QJsonValue &defaultValue = QJsonValue()) const;
    void set(const QString &key, const QJsonValue &value);

    // Section generic (misal "output", "conversion")
    QJsonObject getSection(const QString &section) const;
    void setSection(const QString &section, const QJsonObject &value);

    // Convenience getters/setters untuk settings utama
    QString getTheme() const;
    void setTheme(const QString &theme);

    QString getOutputFolder() const;
    void setOutputFolder(const QString &folder);

    bool getRememberLastFolder() const;
    void setRememberLastFolder(bool remember);

    int getImageQuality() const;
    void setImageQuality(int quality);

    QString getVideoQuality() const;
    void setVideoQuality(const QString &quality);

    QString getAudioQuality() const;
    void setAudioQuality(const QString &quality);

    static QJsonObject defaultSettings();

private:
    QString settingsPath_;
    QString legacySettingsPath_;
    QJsonObject settings_;

    static QJsonObject mergeDefaults(const QJsonObject &defaults, const QJsonObject &loaded);
};

