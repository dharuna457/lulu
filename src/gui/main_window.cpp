#include "main_window.h"
#include "setup_wizard.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_qemuManager(new QemuManager(this)),
      m_currentInstance(nullptr) {

    setWindowTitle("LinuxDroid - Android Emulator");
    setMinimumSize(900, 600);

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupTrayIcon();
    loadInstances();

    connect(m_qemuManager, &QemuManager::vmStarted, this, &MainWindow::onVMStarted);
    connect(m_qemuManager, &QemuManager::vmStopped, this, &MainWindow::onVMStopped);
    connect(m_qemuManager, &QemuManager::vmError, this, &MainWindow::onVMError);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel *titleLabel = new QLabel("<h2>📱 Android Instances</h2>");
    mainLayout->addWidget(titleLabel);

    // Instance list
    m_instanceList = new QListWidget();
    connect(m_instanceList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onInstanceSelected);
    mainLayout->addWidget(m_instanceList);

    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *newButton = new QPushButton("New Instance");
    connect(newButton, &QPushButton::clicked, this, &MainWindow::onNewInstance);

    m_startButton = new QPushButton("Start");
    m_startButton->setEnabled(false);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartInstance);

    m_stopButton = new QPushButton("Stop");
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopInstance);

    m_deleteButton = new QPushButton("Delete");
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteInstance);

    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenuBar() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New Instance", this, &MainWindow::onNewInstance, QKeySequence::New);
    fileMenu->addAction("&Settings", this, &MainWindow::onSettings);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    QToolBar *toolBar = addToolBar("Main");
    toolBar->addAction("New", this, &MainWindow::onNewInstance);
    toolBar->addAction("Start", this, &MainWindow::onStartInstance);
    toolBar->addAction("Stop", this, &MainWindow::onStopInstance);
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel("Ready");
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("LinuxDroid");

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("Show", this, &QWidget::showNormal);
    trayMenu->addAction("Quit", this, &QWidget::close);

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void MainWindow::loadInstances() {
    QDir instanceDir("/opt/linuxdroid/instances");
    if (!instanceDir.exists()) {
        instanceDir.mkpath(".");
        return;
    }

    QStringList configFiles = instanceDir.entryList(QStringList() << "*.json", QDir::Files);

    for (const QString& configFile : configFiles) {
        QString fullPath = instanceDir.absoluteFilePath(configFile);
        VMConfig config;
        if (config.loadFromFile(fullPath)) {
            m_instances.append(config);
        }
    }

    refreshInstanceList();
}

void MainWindow::refreshInstanceList() {
    m_instanceList->clear();

    for (const VMConfig& config : m_instances) {
        QString displayText = QString("%1 - %2 cores, %3 GB RAM")
                                 .arg(config.name())
                                 .arg(config.cpuCores())
                                 .arg(config.ramMB() / 1024);
        m_instanceList->addItem(displayText);
    }
}

void MainWindow::onNewInstance() {
    SetupWizard wizard(this);

    if (wizard.exec() == QDialog::Accepted) {
        // Create new instance configuration
        VMConfig config = VMConfig::defaultConfig();
        config.setName(wizard.instanceName());
        config.setCpuCores(wizard.cpuCores());
        config.setRamMB(wizard.ramMB());

        // Set image path
        QString imagePath = SystemChecker::getAndroidImagePath();
        if (!imagePath.isEmpty()) {
            config.setImagePath(imagePath);
        }

        // Create instance directory
        QDir instanceDir("/opt/linuxdroid/instances");
        QString instancePath = instanceDir.absoluteFilePath(config.name());
        instanceDir.mkpath(config.name());

        config.setInstancePath(instancePath);

        // Save configuration
        QString configPath = instancePath + "/config.json";
        config.saveToFile(configPath);

        m_instances.append(config);
        refreshInstanceList();

        QMessageBox::information(this, "Instance Created",
                               "Instance '" + config.name() + "' created successfully!");
    }
}

void MainWindow::onStartInstance() {
    int row = m_instanceList->currentRow();
    if (row < 0 || row >= m_instances.size()) {
        return;
    }

    VMConfig& config = m_instances[row];

    if (!config.isValid()) {
        QMessageBox::warning(this, "Invalid Configuration",
                           "Cannot start instance: " + config.validationError());
        return;
    }

    m_statusLabel->setText("Starting " + config.name() + "...");
    m_currentInstance = &config;

    if (m_qemuManager->startVM(config)) {
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
    } else {
        QMessageBox::critical(this, "Start Failed",
                            "Failed to start instance: " + m_qemuManager->getStatus());
        m_statusLabel->setText("Failed to start");
    }
}

void MainWindow::onStopInstance() {
    if (m_qemuManager->isRunning()) {
        m_qemuManager->stopVM();
    }
}

void MainWindow::onDeleteInstance() {
    int row = m_instanceList->currentRow();
    if (row < 0 || row >= m_instances.size()) {
        return;
    }

    const VMConfig& config = m_instances[row];

    auto reply = QMessageBox::question(this, "Delete Instance",
                                      QString("Are you sure you want to delete '%1'?").arg(config.name()),
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Delete instance directory
        QDir instanceDir(config.instancePath());
        instanceDir.removeRecursively();

        m_instances.removeAt(row);
        refreshInstanceList();

        QMessageBox::information(this, "Deleted", "Instance deleted successfully.");
    }
}

void MainWindow::onSettings() {
    QMessageBox::information(this, "Settings", "Settings dialog not yet implemented.");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About LinuxDroid",
                      "<h2>LinuxDroid 1.0.0</h2>"
                      "<p>Professional Android Emulator for Linux</p>"
                      "<p>Powered by QEMU and KVM</p>"
                      "<p>Copyright © 2024 Dharun Ashokkumar</p>"
                      "<p>contact@tripletech.com</p>");
}

void MainWindow::onInstanceSelected() {
    bool hasSelection = m_instanceList->currentRow() >= 0;
    m_startButton->setEnabled(hasSelection && !m_qemuManager->isRunning());
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onVMStarted() {
    m_statusLabel->setText("Running: " + (m_currentInstance ? m_currentInstance->name() : "Unknown"));
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);

    m_trayIcon->showMessage("LinuxDroid", "Android instance started", QSystemTrayIcon::Information, 3000);
}

void MainWindow::onVMStopped() {
    m_statusLabel->setText("Stopped");
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_currentInstance = nullptr;
}

void MainWindow::onVMError(const QString& error) {
    m_statusLabel->setText("Error: " + error);
    QMessageBox::critical(this, "VM Error", error);
}
