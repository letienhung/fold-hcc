#pragma once

#include <QWidget>
#include "ui_VideoToolbar.h"

class VideoToolbar : public QWidget{
    Q_OBJECT

public:
    Ui::VideoToolbarWidget * ui;

public:
    VideoToolbar() : ui(new Ui::VideoToolbarWidget)
    {
        ui->setupUi(this);

        ui->pauseLabel->setVisible(false);
        ui->playLabel->setVisible(false);

        connect(ui->playButton, SIGNAL(clicked()), SLOT(togglePlay()));
        connect(ui->slider, SIGNAL(valueChanged(int)), SLOT(sliderChanged(int)));

        isPlaying = false;
    }

    bool isPlaying;

public slots:
    void togglePlay()
    {
        QString label = ui->playButton->text();

        QString playLabel = ui->playLabel->text();
        QString pauseLabel = ui->pauseLabel->text();

        if(label != pauseLabel)
        {
            ui->playButton->setText(pauseLabel);
            isPlaying = true;
        }
        else
        {
            ui->playButton->setText(playLabel);
            isPlaying = false;
        }
    }

    void sliderChanged(int value)
    {
        QString label = ui->playButton->text();
        QString playLabel = ui->playLabel->text();

        if(!isPlaying && label == playLabel)
        {
            emit valueChanged(value);
        }
    }

signals:
    void valueChanged(int);

};
