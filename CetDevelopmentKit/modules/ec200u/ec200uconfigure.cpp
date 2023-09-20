#include "ec200uconfigure.h"
#include "ui_ec200uconfigure.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QFile>

#define EC200U_SETTINGS_PATH        "./Settings.ini"
#define EC200U_SETTINGS_PREFIX      "EC200UConfig"

EC200UConfigure::EC200UConfigure(QProcess *process, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EC200UConfigure)
    , m_process(process)
{
    ui->setupUi(this);

    QSettings setting(EC200U_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString toolPath = setting.value(QLatin1String("toolPath")).toString();
    QString oldPacket = setting.value(QLatin1String("oldPacket")).toString();
    QString newPacket = setting.value(QLatin1String("newPacket")).toString();
    QString outputDir = setting.value(QLatin1String("outputDir")).toString();
    setting.endGroup();

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
    QString toolexe = ui->toolPathLineEdit->text();
    QString oldPacket = ui->oldPacketLineEdit->text();
    QString newPacket = ui->newPacketLineEdit->text();
    QString outputDir = ui->outputDirLineEdit->text();
    QString outputName = ui->outputNameLineEdit->text();
    if (outputName.isEmpty()) {
        outputName.append("output.pac");
    }

    QFileInfo fileInfo(toolexe);
    QString command = tr("%1 fotacreate2 --pac %2,%3,%4\\setting\\fota8910.xml %5\\%6 -d v")
        .arg(toolexe, oldPacket, newPacket, fileInfo.absolutePath(), outputDir, outputName);
    
    return command;
}

void EC200UConfigure::recvHandle(const QByteArray &data)
{
    Q_UNUSED(data);
    
    emit progress(100);
}

void EC200UConfigure::on_toolPathButton_clicked()
{
    QSettings setting(EC200U_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString toolPath = setting.value(QLatin1String("toolPath")).toString();
    setting.endGroup();

    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个工具路径"), 
        toolPath, "可执行文件 (*.exe)");
    if (!fileName.isEmpty()) {
        ui->toolPathLineEdit->setText(fileName);

        setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("toolPath"), fileName);
        setting.endGroup();
    }
}

void EC200UConfigure::on_oldPacketButton_clicked()
{
    QSettings setting(EC200U_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString oldPacket = setting.value(QLatin1String("oldPacket")).toString();
    setting.endGroup();
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个固件包"), 
        oldPacket, "固件包 (*.pac);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        ui->oldPacketLineEdit->setText(fileName);

        setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("oldPacket"), fileName);
        setting.endGroup();
    }
}

void EC200UConfigure::on_newPacketButton_clicked()
{
    QSettings setting(EC200U_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString newPacket = setting.value(QLatin1String("newPacket")).toString();
    setting.endGroup();
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开一个固件包"), 
        newPacket, "固件包 (*.pac);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        ui->newPacketLineEdit->setText(fileName);
        
        setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("newPacket"), fileName);
        setting.endGroup();
    }
}

void EC200UConfigure::on_outputDirButton_clicked()
{
    QSettings setting(EC200U_SETTINGS_PATH, QSettings::IniFormat);
    setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    QString outputDir = setting.value(QLatin1String("outputDir")).toString();
    setting.endGroup();
    
    QString directory = QFileDialog::getExistingDirectory(this, tr("打开一个输出目录"), outputDir);
    if (!directory.isEmpty()) {
        ui->outputDirLineEdit->setText(directory);
        
        setting.beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
        setting.setValue(QLatin1String("outputDir"), directory);
        setting.endGroup();
    }
}
