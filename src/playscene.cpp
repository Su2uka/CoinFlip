//
// Created by PC on 24-12-9.
//


#include "playscene.h"
#include <QDebug>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QSoundEffect>
#include "mypushbutton.h"
#include "mycoin.h"
#include "dataconfig.h"
#include "recorder.h"

PlayScene::PlayScene(int levelNum) {
    //记录提示对话框
    QMessageBox *msgBox = new QMessageBox(this);

    //记录器背景
    QLabel *bg = new QLabel(this);
    bg->setGeometry(195, 25, 118, 50);
    bg->setStyleSheet("background-color: rgba(255, 255, 255, 150); border-radius: 5px;");

    //记录器
    Recorder *recorder = new Recorder();
    recorder->timer->start();

    //记录时间
    QLabel *timeLabel = new QLabel(this);
    QFont recordFont;
    recordFont.setFamily("汉仪文黑-85W");
    recordFont.setPointSize(13);
    timeLabel->setFont(recordFont);
    timeLabel->setGeometry(200, 25, 140, 30);
    timeLabel->setText(QString("Time:%1").arg(recorder->num2time(recorder->seconds)));

    connect(recorder->timer, &QTimer::timeout, [=]() {
        recorder->seconds++;
        timeLabel->setText(QString("Time:%1").arg(recorder->num2time(recorder->seconds)));
    });

    //记录步数
    QLabel *stepsLabel = new QLabel(this);
    stepsLabel->setFont(recordFont);
    stepsLabel->setGeometry(200, 46, 140, 30);
    stepsLabel->setText("Steps:0");

    connect(recorder, &Recorder::stepsUp, [=]() {
        recorder->steps++;
        stepsLabel->setText(QString("Steps:%1").arg(recorder->steps));
    });


    QString str = QString("进入了第%1关").arg(levelNum);
    qDebug() << str;

    this->levelIndex = levelNum;

    //初始化游戏场景
    //设置固定大小
    this->setFixedSize(320, 588);
    //设置图片
    this->setWindowIcon(QPixmap(":/Coin0001.png"));
    //设置标题
    this->setWindowTitle(QString("第 %1 关").arg(levelNum));

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

    //添加音效资源
    //翻金币音效
    QSoundEffect *flipSound = new QSoundEffect(this);
    flipSound->setSource(QUrl::fromLocalFile(":/ConFlipSound.wav"));
    //胜利音效
    QSoundEffect *winSound = new QSoundEffect(this);
    winSound->setSource(QUrl::fromLocalFile(":/LevelWinSound.wav"));
    //返回按钮音效
    QSoundEffect *backSound = new QSoundEffect(this);
    backSound->setSource(QUrl::fromLocalFile(":/BackButtonSound.wav"));

    //返回按钮
    MyPushButton *backBtn = new MyPushButton(":/BackButton.png", ":/BackButtonSelected.png");
    backBtn->setParent(this);
    backBtn->move(this->width() - backBtn->width() - 10, this->height() - backBtn->height() - 10);

    //点击返回
    connect(backBtn, &MyPushButton::clicked, [=]() {
        qDebug() << "关卡中点击了返回按钮";
        //停止计时器
        recorder->timer->stop();

        if (msgBox->text() != nullptr) {
            msgBox->exec();
        }

        //告诉主场景 我返回了 主场景监听返回
        backSound->play();
        QTimer::singleShot(200, this, [=]() {
            emit this->chooseSceneBack();
        });
    });

    //显示当前关卡数
    QLabel *label = new QLabel();
    label->setParent(this);
    QFont font;
    font.setFamily("华文新魏");
    font.setPointSize(20);
    // font.setBold(true);
    label->setFont(font);
    QString s = QString("Level:%1").arg(this->levelIndex);
    label->setText(s);
    label->setGeometry(30, this->height() - 50, 120, 50);


    dataConfig config;
    //初始化每个关卡的二维数组
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            this->gameArray[i][j] = config.mData[this->levelIndex][i][j];
        }
    }

    //胜利图片显示
    QLabel *winLabel = new QLabel;
    QPixmap tmpPix;
    tmpPix.load(":/LevelCompletedDialogBg.png");
    winLabel->setPixmap(tmpPix);
    winLabel->setParent(this);
    winLabel->move((this->width() - tmpPix.width()) * 0.5, -4 * tmpPix.height());


    //显示金币背景图案
    //创建金币的背景图片
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            //绘制背景图片
            QLabel *label = new QLabel;
            label->setGeometry(0, 0, 50, 50);
            label->setPixmap(QPixmap(":/BoardNode(1).png"));
            label->setParent(this);
            label->move(57 + i * 50, 200 + j * 50);


            //创建金币
            QString coinStr;
            if (this->gameArray[i][j] == 1) {
                //显示金币
                coinStr = ":/Coin0001.png";
            } else {
                coinStr = ":/Coin0008.png";
            }

            MyCoin *coin = new MyCoin(coinStr);
            coin->setParent(this);
            coin->move(59 + i * 50, 202 + j * 50);

            //给金币赋值
            coin->posX = i;
            coin->posY = j;
            coin->flag = this->gameArray[i][j]; //1正面   0反面

            //将金币放入到金币的二维数组里
            coinBtn[i][j] = coin;

            //点击金币 进行翻转
            connect(coin, &MyCoin::clicked, [=]() {
                //步数++
                emit recorder->stepsUp();

                flipSound->play();
                //点击按钮 将所有的按钮都先禁用
                for (int i = 0; i < 4; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        this->coinBtn[i][j]->isWin = true;
                    }
                }

                coin->changeFlag();
                this->gameArray[i][j] = this->gameArray[i][j] == 0 ? 1 : 0;

                //翻转周围硬币
                //延时
                QTimer::singleShot(300, this, [=]() {
                    int dx[4] = {1, -1, 0, 0};
                    int dy[4] = {0, 0, 1, -1};

                    for (int a = 0; a < 4; ++a) {
                        int newX = coin->posX + dx[a];
                        int newY = coin->posY + dy[a];

                        // 检查新位置是否在有效范围内
                        if (newX >= 0 && newX <= 3 && newY >= 0 && newY <= 3) {
                            // 翻转对应的按钮状态和数组值
                            coinBtn[newX][newY]->changeFlag();
                            this->gameArray[newX][newY] = this->gameArray[newX][newY] == 0 ? 1 : 0;
                        }
                    }

                    //翻完按钮 将所有的按钮都解开禁用
                    for (int i = 0; i < 4; ++i) {
                        for (int j = 0; j < 4; ++j) {
                            this->coinBtn[i][j]->isWin = false;
                        }
                    }


                    //判断是否胜利
                    this->isWin = true;
                    for (int m = 0; m < 4; ++m) {
                        for (int n = 0; n < 4; ++n) {
                            if (coinBtn[m][n]->flag == false) {
                                this->isWin = false;
                                break;
                            }
                        }
                    }
                    if (this->isWin) {
                        //胜利
                        qDebug() << "胜利了";

                        //停止计时器
                        recorder->timer->stop();

                        //判断数据是否破纪录
                        //本关现有记录
                        int timeRecord = recorder->timeMap[levelIndex];
                        int stepsRecord = recorder->stepsMap[levelIndex];
                        qDebug() << QString("第%1关 以前的记录 时间：%2  步数：%3").arg(levelIndex).arg(timeRecord).arg(stepsRecord);

                        bool timeFlag = false;
                        bool stepsFlag = false; //是否破纪录的flag
                        //与此次成绩对比
                        if (recorder->seconds < timeRecord) {
                            //时间破纪录了
                            timeFlag = true;
                            qDebug() << "时间破纪录了";
                            recorder->timeMap[levelIndex] = recorder->seconds;
                            //写入文件
                            recorder->saveData("t.dat", recorder->timeMap);
                        } else {
                            qDebug() << "未打破时间纪录";
                        }
                        if (recorder->steps < stepsRecord) {
                            //步数破纪录了
                            stepsFlag = true;
                            qDebug() << "步数破纪录了";
                            recorder->stepsMap[levelIndex] = recorder->steps;
                            //写入文件
                            recorder->saveData("s.dat", recorder->stepsMap);
                        } else {
                            qDebug() << "未打破步数纪录";
                        }

                        //提示记录信息
                        msgBox->setWindowTitle("记录信息");
                        QString info;
                        if (timeFlag && stepsFlag) {
                            info = QString("新纪录（时间:%1 步数:%2）").arg(recorder->num2time(recorder->seconds)).arg(
                                recorder->steps);
                        } else if (timeFlag) {
                            info = QString("新纪录（时间:%1） 最佳记录（步数:%2）").arg(recorder->num2time(recorder->seconds)).arg(
                                recorder->stepsMap[levelIndex]);
                        } else if (stepsFlag) {
                            info = QString("最佳记录（时间:%1） 新纪录（步数:%2）").arg(
                                recorder->num2time(recorder->timeMap[levelIndex])).arg(recorder->steps);
                        } else {
                            info = QString("最佳记录（时间:%1 步数:%2）").arg(recorder->num2time(recorder->timeMap[levelIndex])).
                                    arg(recorder->stepsMap[levelIndex]);
                        }
                        msgBox->setText(info);


                        //将所有按钮的胜利标志改为true,如果再次点击，不做响应
                        for (int m = 0; m < 4; ++m) {
                            for (int n = 0; n < 4; ++n) {
                                coinBtn[m][n]->isWin = true;
                            }
                        }
                        QTimer::singleShot(500, this, [=]() {
                            //胜利音效
                            winSound->play();

                            //将胜利图片砸下来
                            QPropertyAnimation *animation = new QPropertyAnimation(winLabel, "geometry");
                            //设置时间间隔
                            animation->setDuration(1000);
                            //设置开始位置
                            animation->setStartValue(QRect(winLabel->x(), winLabel->y(), winLabel->width(),
                                                           winLabel->height()));
                            //设置结束位置
                            animation->setEndValue(QRect(winLabel->x(), winLabel->y() + 252, winLabel->width(),
                                                         winLabel->height()));
                            //设置缓和曲线
                            animation->setEasingCurve(QEasingCurve::OutBounce);
                            //执行动画
                            animation->start();
                        });
                    }
                });
            });
        }
    }
}

void PlayScene::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap pix;
    pix.load(":/PlayLevelSceneBg.png");
    painter.drawPixmap(0, 0, this->width(), this->height(), pix);

    pix.load(":/Title.png");
    pix = pix.scaled(pix.width() * 0.5, pix.height() * 0.5);
    painter.drawPixmap(10, 30, pix);
}
