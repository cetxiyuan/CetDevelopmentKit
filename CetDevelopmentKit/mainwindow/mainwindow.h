#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSettings>

#include "cettoolplugincontext.h"

#include "ec200uconfigure.h"
#include "adbconfigure.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    enum ConfigType {
        CONFIG_EC200U_DPATH = 0,
        CONFIG_ADB_ZIPEXPORT,
    };
    Q_ENUM(ConfigType)

private slots:
    void doCurrentIndexChanged(int index);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_processHandleButton_clicked();

    void on_processConfigureButton_clicked();

private:
    Ui::MainWindow *ui;
    CetToolPluginContext *const m_toolPluginCtx;
    QSettings *const m_settings;
    QProcess *const m_cmdProcess;
    EC200UConfigure *m_ec200uConfigure;
    ADBConfigure *m_adbConfigure;
};
#endif // MAINWINDOW_H
