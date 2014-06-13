#include "mainwindow.h"
#include <QDir>
#include <QFileInfo>

int main(int argc, char *argv[])
{
#ifdef Q_OS_OSX
    foreach (const QString &path, QApplication::libraryPaths()) {
        if (!QDir(path).exists()) QApplication::removeLibraryPath(path);
    }

    QApplication::addLibraryPath(QFileInfo(argv[0]).absoluteDir().absolutePath() + "/../PlugIns");
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

