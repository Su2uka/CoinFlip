//
// Created by PC on 24-12-8.
//

#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QMainWindow>
#include "chooselevelscene.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainScene;
}

QT_END_NAMESPACE

class MainScene : public QMainWindow {
    Q_OBJECT

public:
    explicit MainScene(QWidget *parent = nullptr);

    ~MainScene() override;

    //重写paintEvent事件 画背景图
    void paintEvent(QPaintEvent *event) override;

    ChooseLevelScene *chooseScene = nullptr;

private:
    Ui::MainScene *ui;
};


#endif //MAINSCENE_H
