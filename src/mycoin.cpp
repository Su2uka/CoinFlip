//
// Created by PC on 24-12-10.
//


#include "mycoin.h"
#include <QDebug>

MyCoin::MyCoin(QString btnImg) {
    QPixmap pix;
    bool ret = pix.load(btnImg);
    if (!ret) {
        QString str = QString("图片 %1 加载失败").arg(btnImg);
        qDebug() << str;
        return;
    }

    this->setFixedSize(pix.height(), pix.width());
    this->setStyleSheet("QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 0px;"
        "}"
        "QPushButton:pressed {"
        "    background-color: transparent;"
        "}");
    this->setIcon(pix);
    this->setIconSize(QSize(pix.width(), pix.height()));


    //初始化定时器对象
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);

    //监听正面翻反面
    connect(timer1, &QTimer::timeout, [=]() {
        QPixmap pix;
        QString str = QString(":/Coin000%1").arg(this->min++);
        pix.load(str);

        this->setFixedSize(pix.height(), pix.width());
        this->setStyleSheet("QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    padding: 0px;"
            "}"
            "QPushButton:pressed {"
            "    background-color: transparent;"
            "}");
        this->setIcon(pix);
        this->setIconSize(QSize(pix.width(), pix.height()));
        //判断 如何翻完了 将min重置为1
        if (this->min > this->max) {
            this->min = 1;
            isAnimation = false;
            timer1->stop();
        }
    });

    //监听反面翻正面
    connect(timer2, &QTimer::timeout, [=]() {
        QPixmap pix;
        QString str = QString(":/Coin000%1").arg(this->max--);
        pix.load(str);
        this->setFixedSize(pix.height(), pix.width());
        this->setStyleSheet("QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    padding: 0px;"
            "}"
            "QPushButton:pressed {"
            "    background-color: transparent;"
            "}");
        this->setIcon(pix);
        this->setIconSize(QSize(pix.width(), pix.height()));
        //判断 如何翻完了 将max重置为8
        if (this->max < this->min) {
            this->max = 8;
            isAnimation = false;
            timer2->stop();
        }
    });
}


void MyCoin::changeFlag() {
    //如果是正面 翻转反面
    if (this->flag) {
        //开始正面翻反面的定时器
        timer1->start(30);
        isAnimation = true; //开始执行动画
        this->flag = false;
    } else {
        timer2->start(30);
        isAnimation = true; //开始执行动画
        this->flag = true;
    }
}

void MyCoin::mousePressEvent(QMouseEvent *event) {
    if (this->isAnimation || this->isWin) return;

    QPushButton::mousePressEvent(event);
}
