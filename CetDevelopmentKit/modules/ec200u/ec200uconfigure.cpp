#include "ec200uconfigure.h"
#include "ui_ec200uconfigure.h"

#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QFile>
#include <QProcess>

#define EC200U_SETTINGS_PREFIX      "EC200UConfig"

EC200UConfigure::EC200UConfigure(QProcess *process, QPlainTextEdit *plainTextEdit, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EC200UConfigure)
    , m_settings(new QSettings("Configs/EC200UConfigure.ini", QSettings::IniFormat))
    , m_process(process)
    , m_plainTextEdit(plainTextEdit)
{
    ui->setupUi(this);

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString toolPath = m_settings->value(QLatin1String("toolPath")).toString();
    QString oldPacket = m_settings->value(QLatin1String("oldPacket")).toString();
    QString newPacket = m_settings->value(QLatin1String("newPacket")).toString();
    QString outputDir = m_settings->value(QLatin1String("outputDir"), desktopPath).toString();
    m_settings->endGroup();

    if (!toolPath.isEmpty()) ui->toolPathLineEdit->setText(toolPath);
    if (!oldPacket.isEmpty()) ui->oldPacketLineEdit->setText(oldPacket);
    if (!newPacket.isEmpty()) ui->newPacketLineEdit->setText(newPacket);
    if (!outputDir.isEmpty()) ui->outputDirLineEdit->setText(outputDir);
}

EC200UConfigure::~EC200UConfigure()
{
    delete ui;
}

QString EC200UConfigure::startCommand(void)
{
    QString toolPath = ui->toolPathLineEdit->text();
    QString oldPacket = ui->oldPacketLineEdit->text();
    QString newPacket = ui->newPacketLineEdit->text();
    QString outputDir = ui->outputDirLineEdit->text();
    QString outputName = ui->outputNameLineEdit->text();
    
    if (!QFile::exists(toolPath)) {
        QMessageBox::warning(this, tr("警告"), 
            tr("工具路径(%1)：路径无效 ！ ").arg(toolPath));
        return QString();
    }
    if (!QFile::exists(oldPacket)) {
        QMessageBox::warning(this, tr("警告"), 
            tr("原固件包(%1)：路径无效 ！ ").arg(oldPacket));
        return QString();
    }
    if (!QFile::exists(newPacket)) {
        QMessageBox::warning(this, tr("警告"), 
            tr("新固件包(%1)：路径无效 ！ ").arg(newPacket));
        return QString();
    }
    
    if (outputName.isEmpty()) {
        outputName.append("output.pac");
        ui->outputNameLineEdit->setText(outputName);
    }

    QFileInfo fileInfo(toolPath);
    QString command = tr("%1 fotacreate2 --pac %2,%3,%4\\setting\\fota8910.xml %5\\%6 -d v")
        .arg(toolPath, oldPacket, newPacket, fileInfo.absolutePath(), outputDir, outputName);
    
    return command;
}

void EC200UConfigure::recvHandle(const QByteArray &data)
{
    Q_UNUSED(data);
    
    emit progress(100);
}

void EC200UConfigure::on_toolPathButton_clicked()
{
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString toolPath = m_settings->value(QLatin1String("toolPath")).toString();
    m_settings->endGroup();

    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个工具路径"), 
        toolPath, "可执行文件 (*.exe)");
    if (!fileName.isEmpty()) {
        ui->toolPathLineEdit->setText(fileName);

        m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("toolPath"), fileName);
        m_settings->endGroup();
    }
}

void EC200UConfigure::on_oldPacketButton_clicked()
{
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString oldPacket = m_settings->value(QLatin1String("oldPacket")).toString();
    m_settings->endGroup();
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个固件包"), 
        oldPacket, "固件包 (*.pac);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        ui->oldPacketLineEdit->setText(fileName);

        m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("oldPacket"), fileName);
        m_settings->endGroup();
    }
}

void EC200UConfigure::on_newPacketButton_clicked()
{
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString newPacket = m_settings->value(QLatin1String("newPacket")).toString();
    m_settings->endGroup();
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个固件包"), 
        newPacket, "固件包 (*.pac);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        ui->newPacketLineEdit->setText(fileName);
        
        m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("newPacket"), fileName);
        m_settings->endGroup();
    }
}

void EC200UConfigure::on_outputDirButton_clicked()
{
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString outputDir = m_settings->value(QLatin1String("outputDir")).toString();
    m_settings->endGroup();
    
    QString directory = QFileDialog::getExistingDirectory(this, tr("打开一个输出目录"), outputDir);
    if (!directory.isEmpty()) {
        ui->outputDirLineEdit->setText(directory);
        
        m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        m_settings->setValue(QLatin1String("outputDir"), directory);
        m_settings->endGroup();
    }
}
