#include "qemu_manager.h"
#include "vm_config.h"
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

QemuManager::QemuManager(QObject *parent)
    : QObject(parent), m_isRunning(false) {
    m_process = std::make_unique<QProcess>(this);

    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &QemuManager::handleProcessOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &QemuManager::handleProcessError);
    connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &QemuManager::handleProcessFinished);
}

QemuManager::~QemuManager() {
    if (m_isRunning) {
        stopVM();
    }
}

bool QemuManager::checkQemuAvailable() {
    QProcess check;
    check.start("which", QStringList() << "qemu-system-x86_64");
    check.waitForFinished();
    return check.exitCode() == 0;
}

bool QemuManager::verifyKVMSupport() {
    return QFile::exists("/dev/kvm");
}

bool QemuManager::startVM(const VMConfig& config) {
    if (m_isRunning) {
        m_lastError = "VM is already running";
        emit vmError(m_lastError);
        return false;
    }

    if (!checkQemuAvailable()) {
        m_lastError = "QEMU not found. Please install qemu-system-x86";
        emit vmError(m_lastError);
        return false;
    }

    if (!verifyKVMSupport()) {
        qWarning() << "KVM not available. Performance will be reduced.";
    }

    QStringList args = buildQemuCommand(config);

    qDebug() << "Starting QEMU with args:" << args;
    m_process->start("qemu-system-x86_64", args);

    if (!m_process->waitForStarted(5000)) {
        m_lastError = "Failed to start QEMU process";
        emit vmError(m_lastError);
        return false;
    }

    m_isRunning = true;
    emit vmStarted();
    return true;
}

QStringList QemuManager::buildQemuCommand(const VMConfig& config) {
    QStringList args;

    // Enable KVM if available
    if (verifyKVMSupport()) {
        args << "-enable-kvm";
    }

    // CPU configuration
    args << "-cpu" << "host";
    args << "-smp" << QString::number(config.cpuCores());

    // Memory
    args << "-m" << QString::number(config.ramMB()) + "M";

    // Display
    args << "-vga" << "virtio";
    args << "-display" << "gtk,gl=on";

    // Boot from image
    args << "-cdrom" << config.imagePath();

    // Disk image for persistent storage
    if (!config.diskPath().isEmpty()) {
        args << "-drive" << "file=" + config.diskPath() + ",if=virtio";
    }

    // Network
    args << "-netdev" << "user,id=net0,hostfwd=tcp::5555-:5555";
    args << "-device" << "virtio-net-pci,netdev=net0";

    // Audio
    args << "-device" << "intel-hda";
    args << "-device" << "hda-duplex";

    // USB support
    args << "-usb";
    args << "-device" << "usb-tablet";

    // Boot order
    args << "-boot" << "d";

    return args;
}

void QemuManager::stopVM() {
    if (!m_isRunning) {
        return;
    }

    m_process->terminate();

    if (!m_process->waitForFinished(5000)) {
        m_process->kill();
        m_process->waitForFinished();
    }

    m_isRunning = false;
    emit vmStopped();
}

void QemuManager::pauseVM() {
    if (m_isRunning) {
        m_process->write("stop\n");
    }
}

void QemuManager::resumeVM() {
    if (m_isRunning) {
        m_process->write("cont\n");
    }
}

bool QemuManager::isRunning() const {
    return m_isRunning;
}

QString QemuManager::getStatus() const {
    if (m_isRunning) {
        return "Running";
    } else if (!m_lastError.isEmpty()) {
        return "Error: " + m_lastError;
    }
    return "Stopped";
}

int QemuManager::getVMPid() const {
    if (m_isRunning) {
        return m_process->processId();
    }
    return -1;
}

void QemuManager::handleProcessOutput() {
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    emit outputReceived(output);
    qDebug() << "QEMU output:" << output;
}

void QemuManager::handleProcessError() {
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    qWarning() << "QEMU error:" << error;
}

void QemuManager::handleProcessFinished(int exitCode) {
    m_isRunning = false;
    qDebug() << "QEMU process finished with exit code:" << exitCode;

    if (exitCode != 0) {
        m_lastError = "QEMU exited with code " + QString::number(exitCode);
        emit vmError(m_lastError);
    }

    emit vmStopped();
}
