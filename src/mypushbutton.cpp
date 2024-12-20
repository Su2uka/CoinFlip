#include "mypushbutton.h"
#include <QDebug>
#include <QPropertyAnimation>

MyPushButton::MyPushButton(QString normalImg, QString pressImg) {
    this->setCursor(Qt::PointingHandCursor);

    this->normalImgPath = normalImg;
    this->pressImgPath = pressImg;

    QPixmap pix;
    bool ret = pix.load(normalImg);
    if (!ret) {
        qDebug() << "图片加载失败";
        return;
    }

    //设置图片固定大小
    this->setFixedSize(pix.width(), pix.height());

    //设置不规则图片样式
    this->setStyleSheet("QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 0px;"
        "}"
        "QPushButton:pressed {"
        "    background-color: transparent;"
        "}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    this->setIconSize(QSize(pix.width(), pix.height()));
}

void MyPushButton::zoom1() {
    //创建动画对象
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    //设置动画时间间隔
    animation->setDuration(300);

    //起始位置
    animation->setStartValue(QRect(this->x(), this->y(), this->width(), this->height()));
    //结束位置
    animation->setEndValue(QRect(this->x(), this->y() + 10, this->width(), this->height()));

    //设置弹跳曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);

    //执行动画
    animation->start();
}

void MyPushButton::zoom2() {
    //创建动画对象
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    //设置动画时间间隔
    animation->setDuration(300);

    //起始位置
    animation->setStartValue(QRect(this->x(), this->y() + 10, this->width(), this->height()));
    //结束位置
    animation->setEndValue(QRect(this->x(), this->y(), this->width(), this->height()));

    //设置弹跳曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);

    //执行动画
    animation->start();
}

void MyPushButton::mousePressEvent(QMouseEvent *event) {
    if (this->pressImgPath != "") {
        QPixmap pix;
        bool ret = pix.load(this->pressImgPath);
        if (!ret) {
            qDebug() << "图片加载失败";
            return;
        }

        //设置图片固定大小
        this->setFixedSize(pix.width(), pix.height());

        //设置不规则图片样式
        // this->setStyleSheet("QPushButton{border:0px;}");

        this->setStyleSheet("QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    padding: 0px;"
            "}"
            "QPushButton:pressed {"
            "    background-color: transparent;"
            "}");

        //设置图标
        this->setIcon(pix);

        //设置图标大小
        this->setIconSize(QSize(pix.width(), pix.height()));
    }
    //让父类执行其他内容
    return QPushButton::mousePressEvent(event);
}

void MyPushButton::mouseReleaseEvent(QMouseEvent *event) {
    if (this->pressImgPath != "") {
        QPixmap pix;
        bool ret = pix.load(this->normalImgPath);
        if (!ret) {
            qDebug() << "图片加载失败";
            return;
        }

        //设置图片固定大小
        this->setFixedSize(pix.width(), pix.height());

        //设置不规则图片样式
        // this->setStyleSheet("QPushButton{border:0px;}");

        this->setStyleSheet("QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    padding: 0px;"
            "}"
            "QPushButton:pressed {"
            "    background-color: transparent;"
            "}");

        //设置图标
        this->setIcon(pix);

        //设置图标大小
        this->setIconSize(QSize(pix.width(), pix.height()));
    }
    //让父类执行其他内容
    return QPushButton::mouseReleaseEvent(event);
}
