//
// Created by PC on 24-12-9.
//


#include "chooselevelscene.h"
#include <QMenuBar>
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>
#include <QLabel>
#include <QSoundEffect>
#include <QTimer>
#include "levelmanager.h"
#include <QGraphicsColorizeEffect>
#include <QMessageBox>

ChooseLevelScene::ChooseLevelScene(QWidget *parent) : QMainWindow(parent) {
    //配置选择关卡场景
    this->setFixedSize(320, 588);

    //设置图标
    this->setWindowIcon(QPixmap(":/Coin0001.png"));

    //设置标题
    setWindowTitle("选择关卡");

    //创建菜单栏
    QMenuBar *bar = menuBar();
    setMenuBar(bar);

    //创建开始菜单
    QMenu *startMenu = bar->addMenu("开始");

    //创建退出 菜单项
    QAction *quitAction = startMenu->addAction("退出");

    connect(quitAction, &QAction::triggered, [=]() {
        this->close();
    });

    //选择关卡音效
    QSoundEffect *chooseSound = new QSoundEffect(this);
    chooseSound->setSource(QUrl::fromLocalFile(":/TapButtonSound.wav"));


    //返回按钮
    MyPushButton *backBtn = new MyPushButton(":/BackButton.png", ":/BackButtonSelected.png");
    backBtn->setParent(this);
    backBtn->move(this->width() - backBtn->width() - 10, this->height() - backBtn->height() - 10);

    //点击返回
    connect(backBtn, &MyPushButton::clicked, [=]() {
        // qDebug() << "点击了返回按钮";
        //告诉主场景 我返回了 主场景监听返回
        emit this->chooseSceneBack();
    });

    //选关管理
    levelManager = new LevelManager(this);

    // levelManager->resetLevels();

    //创建选择关卡的按钮
    for (int i = 0; i < 20; ++i) {
        pushButtonArray[i] = new MyPushButton(":/LevelIcon.png");
        if (!levelManager->isLevelUnlocked(i + 1)) {
            QPixmap pixmap(":/LevelIcon.png"); // 加载图像
            pushButtonArray[i]->setIcon(QIcon(pixmap)); // 设置图标
            pushButtonArray[i]->setIconSize(pixmap.size()); // 设置图标大小

            // 创建QGraphicsColorizeEffect对象来改变颜色
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(pushButtonArray[i]);
            effect->setColor(QColor(255, 37, 37)); // 设置颜色
            effect->setStrength(0.8); // 设置强度
            pushButtonArray[i]->setGraphicsEffect(effect); // 应用效果
        }

        pushButtonArray[i]->setParent(this);
        pushButtonArray[i]->move(25 + i % 4 * 70, 150 + i / 4 * 70);

        //监听每个按钮的点击事件
        connect(pushButtonArray[i], &MyPushButton::clicked, [=]() {
            //判断关卡是否解锁
            if (!levelManager->isLevelUnlocked(i + 1)) {
                qDebug() << "未解锁";
                // 弹出提示框
                QMessageBox::information(this, "提示", QString("第%1关未解锁，请先完成前置关卡。").arg(i + 1));
                return;
            }

            //播放选择关卡音效
            chooseSound->play();

            QString str = QString("您选择的是第%1关").arg(i + 1);
            qDebug() << str;

            //进入到游戏场景
            QTimer::singleShot(200, this, [=]() {
                this->hide();
                play = new PlayScene(i + 1); //创建游戏场景

                //设置初始位置
                play->setGeometry(this->geometry());

                play->show(); //显示游戏场景

                //返回
                connect(play, &PlayScene::chooseSceneBack, [=]() {
                    this->setGeometry(play->geometry());
                    this->show();

                    if (play->isWin) {
                        levelManager->unlockNextLevel(play->levelIndex);

                        updateLevelButtonIcons();
                    }

                    delete play;
                    play = nullptr;
                });
            });
        });

        QLabel *label = new QLabel;
        label->setParent(this);
        label->setFixedSize(pushButtonArray[i]->width(), pushButtonArray[i]->height());
        label->setText(QString::number(i + 1));
        label->move(25 + i % 4 * 70, 150 + i / 4 * 70);

        // 设置 label 上的文字对齐方式
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // 设置字体：加粗和增大字体
        QFont font = label->font(); // 获取当前字体
        font.setPointSize(13); // 设置字体大小为 16
        font.setBold(true); // 设置字体加粗
        label->setFont(font); // 应用字体设置

        // 设置让鼠标进行穿透
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void ChooseLevelScene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap pix;
    pix.load(":/OtherSceneBg.png");
    painter.drawPixmap(0, 0, this->width(), this->height(), pix);

    pix.load(":/Title.png");
    painter.drawPixmap((this->width() - pix.width()) * 0.5, 30, pix);
}

void ChooseLevelScene::updateLevelButtonIcons() {
    // 遍历所有关卡按钮，检查每个关卡是否解锁
    for (int i = 0; i < 20; ++i) {
        // 如果关卡已经解锁，更新按钮图标
        if (levelManager->isLevelUnlocked(i + 1)) {
            QPixmap pixmap(":/LevelIcon.png"); // 正常图标
            pushButtonArray[i]->setIcon(QIcon(pixmap)); // 设置图标
            pushButtonArray[i]->setIconSize(pixmap.size()); // 设置图标大小
            // 去掉加深效果
            pushButtonArray[i]->setGraphicsEffect(nullptr);
        } else {
            // 如果关卡未解锁，保持加深效果
            QPixmap pixmap(":/LevelIcon.png");
            pushButtonArray[i]->setIcon(QIcon(pixmap));
            pushButtonArray[i]->setIconSize(pixmap.size());

            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(pushButtonArray[i]);
            effect->setColor(QColor(255, 37, 37)); // 黑色加深
            effect->setStrength(0.8);
            pushButtonArray[i]->setGraphicsEffect(effect); // 应用加深效果
        }
    }
}
