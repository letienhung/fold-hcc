#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVariant>
#include "FoldManager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    Ui::MainWindow *ui;

	FoldManager * fmanager;

	QMap< QString, QMap<QString,QVariant> > shapeParamters;

public slots:
	void loadShape( QString shapeName );
    void foldabilize();
};

#endif // MAINWINDOW_H
