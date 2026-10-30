#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class VideoConverterService : public QObject
{
    Q_OBJECT

public:
    explicit VideoConverterService(QObject *parent = nullptr);
    ~VideoConverterService() override;

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
    void progressChanged(int percentage);
    void finished(const QString &outputPath);
    void failed(const QString &errorMessage);
    void cancelled();

private slots:
    void readOutput();
    void handleFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus
    );

private:
    QProcess *process_ = nullptr;
    QString outputPath_;

    qint64 durationMs_ = 0;
    bool cancelled_ = false;

    QString ffmpegPath() const;
};