#pragma once

#include <QStringList>

class QWidget;
class ThemeManager;

class FeedbackDialog
{
public:
    static void showResult(
        QWidget *parent,
        ThemeManager *themeManager,
        int successCount,
        int failedCount,
        int skippedCount,
        const QStringList &errors,
        bool cancelled = false
    );
};