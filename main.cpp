#include <QApplication>
#include "mainscene.h"
#include <QDebug>
#include <iostream>
#include <QLoggingCategory>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // 检查是否包含特定的警告消息
    if (msg.contains("libpng warning: iCCP: known incorrect sRGB profile")) {
        // 忽略该警告
        return;
    }
    // 其他消息正常输出
    QTextStream qout(stdout);
    qout.setEncoding (QStringConverter::System);
    qout << msg << Qt::endl;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 设置自定义日志处理器
    qInstallMessageHandler(customMessageHandler);

    MainScene m;
    m.show();

    // //测试
    // dataConfig config;
    // for (int i = 0; i < 4; ++i) {
    //     for (int j = 0; j < 4; ++j) {
    //         // qDebug() << config.mData[1][i][j];
    //         std::cout << config.mData[1][i][j] << " ";
    //     }
    //     // qDebug() << " ";
    //     std::cout << std::endl;
    // }


    return QApplication::exec();
}
