#include "widget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget mywidget;
    mywidget.setWindowTitle("Keyboard-COM V1.3"); // 设置窗口名称
    mywidget.show();
    mywidget.setFocusPolicy(Qt::StrongFocus);
    
    a.exec();
    
    return 0;
}
