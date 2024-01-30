#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"

#include <QMessageBox>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include <QTextCodec>

#define INIT_INTERFACE(mInterface, name)    \
    mInterface(qobject_cast<typeof(mInterface)>(loadPlugin(name)))
#define LOAD_INTERFACE(mInterface, name)    \
    mInterface = qobject_cast<typeof(mInterface)>(loadPlugin(name))

#define CETDEVELOPMENTKIT_VERSION  "Cet开发集成工具包 V" APP_VERSION " " TIP_VERSION

#define CETDKIT_SETTINGS_PREFIX      "CETDKITConfig"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , INIT_INTERFACE(m_cetLogManagerInterface, "CetLogManagerPlugin.dll")
    , m_settings(new QSettings("Configs/AppMaster.ini", QSettings::IniFormat))
    , m_cmdProcess(new QProcess(this))
    , m_ec200uConfigure(new EC200UConfigure(m_cmdProcess))
    , m_adbConfigure(new ADBConfigure(m_cmdProcess))
    
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    ui->setupUi(this);
    setWindowTitle(tr(CETDEVELOPMENTKIT_VERSION));

    ui->progressBar->setMaximum(100);
    ui->progressBar->setMinimum(0);
    ui->configureGroupBox->setVisible(false);
    ui->configureGroupBox->layout()->setMargin(0);
    ui->configureGroupBox->layout()->addWidget(m_ec200uConfigure);
    ui->configureGroupBox->layout()->addWidget(m_adbConfigure);

    // 工具包种类
    ui->processComboBox->addItem("EC200U 差分包制作", CONFIG_EC200U_DPATH);
    ui->processComboBox->addItem("ADB 设备数据打包导出", CONFIG_ADB_ZIPEXPORT);
    
    m_settings->beginGroup(QLatin1String(CETDKIT_SETTINGS_PREFIX));
    int processIndex = m_settings->value(QLatin1String("processIndex")).toInt();
    m_settings->endGroup();
    ui->processComboBox->setCurrentIndex(processIndex);
    doCurrentIndexChanged(processIndex);

    initFeaturesPlugin();
    connect(m_cmdProcess, static_cast<void (QProcess:: *)(int exitCode, QProcess::ExitStatus exitStatus)>
        (&QProcess::finished), this, &MainWindow::onProcessFinished);
    connect(m_cmdProcess, &QProcess::readyReadStandardOutput, this, [=]() {
            QByteArray data = m_cmdProcess->readAllStandardOutput();
            ui->plainTextEdit->appendPlainText(data);

            ConfigType type = ui->processComboBox->currentData().value<ConfigType>();
            switch (type) {
            case CONFIG_EC200U_DPATH:
                m_ec200uConfigure->recvHandle(data);
                break;
            case CONFIG_ADB_ZIPEXPORT:
                m_adbConfigure->recvHandle(data);
                break;
            default: break;
            }
        });
    connect(m_cmdProcess, &QProcess::readyReadStandardError, this, [=]() {
            QByteArray data = m_cmdProcess->readAllStandardError();
            ui->plainTextEdit->appendPlainText(data);
        });

    connect(m_ec200uConfigure, &EC200UConfigure::progress, ui->progressBar, &QProgressBar::setValue);
    connect(m_adbConfigure, &ADBConfigure::progress, ui->progressBar, &QProgressBar::setValue);
    connect(ui->processComboBox, static_cast<void (QComboBox:: *)(int)>
        (&QComboBox::currentIndexChanged), this, &MainWindow::doCurrentIndexChanged);
}

MainWindow::~MainWindow()
{
    delete m_cmdProcess;
    delete ui;
}

QObject *MainWindow::loadPlugin(const QString &dllName)
{
    QObject *plugin = nullptr;
    QDir pluginsDir("D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins");

    if (!pluginsDir.exists()) {
        pluginsDir.setPath("./plugins");
    }

    foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
        if (0 == dllName.compare(fileName, Qt::CaseInsensitive)) {
            QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
            plugin = pluginLoader.instance();
            if (plugin) {
                plugin->setParent(this);
            }
            break;
        }
    }

    return plugin;
}

void MainWindow::initFeaturesPlugin()
{
    ui->logManagerMenu->addAction(tr("控制"), this, [=]() {
            m_cetLogManagerInterface->show();
        });

    LOAD_INTERFACE(m_cetLicenseInterface, "CetLicensePlugin.dll");
    if (!m_cetLicenseInterface) {
        QMessageBox::warning(this, tr("警告"), tr("缺少 CetLicensePlugin.dll 库\t"));
    }
    if (!m_cetLicenseInterface || CetLicenseInterface::ACTIVATE_OK
            != m_cetLicenseInterface->activate(PRODUCT_NAME)) {
        this->centralWidget()->setEnabled(false);

        QString expireInfo = tr(" [ 已到期 (%1) (%2) ]").arg(PRODUCT_NAME, 
            m_cetLicenseInterface? m_cetLicenseInterface->solidKey() : "");
        this->setWindowTitle(windowTitle().append(expireInfo));
        this->centralWidget()->setStatusTip(tr("%1 请激活后使用！").arg(expireInfo));
    }
    ui->licenseMenu->addAction(tr("激活"), this, [=]() {
        if (m_cetLicenseInterface) {
            int result = m_cetLicenseInterface->execResult();
            if (CetLicenseInterface::ACTIVATE_OK == result) {
                this->setWindowTitle(tr(CETDEVELOPMENTKIT_VERSION));
                this->centralWidget()->setEnabled(true);
            }

            if (CetLicenseInterface::ACTIVATE_CANCEL != result) {
                QMessageBox::information(this, tr("通知"), 
                    m_cetLicenseInterface->resultString() + "\t", QMessageBox::Ok);
            }
        }
    });

    LOAD_INTERFACE(m_cetUpdateInterface, "CetUpdatePlugin.dll");
    if (!m_cetUpdateInterface) {
        QMessageBox::warning(this, tr("警告"), tr("缺少 CetUpdatePlugin.dll 库\t"));
    }
    if (m_cetUpdateInterface) {
        m_cetUpdateInterface->checkUpdate(APP_NAME, APP_VERSION, QCoreApplication::quit);
    }

    LOAD_INTERFACE(m_cetProgressInterface, "CetProgressPlugin.dll");
}

void MainWindow::doCurrentIndexChanged(int index)
{
    ui->plainTextEdit->clear();
    m_ec200uConfigure->setVisible(false);
    m_adbConfigure->setVisible(false);
    
    ConfigType type = ui->processComboBox->itemData(index).value<ConfigType>();
    qDebug() << "on_processComboBox_currentIndexChanged type" << type;
    switch (type) {
    case CONFIG_EC200U_DPATH:
        m_ec200uConfigure->setVisible(true);
        break;
    case CONFIG_ADB_ZIPEXPORT:
        m_adbConfigure->setVisible(true);
        break;
    default: break;
    }
    
    qDebug() << "processIndex" << ui->processComboBox->currentIndex();
    m_settings->beginGroup(QLatin1String(CETDKIT_SETTINGS_PREFIX));
    m_settings->setValue(QLatin1String("processIndex"), ui->processComboBox->currentIndex());
    m_settings->endGroup();
}


void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "exitCode" << exitCode << "exitStatus" << exitStatus;

    if (QProcess::NormalExit == exitStatus) {
        ui->progressBar->setValue(100);
    } else {
        qDebug() << "进程错误退出！" << exitStatus;
    }

    m_cmdProcess->close();
}

void MainWindow::on_processHandleButton_clicked()
{
    if (m_cmdProcess->isOpen()) {
        QMessageBox::warning(this, tr("警告"), 
            tr("进程：%1 已经在运行中！").arg(m_cmdProcess->program()));
        return;
    }

    QString command;
    ConfigType type = ui->processComboBox->currentData().value<ConfigType>();
    switch (type) {
    case CONFIG_EC200U_DPATH:
        command = m_ec200uConfigure->startCommand();
        break;
    case CONFIG_ADB_ZIPEXPORT:
        command = m_adbConfigure->startCommand();
        break;
    default: break;
    }
    qDebug() << "command" << command;
    ui->progressBar->setValue(0);
    m_cmdProcess->start(command);
}

void MainWindow::on_processConfigureButton_clicked()
{
    if ("▼" == ui->processConfigureButton->text()) {
        ui->processConfigureButton->setText("▲");
        ui->configureGroupBox->setVisible(true);
    } else {
        ui->processConfigureButton->setText("▼");
        ui->configureGroupBox->setVisible(false);
    }
}

