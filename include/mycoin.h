//
// Created by PC on 24-12-10.
//

#ifndef MYCOIN_H
#define MYCOIN_H

#include <QPushButton>
#include <QTimer>


class MyCoin : public QPushButton {
    Q_OBJECT

public:
    // explicit MyCoin(QWidget *parent = nullptr);

    //参数代表 传入的金币路径 还是银币路径
    MyCoin(QString btnImg);

    //金币的属性
    int posX;
    int posY;
    bool flag; //正反标识

    //改变标志的方法
    void changeFlag();

    QTimer *timer1; //正面翻反面的定时器
    QTimer *timer2; //反面翻正面的定时器
    int min = 1;
    int max = 8;

    //执行动画 标志
    bool isAnimation = false;

    //重写 按下
    void mousePressEvent(QMouseEvent *event) override;

    //是否胜利
    bool isWin = false;

signals:

public slots:
};


#endif //MYCOIN_H
