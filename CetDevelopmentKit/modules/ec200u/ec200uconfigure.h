#ifndef EC200UCONFIGURE_H
#define EC200UCONFIGURE_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class EC200UConfigure;
}

class QProcess;
class QPlainTextEdit;

class EC200UConfigure : public QWidget
{
    Q_OBJECT

public:
    explicit EC200UConfigure(QProcess *process, 
        QPlainTextEdit *plainTextEdit, QWidget *parent = nullptr);
    ~EC200UConfigure();
    QString startCommand(void);
    void recvHandle(const QByteArray &data);

Q_SIGNALS:
    void progress(int value);

private slots:
    void on_toolPathButton_clicked();

    void on_oldPacketButton_clicked();

    void on_newPacketButton_clicked();

    void on_outputDirButton_clicked();

private:
    Ui::EC200UConfigure *ui;
    QSettings *const m_settings;
    QProcess *const m_process;
    QPlainTextEdit *const m_plainTextEdit;
};

#endif // EC200UCONFIGURE_H
