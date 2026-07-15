#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include "cettooldlls_hashes.h"
#include <QMessageBox>
#include <QPluginLoader>
#include <QStatusBar>
#include <QDir>
#include <QTimer>
#include <QTextCodec>
#include <QCryptographicHash>
#include <QFile>

#include <QDebug>

#define CETDEVELOPMENTKIT_VERSION  "Cet开发集成工具包 V" APP_VERSION " " TIP_VERSION

#define CETDKIT_SETTINGS_PREFIX      "CETDKITConfig"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_toolPluginCtx(new CetToolPluginContext(HASH_CetProductWizard_DLL, this))
    , m_settings(new QSettings("Configs/AppMaster.ini", QSettings::IniFormat))
    , m_cmdProcess(new QProcess(this))
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    ui->setupUi(this);
    setWindowTitle(tr(CETDEVELOPMENTKIT_VERSION));

    m_ec200uConfigure = new EC200UConfigure(m_cmdProcess, ui->plainTextEdit);
    m_adbConfigure = new ADBConfigure(m_cmdProcess, ui->plainTextEdit);

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

    m_toolPluginCtx->initFeatures(ui->logManagerMenu, ui->licenseMenu,
        PRODUCT_NAME, tr(CETDEVELOPMENTKIT_VERSION), APP_NAME, APP_VERSION);
    connect(m_cmdProcess, static_cast<void (QProcess:: *)(int exitCode, QProcess::ExitStatus exitStatus)>
        (&QProcess::finished), this, &MainWindow::onProcessFinished);
    connect(m_cmdProcess, &QProcess::readyReadStandardOutput, this, [=]() {
            QByteArray data = m_cmdProcess->readAllStandardOutput();
//            qDebug() << "StandardOutput:data" << data;
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
//            qDebug() << "StandardError:data" << data;
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
#if 1
    m_cmdProcess->close();
#else
    if (m_cmdProcess->isOpen()) {
        QMessageBox::warning(this, tr("警告"), 
            tr("进程：%1 已经在运行中！").arg(m_cmdProcess->program()));
        return;
    }
#endif

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
    ui->plainTextEdit->appendPlainText("[执行] " + command);

    QStringList commands = QProcess::splitCommand(command);
    QString program = commands.first();
    QStringList arguments(commands.mid(1));
//    qDebug() << "program" << program << "arguments" << arguments;

    m_cmdProcess->start(program, arguments);
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

