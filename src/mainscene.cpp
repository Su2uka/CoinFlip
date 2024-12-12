//
// Created by PC on 24-12-8.
//


#include "mainscene.h"
#include "ui_MainScene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>
#include <QTimer>
#include <QSoundEffect>

MainScene::MainScene(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainScene) {
    ui->setupUi(this);

    //配置主场景

    //设置固定大小
    setFixedSize(320, 588);

    //设置图标
    setWindowIcon(QIcon(":/Coin0001.png"));

    //设置标题
    setWindowTitle("翻金币 (CoinFlip)");

    //退出按钮实现
    connect(ui->actionquit, &QAction::triggered, [=]() {
        this->close();
    });

    //准备开始按钮的音效
    QSoundEffect *startSound = new QSoundEffect(this);
    startSound->setSource(QUrl::fromLocalFile(":/TapButtonSound.wav"));
    //返回按钮音效
    QSoundEffect *backSound = new QSoundEffect(this);
    backSound->setSource(QUrl::fromLocalFile(":/BackButtonSound.wav"));

    //开始按钮
    MyPushButton *startBth = new MyPushButton(":/MenuSceneStartButton.png");
    startBth->setParent(this);
    startBth->move(this->width() * 0.5 - startBth->width() * 0.5, this->height() * 0.7);

    //实例化选择关卡场景
    chooseScene = new ChooseLevelScene;

    //监听选择关卡的返回按钮的信号
    connect(chooseScene, &ChooseLevelScene::chooseSceneBack, [=]() {
        backSound->play();
        QTimer::singleShot(200, this, [=]() {
            //设置chooseScene场景位置
            chooseScene->setGeometry(this->geometry());
            chooseScene->hide();
            this->show();
        });
    });

    connect(startBth, &QPushButton::clicked, [=]() {
        //播放开始音效
        if (startSound->status() == QSoundEffect::Ready) {
            // qDebug() << "Sound effect loaded successfully.";
            startSound->play(); // 播放音效
        } else if (startSound->status() == QSoundEffect::Error) {
            // qDebug() << "Error: Failed to load sound effect.";
        }

        startBth->zoom1();
        startBth->zoom2();

        //延时进入到选择关卡场景
        QTimer::singleShot(200, this, [=]() {
            //进入到选择关卡场景中
            //设置chooseScene场景位置
            chooseScene->setGeometry(this->geometry());
            //自身隐藏
            this->hide();
            //显示选择关卡场景
            chooseScene->show();
        });
    });
}

MainScene::~MainScene() {
    delete ui;
}

void MainScene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap pix;
    pix.load(":/PlayLevelSceneBg.png");
    painter.drawPixmap(0, 0, this->width(), this->height(), pix);

    //画背景上图标
    pix.load(":/Title.png");
    pix = pix.scaled(pix.width() * 0.5, pix.height() * 0.5);
    painter.drawPixmap(10, 30, pix);
}
