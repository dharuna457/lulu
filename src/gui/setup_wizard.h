#ifndef SETUP_WIZARD_H
#define SETUP_WIZARD_H

#include <QWizard>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include "../utils/system_checker.h"
#include "../core/download_manager.h"

// Forward declarations
class WelcomePage;
class SystemConfigPage;
class ImageSelectionPage;
class DownloadProgressPage;
class InstanceSetupPage;
class CompletionPage;

class SetupWizard : public QWizard {
    Q_OBJECT

public:
    explicit SetupWizard(QWidget *parent = nullptr);
    ~SetupWizard();

    enum PageId {
        Page_Welcome = 0,
        Page_SystemConfig,
        Page_ImageSelection,
        Page_DownloadProgress,
        Page_InstanceSetup,
        Page_Completion
    };

    // Shared data
    QString selectedImageUrl() const { return m_selectedImageUrl; }
    QString selectedImageName() const { return m_selectedImageName; }
    qint64 selectedImageSize() const { return m_selectedImageSize; }
    QString instanceName() const { return m_instanceName; }
    int cpuCores() const { return m_cpuCores; }
    int ramMB() const { return m_ramMB; }
    QString resolution() const { return m_resolution; }
    bool rootEnabled() const { return m_rootEnabled; }

    void setSelectedImage(const QString& url, const QString& name, qint64 size);
    void setInstanceConfig(const QString& name, int cores, int ram, const QString& res, bool root);

private:
    QString m_selectedImageUrl;
    QString m_selectedImageName;
    qint64 m_selectedImageSize;
    QString m_instanceName;
    int m_cpuCores;
    int m_ramMB;
    QString m_resolution;
    bool m_rootEnabled;
};

// Welcome Page
class WelcomePage : public QWizardPage {
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

private:
    void checkSystemRequirements();
    void setupUI();

    QLabel *m_logoLabel;
    QLabel *m_welcomeLabel;
    QLabel *m_requirementsLabel;
    QWidget *m_statusWidget;
};

// System Configuration Page
class SystemConfigPage : public QWizardPage {
    Q_OBJECT

public:
    explicit SystemConfigPage(QWidget *parent = nullptr);

    bool validatePage() override;

private slots:
    void updateRamLabel(int value);
    void updateCpuLabel(int value);

private:
    void setupUI();
    void loadSystemInfo();

    QSlider *m_ramSlider;
    QSlider *m_cpuSlider;
    QLabel *m_ramLabel;
    QLabel *m_cpuLabel;
    QLabel *m_kvmStatus;
    QLabel *m_diskSpaceLabel;

    SystemChecker::SystemInfo m_systemInfo;
};

// Image Selection Page
class ImageSelectionPage : public QWizardPage {
    Q_OBJECT

public:
    explicit ImageSelectionPage(QWidget *parent = nullptr);

    bool validatePage() override;
    void initializePage() override;

private slots:
    void onImageSelected(int index);

private:
    void setupUI();
    void loadAvailableImages();

    QComboBox *m_imageCombo;
    QLabel *m_imageSizeLabel;
    QLabel *m_imageSourceLabel;
    QCheckBox *m_gappsCheckbox;

    struct ImageInfo {
        QString name;
        QString version;
        QString url;
        qint64 sizeMB;
        QString sha256;
        bool recommended;
    };

    QVector<ImageInfo> m_availableImages;
};

// Download Progress Page
class DownloadProgressPage : public QWizardPage {
    Q_OBJECT

public:
    explicit DownloadProgressPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished(const QString& filePath);
    void onDownloadError(const QString& error);
    void onSpeedUpdated(double bytesPerSecond);
    void onBackgroundClicked();
    void onCancelClicked();

private:
    void setupUI();
    void startDownload();
    QString formatSize(qint64 bytes) const;
    QString formatSpeed(double bytesPerSecond) const;

    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_speedLabel;
    QLabel *m_timeLabel;
    QLabel *m_sizeLabel;
    QPushButton *m_backgroundButton;
    QPushButton *m_cancelButton;

    DownloadManager *m_downloadManager;
    bool m_downloadComplete;
    QString m_downloadedFilePath;
};

// Instance Setup Page
class InstanceSetupPage : public QWizardPage {
    Q_OBJECT

public:
    explicit InstanceSetupPage(QWidget *parent = nullptr);

    bool validatePage() override;

private slots:
    void updateRamLabel(int value);
    void updateCpuLabel(int value);

private:
    void setupUI();

    QLineEdit *m_nameEdit;
    QComboBox *m_resolutionCombo;
    QSlider *m_cpuSlider;
    QSlider *m_ramSlider;
    QLabel *m_cpuLabel;
    QLabel *m_ramLabel;
    QCheckBox *m_rootCheckbox;
};

// Completion Page
class CompletionPage : public QWizardPage {
    Q_OBJECT

public:
    explicit CompletionPage(QWidget *parent = nullptr);

    void initializePage() override;

private:
    void setupUI();

    QLabel *m_successLabel;
    QLabel *m_tipsLabel;
};

#endif // SETUP_WIZARD_H
