#ifndef QEMU_MANAGER_H
#define QEMU_MANAGER_H

#include <QString>
#include <QProcess>
#include <QObject>
#include <memory>

class VMConfig;

class QemuManager : public QObject {
    Q_OBJECT

public:
    explicit QemuManager(QObject *parent = nullptr);
    ~QemuManager();

    bool startVM(const VMConfig& config);
    void stopVM();
    void pauseVM();
    void resumeVM();
    bool isRunning() const;
    QString getStatus() const;
    int getVMPid() const;

signals:
    void vmStarted();
    void vmStopped();
    void vmError(const QString& error);
    void outputReceived(const QString& output);

private slots:
    void handleProcessOutput();
    void handleProcessError();
    void handleProcessFinished(int exitCode);

private:
    bool checkQemuAvailable();
    QStringList buildQemuCommand(const VMConfig& config);
    bool verifyKVMSupport();

    std::unique_ptr<QProcess> m_process;
    bool m_isRunning;
    QString m_lastError;
};

#endif // QEMU_MANAGER_H
