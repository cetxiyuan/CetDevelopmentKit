#include "mainwindow.h"

#include <QTextCodec>
#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QApplication::setStyle(QStyleFactory::create(QStyleFactory::keys().last()));
    
    MainWindow w;
    w.show();
    return a.exec();
}
