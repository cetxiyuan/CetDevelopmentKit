#include "mainwindow.h"

#include <QTextCodec>
#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    /* 支持程序支持高分辨率自动调整 */
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    /* 设置高 DPI 缩放因子的四舍五入策略 */
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QStringList list = QStyleFactory::keys();
    QApplication::setStyle(QStyleFactory::create(list.last()));
    
    MainWindow w;
    w.show();
    return a.exec();
}
