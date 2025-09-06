#include "widget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/AirButton.ico")); // 显式设置应用图标
    Widget mywidget;
    mywidget.setWindowTitle("AirKeybrd V1.4"); // 设置窗口名称
    mywidget.show();
    mywidget.setFocusPolicy(Qt::StrongFocus);
    
    setbuf(stdout, NULL); // 设置printf缓冲为0 以使之立即输出
    
    a.exec();
    
    return 0;
}
