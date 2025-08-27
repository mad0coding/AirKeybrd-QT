#ifndef TOOLS_H
#define TOOLS_H

#include <Windows.h>

//#include "widget.h"
#include <QCoreApplication>
#include <QTime>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QWidget>
#include <QPixmap>
#include <QDesktopWidget>


#define kv_A            4
#define kv_1            30
#define kv_0            39
#define kv_F1           58
#define kv_NUM1         89
#define kv_NUM0         98

#define kv_return       40
#define kv_esc          41
#define kv_backspace    42
#define kv_tab          43
#define kv_space        44
#define kv_delete       76
#define kv_right        79
#define kv_left         80

#define kv_report       234
#define kv_loop         235
#define kv_move         236
#define kv_press		237
#define kv_vol_next     238
#define kv_vol_prev     239
#define kv_wheel_up     240
#define kv_wheel_down   241
#define kv_vol_up       242
#define kv_vol_down     243
#define kv_vol_mute     244
#define kv_vol_stop     245
#define kv_mouse_l      246
#define kv_mouse_m      247
#define kv_mouse_r      248
#define kv_ctrl         249
#define kv_shift        250
#define kv_alt          251
#define kv_win          252
#define kv_shortcut     253
#define kv_delay		254

#define LIMIT(x,min,max)	(((x)<=(min))?(min):(((x)>=(max))?(max):(x)))//上下限
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define colorAngle 600//色环的1/6

void mySleep(int ms);//毫秒延时

uint8_t key_to_USB(int key, int Vkey);//QT键值转USB键值
QString USB_to_str(uint8_t key, bool shift);//USB键值转按键名
WORD USB_to_wVK(uint8_t usbKV); // USB键值转Windows虚拟键盘键值

uint8_t key_to_report(uint8_t *report, uint8_t *keyArray); // 按键数值转键盘报文

void SendInputKey(uint8_t usbKV, bool down); // 按USB键值发送虚拟按键

uint16_t endianConvert16(uint16_t num);//16位大小端转换


#endif // TOOLS_H




