#include "adbconfigure.h"
#include "ui_adbconfigure.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>
#include <QDir>
#include <QDebug>

#define ADB_SETTINGS_PATH        "./Settings.ini"
#define ADB_SETTINGS_PREFIX      "ADBConfig"

ADBConfigure::ADBConfigure(QProcess *process, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ADBConfigure)
    , m_process(process)
{
    ui->setupUi(this);
    ui->usernameLineEdit->setPlaceholderText(tr("登入设备的用户名 ..."));
    ui->passwordLineEdit->setPlaceholderText(tr("登入设备的密码 ..."));
    ui->devicePackPathLineEdit->setPlaceholderText(tr("设备中的待打包的路径 ..."));

    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString toolPath = setting.value(QLatin1String("toolPath")).toString();
    QString username = setting.value(QLatin1String("username")).toString();
    QString password = setting.value(QLatin1String("password")).toString();
    QString packPath = setting.value(QLatin1String("packPath")).toString();
    QString outputDir = setting.value(QLatin1String("outputDir")).toString();
    setting.endGroup();

    if (!toolPath.isEmpty()) ui->toolPathLineEdit->setText(toolPath);
    if (!username.isEmpty()) ui->usernameLineEdit->setText(username);
    if (!password.isEmpty()) ui->passwordLineEdit->setText(password);
    if (!packPath.isEmpty()) ui->devicePackPathLineEdit->setText(packPath);
    if (!outputDir.isEmpty()) ui->outputPathLineEdit->setText(outputDir);
}

ADBConfigure::~ADBConfigure()
{
    delete ui;
}

QString ADBConfigure::startCommand(void)
{
    return tr("%1 shell").arg(ui->toolPathLineEdit->text());
}

void ADBConfigure::recvHandle(const QByteArray &data)
{
    static int step = 0;
    QByteArray input;
    if (data.contains("login")) {
        input = ui->usernameLineEdit->text().toUtf8();
    } else if (data.contains("Password")) {
        input = ui->passwordLineEdit->text().toUtf8();
        step = 0;
        emit progress(30);
    } else if (data.contains(ui->usernameLineEdit->text().toUtf8()) && data.contains("#")) {
        switch (step) {
        case 0: { // 打包
            QString path = ui->devicePackPathLineEdit->text();
            input = tr("tar -zcf /tmp/cetmp.tar.gz %1/*").arg(path).toUtf8();
            step = 1;
            emit progress(50);
            break;
        }
        case 1: // 关闭当前通道
            m_process->close();
            QTimer::singleShot(1000, this, [=]() { // 1000ms 后启动进程执行导出压缩包操作
                QByteArray command = tr("adb pull /tmp/cetmp.tar.gz %1\n").arg(
                    ui->outputPathLineEdit->text()).toUtf8();
                m_process->start(command);
                emit progress(100);
            });
            step = -1;
            emit progress(80);
            break;
        }
    }

    //qDebug() << "data" << data << "input" << input;
    if (!input.isEmpty()) 
        m_process->write(input.append("\n"));
}

void ADBConfigure::on_exeToolButton_clicked()
{
    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString toolPath = setting.value(QLatin1String("toolPath")).toString();
    setting.endGroup();
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个工具路径"), 
        toolPath, "可执行文件 (*.exe)");
    if (!fileName.isEmpty()) {
        ui->toolPathLineEdit->setText(fileName);
        
        setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("toolPath"), fileName);
        setting.endGroup();
    }
}

void ADBConfigure::on_usernameLineEdit_textChanged(const QString &arg1)
{
    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    setting.setValue(QLatin1String("username"), arg1);
    setting.endGroup();
}

void ADBConfigure::on_passwordLineEdit_textChanged(const QString &arg1)
{
    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    setting.setValue(QLatin1String("password"), arg1);
    setting.endGroup();
}

void ADBConfigure::on_devicePackPathLineEdit_textChanged(const QString &arg1)
{
    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    setting.setValue(QLatin1String("packPath"), arg1);
    setting.endGroup();
}

void ADBConfigure::on_outputToolButton_clicked()
{
    QSettings setting(ADB_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString outputDir = setting.value(QLatin1String("outputDir")).toString();
    setting.endGroup();
    
    QString directory = QFileDialog::getExistingDirectory(this, tr("打开一个输出目录"), outputDir);
    if (!directory.isEmpty()) {
        ui->outputPathLineEdit->setText(directory);
        
        setting.beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("outputDir"), directory);
        setting.endGroup();
    }
}

