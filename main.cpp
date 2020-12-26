#include "ServoPowerDlg.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServoPowerDlg w;
    w.show();
    return a.exec();
}
