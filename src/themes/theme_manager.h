
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class QWidget;

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    explicit ThemeManager(QObject *parent = nullptr);

    QStringList availableThemes() const;

    QString currentTheme() const;
    QString themeImagePath(const QString &theme) const;

    void applyTheme(const QString &theme);
    void applyToWidget(QWidget *widget);

signals:
    void themeChanged(const QString &theme);

private:
    QString currentTheme_;
    QString getThemeStyleSheet(const QString &theme) const;
    void loadSavedTheme();
    void saveTheme(const QString &theme);
};
