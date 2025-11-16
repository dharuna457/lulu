#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QCryptographicHash>

class DownloadManager : public QObject {
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    void startDownload(const QString& url, const QString& destination);
    void pauseDownload();
    void resumeDownload();
    void cancelDownload();

    bool isDownloading() const { return m_isDownloading; }
    qint64 bytesReceived() const { return m_bytesReceived; }
    qint64 totalBytes() const { return m_totalBytes; }
    int progressPercentage() const;
    double downloadSpeed() const { return m_downloadSpeed; }
    QString estimatedTimeRemaining() const;

    // Checksum verification
    void setExpectedChecksum(const QString& sha256);
    bool verifyChecksum();

signals:
    void downloadProgress(qint64 bytesReceived, qint64 totalBytes);
    void downloadFinished(const QString& filePath);
    void downloadError(const QString& error);
    void downloadSpeedUpdated(double bytesPerSecond);
    void checksumVerified(bool success);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 totalBytes);
    void onFinished();
    void onReadyRead();
    void onError(QNetworkReply::NetworkError error);
    void updateSpeed();

private:
    bool supportsResume();
    void calculateSpeed();

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_reply;
    QFile *m_file;

    QString m_url;
    QString m_destination;
    QString m_expectedChecksum;

    bool m_isDownloading;
    qint64 m_bytesReceived;
    qint64 m_totalBytes;
    qint64 m_previousBytes;
    double m_downloadSpeed;

    QTimer *m_speedTimer;
    QElapsedTimer m_downloadTime;

    int m_retryCount;
    static const int MAX_RETRIES = 3;
};

#endif // DOWNLOAD_MANAGER_H
