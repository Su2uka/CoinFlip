//
// Created by PC on 24-12-9.
//

#ifndef CHOOSELEVELSCENE_H
#define CHOOSELEVELSCENE_H

#include <QMainWindow>
#include "playscene.h"
#include "levelmanager.h"
#include "mypushbutton.h"


class ChooseLevelScene : public QMainWindow {
    Q_OBJECT

public:
    explicit ChooseLevelScene(QWidget *parent = nullptr);

    //重写绘图事件
    void paintEvent(QPaintEvent *event) override;

    //游戏场景对象指针
    PlayScene * play = nullptr;

    //关卡管理指针
    LevelManager *levelManager;

    MyPushButton * pushButtonArray[20];

    // 更新按钮图标
    void updateLevelButtonIcons();

signals:
    //自定义信号，告诉主场景 点击了返回
    void chooseSceneBack();

public slots:
};


#endif //CHOOSELEVELSCENE_H
