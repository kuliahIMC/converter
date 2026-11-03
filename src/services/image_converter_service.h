#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class QTemporaryDir;

class ImageConverterService : public QObject
{
    Q_OBJECT

public:
    explicit ImageConverterService(QObject *parent = nullptr);
    ~ImageConverterService() override;

    bool isRunning() const;

public slots:
    void convert(
        const QString &inputPath,
        const QString &outputPath,
        const QString &format
    );

    void cancel();

signals:
    void started();
    void finished(const QString &outputPath);
    void failed(const QString &errorMessage);
    void cancelled();

private slots:
    void handleFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus
    );

    void handleProcessError(QProcess::ProcessError error);

private:
    bool prepareQtInput(const QString &inputPath);

    bool convertWithQt(
        const QString &inputPath,
        const QString &outputPath
    );

    bool convertToSvg(
        const QString &inputPath,
        const QString &outputPath
    );

    void cleanup();

    QProcess *process_ = nullptr;
    QTemporaryDir *temporaryDir_ = nullptr;

    QString outputPath_;
    QString inputPath_;

    bool running_ = false;
    bool cancelled_ = false;
};