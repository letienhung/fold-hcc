#include "MainWindow.h"
#include <QApplication>
#include <QGLWidget>

int main(int argc, char *argv[])
{
    // Anti-aliasing
    {
        QGLFormat glf = QGLFormat::defaultFormat();
        glf.setSamples(8);
        QGLFormat::setDefaultFormat(glf);
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
