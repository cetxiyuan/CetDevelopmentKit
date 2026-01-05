#include "adbconfigure.h"
#include "ui_adbconfigure.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QProcess>
#include <QPlainTextEdit>

#include <QDebug>

#define ADB_SETTINGS_PREFIX      "ADBConfig"

ADBConfigure::ADBConfigure(QProcess *process, QPlainTextEdit *plainTextEdit, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ADBConfigure)
    , m_settings(new QSettings("Configs/ADBConfigure.ini", QSettings::IniFormat))
    , m_process(process)
    , m_plainTextEdit(plainTextEdit)
{
    ui->setupUi(this);
    ui->usernameLineEdit->setPlaceholderText(tr("登入设备的用户名 ..."));
    ui->passwordLineEdit->setPlaceholderText(tr("登入设备的密码 ..."));
    ui->devicePackPathLineEdit->setPlaceholderText(tr("设备中的待打包的路径 ..."));

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString toolPath = m_settings->value(QLatin1String("toolPath")).toString();
    QString username = m_settings->value(QLatin1String("username")).toString();
    QString password = m_settings->value(QLatin1String("password")).toString();
    QString packPath = m_settings->value(QLatin1String("packPath")).toString();
    QString outputDir = m_settings->value(QLatin1String("outputDir"), desktopPath).toString();
    m_settings->endGroup();

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
    QString toolPath = ui->toolPathLineEdit->text().remove('\"');
    if (!QFile::exists(toolPath)) {
        QMessageBox::warning(this, tr("警告"), 
            tr("工具路径(%1)：路径无效 ！ ").arg(toolPath));
        return QString();
    }
    m_step = 0;
    return tr("\"%1\" shell").arg(toolPath);
}

void ADBConfigure::recvHandle(const QByteArray &data)
{
    QByteArray input;
    static QString packPath;

    if (data.contains("ogin")) {
        input = ui->usernameLineEdit->text().toUtf8();
    } else if (data.contains("assword")) {
        input = ui->passwordLineEdit->text().toUtf8();
        emit progress(20);
    } else if (data.contains("#")) {
        switch (m_step) {
        case 0: { // 删除 历史打包文件
            input = tr("rm /tmp/cetmp*.tar.gz").toUtf8();
            m_step = 1;
            emit progress(45);
            break;
        }
        case 1: { // 打包
            packPath = tr("/tmp/cetmp_%1.tar.gz").arg(
                QDateTime::currentDateTime().toString("yyyy-MM-ddThh-mm-ss"));
            input = tr("tar -zcf %1 %2/*").arg(packPath, ui->devicePackPathLineEdit->text()).toUtf8();
            m_step = 2;
            emit progress(65);
            break;
        }
        case 2: // 关闭当前通道
            m_process->close();
            QTimer::singleShot(1000, this, [=]() { // 1000ms 后启动进程执行导出压缩包操作
                QString command = tr("%1 pull %2 %3").arg(ui->toolPathLineEdit->text(),
                    packPath, ui->outputPathLineEdit->text());
                m_plainTextEdit->appendPlainText("[执行] " + command);

                QStringList commands = QProcess::splitCommand(command);
                QString program = commands.first();
                QStringList arguments(commands.mid(1));
                m_process->start(program, arguments);
                emit progress(100);
            });
            m_step = -1;
            emit progress(85);
            break;
        }
    }

    //qDebug() << "data" << data << "input" << input;
    if (!input.isEmpty()) {
        m_plainTextEdit->appendPlainText("[执行] " + input);
        m_process->write(input.append("\n"));
    }
}

void ADBConfigure::on_exeToolButton_clicked()
{
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString toolPath = m_settings->value(QLatin1String("toolPath")).toString();
    m_settings->endGroup();

    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个工具路径"), 
        toolPath, "可执行文件 (*.exe)");
    qDebug() << "fileName" << fileName;
    if (!fileName.isEmpty()) {
        fileName = tr("\"%1\"").arg(fileName);
        ui->toolPathLineEdit->setText(fileName);

        m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("toolPath"), fileName);
        m_settings->endGroup();
    }
}

void ADBConfigure::on_usernameLineEdit_textChanged(const QString &arg1)
{
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    m_settings->setValue(QLatin1String("username"), arg1);
    m_settings->endGroup();
}

void ADBConfigure::on_passwordLineEdit_textChanged(const QString &arg1)
{
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    m_settings->setValue(QLatin1String("password"), arg1);
    m_settings->endGroup();
}

void ADBConfigure::on_devicePackPathLineEdit_textChanged(const QString &arg1)
{
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    m_settings->setValue(QLatin1String("packPath"), arg1);
    m_settings->endGroup();
}

void ADBConfigure::on_outputToolButton_clicked()
{
    m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
    QString outputDir = m_settings->value(QLatin1String("outputDir")).toString();
    m_settings->endGroup();
    
    QString directory = QFileDialog::getExistingDirectory(this, tr("打开一个输出目录"), outputDir);
    if (!directory.isEmpty()) {
        ui->outputPathLineEdit->setText(directory);
        
        m_settings->beginGroup(QLatin1String(ADB_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("outputDir"), directory);
        m_settings->endGroup();
    }
}

