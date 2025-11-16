#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <signal.h>
#include "core/download_manager.h"

class LinuxDroidDaemon : public QObject {
    Q_OBJECT

public:
    LinuxDroidDaemon(QObject *parent = nullptr) : QObject(parent) {
        m_logFile.setFileName("/var/log/linuxdroid/download.log");

        QDir logDir = QFileInfo(m_logFile).dir();
        if (!logDir.exists()) {
            logDir.mkpath(".");
        }

        if (m_logFile.open(QIODevice::Append | QIODevice::Text)) {
            log("LinuxDroid daemon started");
        }

        m_downloadManager = new DownloadManager(this);

        connect(m_downloadManager, &DownloadManager::downloadProgress,
                this, &LinuxDroidDaemon::onDownloadProgress);
        connect(m_downloadManager, &DownloadManager::downloadFinished,
                this, &LinuxDroidDaemon::onDownloadFinished);
        connect(m_downloadManager, &DownloadManager::downloadError,
                this, &LinuxDroidDaemon::onDownloadError);
        connect(m_downloadManager, &DownloadManager::checksumVerified,
                this, &LinuxDroidDaemon::onChecksumVerified);
    }

    ~LinuxDroidDaemon() {
        log("LinuxDroid daemon stopped");
        m_logFile.close();
    }

    void startDownload(const QString& url, const QString& destination) {
        log("Starting download: " + url);
        log("Destination: " + destination);
        m_downloadManager->startDownload(url, destination);
    }

public slots:
    void onDownloadProgress(qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = static_cast<int>((received * 100) / total);
            if (percentage % 10 == 0 && percentage != m_lastLoggedPercentage) {
                log(QString("Download progress: %1%").arg(percentage));
                m_lastLoggedPercentage = percentage;

                // Send D-Bus notification (in production)
                // notifyProgress(percentage);
            }
        }
    }

    void onDownloadFinished(const QString& filePath) {
        log("Download completed: " + filePath);

        // Verify checksum if set
        if (!m_expectedChecksum.isEmpty()) {
            log("Verifying checksum...");
            m_downloadManager->verifyChecksum();
        } else {
            log("Download finished successfully (no checksum verification)");
            QCoreApplication::quit();
        }
    }

    void onDownloadError(const QString& error) {
        log("Download error: " + error);
        QCoreApplication::exit(1);
    }

    void onChecksumVerified(bool success) {
        if (success) {
            log("Checksum verification: SUCCESS");
            QCoreApplication::quit();
        } else {
            log("Checksum verification: FAILED");
            QCoreApplication::exit(2);
        }
    }

private:
    void log(const QString& message) {
        QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        QString logMessage = QString("[%1] %2\n").arg(timestamp, message);

        if (m_logFile.isOpen()) {
            QTextStream stream(&m_logFile);
            stream << logMessage;
            m_logFile.flush();
        }

        qDebug() << message;
    }

    DownloadManager *m_downloadManager;
    QFile m_logFile;
    QString m_expectedChecksum;
    int m_lastLoggedPercentage = -1;
};

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    qDebug() << "Received signal:" << signal;
    QCoreApplication::quit();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("linuxdroid-daemon");
    app.setApplicationVersion("1.0.0");

    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    LinuxDroidDaemon daemon;

    // Check for command line arguments
    if (argc >= 3) {
        QString url = argv[1];
        QString destination = argv[2];
        daemon.startDownload(url, destination);
    } else {
        qWarning() << "Usage: linuxdroid-daemon <url> <destination>";
        qWarning() << "Running in idle mode - waiting for D-Bus commands";

        // In production, would listen for D-Bus commands
        // For now, just run event loop
    }

    return app.exec();
}

#include "daemon.moc"
