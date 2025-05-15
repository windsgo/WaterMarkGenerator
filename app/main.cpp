#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QGuiApplication>
#include <QScreen>

#ifdef BUILD_FOR_ANDROID
#include "MainWindowAndroid.h"
#else
#include "mainwindow.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "watermark_qt6_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

#ifdef BUILD_FOR_ANDROID
    MainWindowAndroid w;
    w.show();
    w.setFixedSize(w.size());
#else
    MainWindow w;
    w.show();
#endif

    return a.exec();
}
