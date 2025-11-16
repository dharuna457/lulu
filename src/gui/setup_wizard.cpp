#include "setup_wizard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QPixmap>
#include <QThread>
#include <QGroupBox>

// SetupWizard Implementation
SetupWizard::SetupWizard(QWidget *parent)
    : QWizard(parent),
      m_cpuCores(2),
      m_ramMB(4096),
      m_resolution("1920x1080"),
      m_rootEnabled(false) {

    setWindowTitle("LinuxDroid Setup Wizard");
    setWizardStyle(QWizard::ModernStyle);
    setMinimumSize(800, 600);

    setPage(Page_Welcome, new WelcomePage(this));
    setPage(Page_SystemConfig, new SystemConfigPage(this));
    setPage(Page_ImageSelection, new ImageSelectionPage(this));
    setPage(Page_DownloadProgress, new DownloadProgressPage(this));
    setPage(Page_InstanceSetup, new InstanceSetupPage(this));
    setPage(Page_Completion, new CompletionPage(this));

    setStartId(Page_Welcome);
}

SetupWizard::~SetupWizard() {
}

void SetupWizard::setSelectedImage(const QString& url, const QString& name, qint64 size) {
    m_selectedImageUrl = url;
    m_selectedImageName = name;
    m_selectedImageSize = size;
}

void SetupWizard::setInstanceConfig(const QString& name, int cores, int ram, const QString& res, bool root) {
    m_instanceName = name;
    m_cpuCores = cores;
    m_ramMB = ram;
    m_resolution = res;
    m_rootEnabled = root;
}

// WelcomePage Implementation
WelcomePage::WelcomePage(QWidget *parent) : QWizardPage(parent) {
    setTitle("Welcome to LinuxDroid");
    setSubTitle("Professional Android Emulator for Linux");

    setupUI();
    checkSystemRequirements();
}

void WelcomePage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_logoLabel = new QLabel(this);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setText("<h1>📱 LinuxDroid</h1>");

    m_welcomeLabel = new QLabel(
        "<h2>Welcome to LinuxDroid Setup</h2>"
        "<p>This wizard will guide you through the installation and configuration of LinuxDroid.</p>"
        "<p>LinuxDroid is a high-performance Android emulator for Ubuntu/Debian featuring:</p>"
        "<ul>"
        "<li>KVM hardware acceleration</li>"
        "<li>Multi-instance support</li>"
        "<li>Easy setup and configuration</li>"
        "<li>Automatic Android image download</li>"
        "</ul>"
    );
    m_welcomeLabel->setWordWrap(true);

    m_requirementsLabel = new QLabel("<h3>System Requirements:</h3>");

    m_statusWidget = new QWidget(this);
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusWidget);

    layout->addWidget(m_logoLabel);
    layout->addWidget(m_welcomeLabel);
    layout->addWidget(m_requirementsLabel);
    layout->addWidget(m_statusWidget);
    layout->addStretch();
}

void WelcomePage::checkSystemRequirements() {
    SystemChecker::SystemInfo info = SystemChecker::checkSystem();

    QVBoxLayout *statusLayout = qobject_cast<QVBoxLayout*>(m_statusWidget->layout());
    if (!statusLayout) return;

    auto addStatus = [statusLayout](const QString& label, bool ok) {
        QHBoxLayout *row = new QHBoxLayout();
        QString icon = ok ? "✅" : "⚠️";
        QLabel *statusLabel = new QLabel(icon + " " + label);
        row->addWidget(statusLabel);
        statusLayout->addLayout(row);
    };

    addStatus(QString("CPU: %1 cores").arg(info.cpuCores),
              info.cpuCores >= SystemChecker::MIN_CPU_CORES);

    addStatus(QString("RAM: %1 GB").arg(info.totalRamMB / 1024),
              info.totalRamMB >= SystemChecker::MIN_RAM_MB);

    addStatus(QString("Disk Space: %1 GB available").arg(info.diskSpaceGB),
              info.diskSpaceGB >= SystemChecker::MIN_DISK_GB);

    addStatus("QEMU Installed", info.qemuInstalled);

    addStatus("KVM Acceleration", info.kvmAvailable && info.virtualizationEnabled);

    QStringList warnings = SystemChecker::getWarnings(info);
    if (!warnings.isEmpty()) {
        QLabel *warningLabel = new QLabel(
            "<p style='color: orange;'><b>Warnings:</b><br>" +
            warnings.join("<br>") +
            "</p>"
        );
        warningLabel->setWordWrap(true);
        statusLayout->addWidget(warningLabel);
    }
}

// SystemConfigPage Implementation
SystemConfigPage::SystemConfigPage(QWidget *parent) : QWizardPage(parent) {
    setTitle("System Configuration");
    setSubTitle("Configure your emulator settings");

    loadSystemInfo();
    setupUI();
}

void SystemConfigPage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    // KVM Status
    QGroupBox *kvmGroup = new QGroupBox("Virtualization Status");
    QVBoxLayout *kvmLayout = new QVBoxLayout(kvmGroup);
    m_kvmStatus = new QLabel();

    if (m_systemInfo.kvmAvailable && m_systemInfo.virtualizationEnabled) {
        m_kvmStatus->setText("✅ KVM acceleration enabled - optimal performance");
        m_kvmStatus->setStyleSheet("color: green; font-weight: bold;");
    } else {
        m_kvmStatus->setText("⚠️ KVM not available - enable virtualization in BIOS for better performance");
        m_kvmStatus->setStyleSheet("color: orange; font-weight: bold;");
    }
    kvmLayout->addWidget(m_kvmStatus);
    layout->addWidget(kvmGroup);

    // CPU Configuration
    QGroupBox *cpuGroup = new QGroupBox("CPU Cores");
    QVBoxLayout *cpuLayout = new QVBoxLayout(cpuGroup);

    m_cpuSlider = new QSlider(Qt::Horizontal);
    m_cpuSlider->setMinimum(1);
    m_cpuSlider->setMaximum(m_systemInfo.cpuCores);
    m_cpuSlider->setValue(qMin(4, m_systemInfo.cpuCores));
    m_cpuSlider->setTickPosition(QSlider::TicksBelow);

    m_cpuLabel = new QLabel();
    updateCpuLabel(m_cpuSlider->value());

    connect(m_cpuSlider, &QSlider::valueChanged, this, &SystemConfigPage::updateCpuLabel);

    cpuLayout->addWidget(m_cpuLabel);
    cpuLayout->addWidget(m_cpuSlider);
    layout->addWidget(cpuGroup);

    // RAM Configuration
    QGroupBox *ramGroup = new QGroupBox("Memory (RAM)");
    QVBoxLayout *ramLayout = new QVBoxLayout(ramGroup);

    m_ramSlider = new QSlider(Qt::Horizontal);
    m_ramSlider->setMinimum(2048);  // 2GB
    m_ramSlider->setMaximum(m_systemInfo.totalRamMB - 2048);  // Reserve 2GB for system
    m_ramSlider->setValue(4096);  // Default 4GB
    m_ramSlider->setSingleStep(512);
    m_ramSlider->setTickPosition(QSlider::TicksBelow);

    m_ramLabel = new QLabel();
    updateRamLabel(m_ramSlider->value());

    connect(m_ramSlider, &QSlider::valueChanged, this, &SystemConfigPage::updateRamLabel);

    ramLayout->addWidget(m_ramLabel);
    ramLayout->addWidget(m_ramSlider);
    layout->addWidget(ramGroup);

    // Disk Space Info
    QGroupBox *diskGroup = new QGroupBox("Disk Space");
    QVBoxLayout *diskLayout = new QVBoxLayout(diskGroup);
    m_diskSpaceLabel = new QLabel(QString("Available: %1 GB").arg(m_systemInfo.diskSpaceGB));
    diskLayout->addWidget(m_diskSpaceLabel);
    layout->addWidget(diskGroup);

    layout->addStretch();
}

void SystemConfigPage::loadSystemInfo() {
    m_systemInfo = SystemChecker::checkSystem();
}

void SystemConfigPage::updateRamLabel(int value) {
    m_ramLabel->setText(QString("Allocated RAM: %1 GB / %2 GB total")
                            .arg(value / 1024.0, 0, 'f', 1)
                            .arg(m_systemInfo.totalRamMB / 1024.0, 0, 'f', 1));
}

void SystemConfigPage::updateCpuLabel(int value) {
    m_cpuLabel->setText(QString("CPU Cores: %1 / %2 available")
                            .arg(value)
                            .arg(m_systemInfo.cpuCores));
}

bool SystemConfigPage::validatePage() {
    if (m_ramSlider->value() < 2048) {
        QMessageBox::warning(this, "Invalid Configuration",
                           "At least 2GB of RAM is required.");
        return false;
    }

    if (m_cpuSlider->value() < 1) {
        QMessageBox::warning(this, "Invalid Configuration",
                           "At least 1 CPU core is required.");
        return false;
    }

    return true;
}

// ImageSelectionPage Implementation
ImageSelectionPage::ImageSelectionPage(QWidget *parent) : QWizardPage(parent) {
    setTitle("Android Image Selection");
    setSubTitle("Choose the Android version to download");

    loadAvailableImages();
    setupUI();
}

void ImageSelectionPage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *infoLabel = new QLabel(
        "Select an Android x86 image to download. "
        "The recommended version provides the best compatibility."
    );
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Image selection
    QGroupBox *imageGroup = new QGroupBox("Android Version");
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);

    m_imageCombo = new QComboBox();
    for (const auto& img : m_availableImages) {
        QString displayText = img.name;
        if (img.recommended) {
            displayText += " [Recommended]";
        }
        m_imageCombo->addItem(displayText);
    }

    connect(m_imageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageSelectionPage::onImageSelected);

    m_imageSizeLabel = new QLabel();
    m_imageSourceLabel = new QLabel();

    imageLayout->addWidget(m_imageCombo);
    imageLayout->addWidget(m_imageSizeLabel);
    imageLayout->addWidget(m_imageSourceLabel);

    layout->addWidget(imageGroup);

    // Additional options
    m_gappsCheckbox = new QCheckBox("Include Google Play Services (if available)");
    layout->addWidget(m_gappsCheckbox);

    layout->addStretch();

    // Trigger initial selection
    onImageSelected(0);
}

void ImageSelectionPage::loadAvailableImages() {
    // Load from bundled JSON or use defaults
    m_availableImages = {
        {"Android 9.0 (Pie) - x86_64", "9.0",
         "https://sourceforge.net/projects/android-x86/files/Release%209.0/android-x86_64-9.0-r2.iso/download",
         1200, "", true},
        {"Android 11 (R) - x86_64", "11.0",
         "https://sourceforge.net/projects/android-x86/files/Release%2011/android-x86_64-11.0-r4.iso/download",
         1400, "", false},
        {"Android 13 (Tiramisu) - x86_64", "13.0",
         "https://sourceforge.net/projects/android-x86/files/Release%2013/android-x86_64-13.0-r1.iso/download",
         1600, "", false}
    };
}

void ImageSelectionPage::onImageSelected(int index) {
    if (index < 0 || index >= m_availableImages.size()) {
        return;
    }

    const ImageInfo& img = m_availableImages[index];
    m_imageSizeLabel->setText(QString("Size: %1 MB (%2 GB)")
                                  .arg(img.sizeMB)
                                  .arg(img.sizeMB / 1024.0, 0, 'f', 2));
    m_imageSourceLabel->setText("Source: SourceForge/Android-x86 Project");
}

void ImageSelectionPage::initializePage() {
    // Called when page is shown
}

bool ImageSelectionPage::validatePage() {
    int index = m_imageCombo->currentIndex();
    if (index >= 0 && index < m_availableImages.size()) {
        const ImageInfo& img = m_availableImages[index];

        SetupWizard *wiz = qobject_cast<SetupWizard*>(wizard());
        if (wiz) {
            wiz->setSelectedImage(img.url, img.name, img.sizeMB * 1024 * 1024);
        }

        return true;
    }

    return false;
}

// DownloadProgressPage Implementation
DownloadProgressPage::DownloadProgressPage(QWidget *parent)
    : QWizardPage(parent),
      m_downloadManager(nullptr),
      m_downloadComplete(false) {

    setTitle("Downloading Android Image");
    setSubTitle("Please wait while the Android system image is downloaded");

    setupUI();
}

void DownloadProgressPage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel("Preparing download...");
    layout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    m_sizeLabel = new QLabel("0 MB / 0 MB");
    layout->addWidget(m_sizeLabel);

    m_speedLabel = new QLabel("Speed: 0 MB/s");
    layout->addWidget(m_speedLabel);

    m_timeLabel = new QLabel("Time remaining: Calculating...");
    layout->addWidget(m_timeLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_backgroundButton = new QPushButton("Download in Background");
    m_cancelButton = new QPushButton("Cancel");

    connect(m_backgroundButton, &QPushButton::clicked, this, &DownloadProgressPage::onBackgroundClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &DownloadProgressPage::onCancelClicked);

    buttonLayout->addWidget(m_backgroundButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    layout->addStretch();
}

void DownloadProgressPage::initializePage() {
    m_downloadComplete = false;
    m_downloadManager = new DownloadManager(this);

    connect(m_downloadManager, &DownloadManager::downloadProgress,
            this, &DownloadProgressPage::onDownloadProgress);
    connect(m_downloadManager, &DownloadManager::downloadFinished,
            this, &DownloadProgressPage::onDownloadFinished);
    connect(m_downloadManager, &DownloadManager::downloadError,
            this, &DownloadProgressPage::onDownloadError);
    connect(m_downloadManager, &DownloadManager::downloadSpeedUpdated,
            this, &DownloadProgressPage::onSpeedUpdated);

    startDownload();
}

void DownloadProgressPage::startDownload() {
    SetupWizard *wiz = qobject_cast<SetupWizard*>(wizard());
    if (!wiz) return;

    QString url = wiz->selectedImageUrl();
    QString name = wiz->selectedImageName();

    // Ensure directory exists
    QDir dir("/opt/linuxdroid/images");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Extract filename from name
    QString filename = name.split(" - ").first().replace(" ", "_") + ".iso";
    QString destination = "/opt/linuxdroid/images/" + filename;

    m_statusLabel->setText("Downloading: " + name);
    m_downloadManager->startDownload(url, destination);
}

void DownloadProgressPage::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        m_sizeLabel->setText(QString("%1 / %2")
                                .arg(formatSize(received))
                                .arg(formatSize(total)));
    }

    emit completeChanged();
}

void DownloadProgressPage::onDownloadFinished(const QString& filePath) {
    m_downloadComplete = true;
    m_downloadedFilePath = filePath;

    m_statusLabel->setText("✅ Download completed successfully!");
    m_progressBar->setValue(100);
    m_backgroundButton->setEnabled(false);
    m_cancelButton->setEnabled(false);

    emit completeChanged();
}

void DownloadProgressPage::onDownloadError(const QString& error) {
    m_statusLabel->setText("❌ Download failed: " + error);
    m_progressBar->setValue(0);

    QMessageBox::critical(this, "Download Error",
                        "Failed to download Android image:\n" + error);
}

void DownloadProgressPage::onSpeedUpdated(double bytesPerSecond) {
    m_speedLabel->setText("Speed: " + formatSpeed(bytesPerSecond));
    m_timeLabel->setText("Time remaining: " + m_downloadManager->estimatedTimeRemaining());
}

void DownloadProgressPage::onBackgroundClicked() {
    QMessageBox::information(this, "Background Download",
                           "Download will continue in the background.\n"
                           "You can check progress in the system tray.");
    // In production, this would minimize to tray
}

void DownloadProgressPage::onCancelClicked() {
    auto reply = QMessageBox::question(this, "Cancel Download",
                                      "Are you sure you want to cancel the download?",
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_downloadManager) {
            m_downloadManager->cancelDownload();
        }
        wizard()->reject();
    }
}

bool DownloadProgressPage::isComplete() const {
    return m_downloadComplete;
}

QString DownloadProgressPage::formatSize(qint64 bytes) const {
    if (bytes < 1024) {
        return QString::number(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return QString::number(bytes / 1024.0, 'f', 2) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 2) + " MB";
    } else {
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
}

QString DownloadProgressPage::formatSpeed(double bytesPerSecond) const {
    return formatSize(static_cast<qint64>(bytesPerSecond)) + "/s";
}

// InstanceSetupPage Implementation
InstanceSetupPage::InstanceSetupPage(QWidget *parent) : QWizardPage(parent) {
    setTitle("Instance Configuration");
    setSubTitle("Configure your Android instance");

    setupUI();
}

void InstanceSetupPage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Instance name
    QGroupBox *nameGroup = new QGroupBox("Instance Name");
    QVBoxLayout *nameLayout = new QVBoxLayout(nameGroup);
    m_nameEdit = new QLineEdit("My Android");
    nameLayout->addWidget(m_nameEdit);
    layout->addWidget(nameGroup);

    // Resolution
    QGroupBox *resGroup = new QGroupBox("Display Resolution");
    QVBoxLayout *resLayout = new QVBoxLayout(resGroup);
    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->addItems({"1280x720 (720p)", "1920x1080 (1080p)",
                                 "2560x1440 (1440p)", "3840x2160 (4K)"});
    m_resolutionCombo->setCurrentIndex(1);  // Default 1080p
    resLayout->addWidget(m_resolutionCombo);
    layout->addWidget(resGroup);

    // CPU cores
    QGroupBox *cpuGroup = new QGroupBox("CPU Cores");
    QVBoxLayout *cpuLayout = new QVBoxLayout(cpuGroup);
    m_cpuSlider = new QSlider(Qt::Horizontal);
    m_cpuSlider->setMinimum(1);
    m_cpuSlider->setMaximum(QThread::idealThreadCount());
    m_cpuSlider->setValue(qMin(4, QThread::idealThreadCount()));
    m_cpuLabel = new QLabel();
    updateCpuLabel(m_cpuSlider->value());
    connect(m_cpuSlider, &QSlider::valueChanged, this, &InstanceSetupPage::updateCpuLabel);
    cpuLayout->addWidget(m_cpuLabel);
    cpuLayout->addWidget(m_cpuSlider);
    layout->addWidget(cpuGroup);

    // RAM
    QGroupBox *ramGroup = new QGroupBox("Memory (RAM)");
    QVBoxLayout *ramLayout = new QVBoxLayout(ramGroup);
    m_ramSlider = new QSlider(Qt::Horizontal);
    m_ramSlider->setMinimum(2048);
    m_ramSlider->setMaximum(16384);
    m_ramSlider->setValue(4096);
    m_ramSlider->setSingleStep(512);
    m_ramLabel = new QLabel();
    updateRamLabel(m_ramSlider->value());
    connect(m_ramSlider, &QSlider::valueChanged, this, &InstanceSetupPage::updateRamLabel);
    ramLayout->addWidget(m_ramLabel);
    ramLayout->addWidget(m_ramSlider);
    layout->addWidget(ramGroup);

    // Root access
    m_rootCheckbox = new QCheckBox("Enable root access (for development)");
    layout->addWidget(m_rootCheckbox);

    layout->addStretch();
}

void InstanceSetupPage::updateRamLabel(int value) {
    m_ramLabel->setText(QString("RAM: %1 GB").arg(value / 1024.0, 0, 'f', 1));
}

void InstanceSetupPage::updateCpuLabel(int value) {
    m_cpuLabel->setText(QString("CPU Cores: %1").arg(value));
}

bool InstanceSetupPage::validatePage() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Please enter an instance name.");
        return false;
    }

    SetupWizard *wiz = qobject_cast<SetupWizard*>(wizard());
    if (wiz) {
        QString resolution = m_resolutionCombo->currentText().split(" ").first();
        wiz->setInstanceConfig(
            m_nameEdit->text(),
            m_cpuSlider->value(),
            m_ramSlider->value(),
            resolution,
            m_rootCheckbox->isChecked()
        );
    }

    return true;
}

// CompletionPage Implementation
CompletionPage::CompletionPage(QWidget *parent) : QWizardPage(parent) {
    setTitle("Setup Complete");
    setSubTitle("LinuxDroid is ready to use!");

    setupUI();
}

void CompletionPage::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_successLabel = new QLabel(
        "<h2>✅ Setup Completed Successfully!</h2>"
        "<p>LinuxDroid has been configured and is ready to launch.</p>"
    );
    m_successLabel->setWordWrap(true);
    layout->addWidget(m_successLabel);

    m_tipsLabel = new QLabel(
        "<h3>Quick Tips:</h3>"
        "<ul>"
        "<li><b>Ctrl+Alt+F</b> - Toggle fullscreen</li>"
        "<li><b>Ctrl+Alt+G</b> - Release mouse grab</li>"
        "<li><b>Ctrl+Alt+Q</b> - Quit emulator</li>"
        "<li>Use <b>adb connect localhost:5555</b> to connect via ADB</li>"
        "</ul>"
        "<p>You can create additional instances from the main window.</p>"
    );
    m_tipsLabel->setWordWrap(true);
    layout->addWidget(m_tipsLabel);

    layout->addStretch();
}

void CompletionPage::initializePage() {
    setFinalPage(true);
}
