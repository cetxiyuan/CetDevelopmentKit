#ifndef ADBCONFIGURE_H
#define ADBCONFIGURE_H

#include <QWidget>
#include <QProcess>
#include <QSettings>

namespace Ui {
class ADBConfigure;
}

class ADBConfigure : public QWidget
{
    Q_OBJECT

public:
    explicit ADBConfigure(QProcess *process, QWidget *parent = nullptr);
    ~ADBConfigure();
    QString startCommand(void);
    void recvHandle(const QByteArray &data);

Q_SIGNALS:
    void progress(int value);

private slots:
    void on_exeToolButton_clicked();

    void on_usernameLineEdit_textChanged(const QString &arg1);

    void on_passwordLineEdit_textChanged(const QString &arg1);

    void on_devicePackPathLineEdit_textChanged(const QString &arg1);

    void on_outputToolButton_clicked();

private:
    Ui::ADBConfigure *ui;
    QSettings *const m_settings;
    QProcess *const m_process;
};

#endif // ADBCONFIGURE_H
