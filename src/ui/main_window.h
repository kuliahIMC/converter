#pragma once

#include <QMainWindow>
#include <QList>
#include <QPixmap>
#include <QString>

#include "../themes/theme_manager.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class QComboBox;
class QPaintEvent;
class QResizeEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QStackedWidget *pages_ = nullptr;
    QLabel *pageTitle_ = nullptr;
    QList<QPushButton *> navButtons_;

    ThemeManager *themeManager_ = nullptr;
    QComboBox *themeSelector_ = nullptr;

    QPixmap bgOriginal_;
    QPixmap bgScaled_;
    QString bgPath_;

    void setupUI();
    void setupThemeSelector();
    void navigateTo(int index);
    void updateBackground();
};