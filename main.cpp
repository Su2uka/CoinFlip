#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName(QStringLiteral("Su2uka"));
    QApplication::setApplicationName(QStringLiteral("CoinFlip"));
    QApplication::setApplicationDisplayName(QStringLiteral("Mystic CoinFlip"));
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    app.setWindowIcon(QIcon(QStringLiteral(":/images/Coin0001.png")));

    MainWindow window;
    window.show();

    return app.exec();
}
