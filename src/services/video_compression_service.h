#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class VideoCompressionService : public QObject
{
    Q_OBJECT

public:
    explicit VideoCompressionService(QObject *parent = nullptr);
    ~VideoCompressionService() override;

    bool isRunning() const;

public slots:
    void compress(
        const QString &inputPath,
        const QString &outputPath,
        int quality
    );

    void cancel();

signals:
    void started();
    void progressChanged(int percentage);

    void finished(
        const QString &outputPath,
        qint64 originalSize,
        qint64 compressedSize
    );

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
    qint64 originalSize_ = 0;
    qint64 durationMs_ = 0;
    bool cancelled_ = false;

    QString ffmpegPath() const;
};