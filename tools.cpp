#include "tools.h"


void mySleep(int ms)//毫秒延时
{
    QTime dieTime = QTime::currentTime().addMSecs(ms);
    while( QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


uint8_t key_to_USB(int key, int Vkey)//QT键值转USB键值
{
    if(Vkey >= 65 && Vkey <= 90) return Vkey - 61;//A~Z(4~29)
    
    if(Vkey >= 49 && Vkey <= 57) return Vkey - 19;//1~9(30~38)
    if(Vkey == 48) return 39;//0
    
    if(key == Qt::Key_Return) return 40;//Return
    if(key == Qt::Key_Escape) return 41;//Escape
    if(key == Qt::Key_Backspace) return 42;//Backspace
    if(key == Qt::Key_Tab) return 43;//Tab
    if(key == Qt::Key_Space) return 44;//Space
    if(Vkey == 189) return 45;//-_
    if(Vkey == 187) return 46;//=+
    if(Vkey == 219) return 47;//[{
    if(Vkey == 221) return 48;//]}
    if(Vkey == 220) return 49;//(|\)
    //if(Vkey == 192) return 50;//`~
    if(Vkey == 186) return 51;//;:
    if(Vkey == 222) return 52;//'"
    if(Vkey == 192) return 53;//`~
    if(Vkey == 188) return 54;//,<
    if(Vkey == 190) return 55;//.>
    if(Vkey == 191) return 56;//(/?)
    if(key == Qt::Key_CapsLock) return 57;//caps lock
    
    if(key >= Qt::Key_F1 && key <= Qt::Key_F12) return key - Qt::Key_F1 + 58;//F1~F12(58~69)
    
    if(key == Qt::Key_Print) return 70;//截图
    if(key == Qt::Key_ScrollLock) return 71;//滚动锁定
    if(key == Qt::Key_Pause) return 72;//暂停键
    if(key == Qt::Key_Insert) return 73;//Insert
    if(key == Qt::Key_Home) return 74;//Home
    if(key == Qt::Key_PageUp) return 75;//上页
    if(key == Qt::Key_Delete) return 76;//Delete
    if(key == Qt::Key_End) return 77;//End
    if(key == Qt::Key_PageDown) return 78;//下页
    if(key == Qt::Key_Right) return 79;//→
    if(key == Qt::Key_Left) return 80;//←
    if(key == Qt::Key_Down) return 81;//↓
    if(key == Qt::Key_Up) return 82;//↑
    if(key == Qt::Key_NumLock) return 83;//NumLock
    if(Vkey == 111) return 84;//(/)
    if(Vkey == 106) return 85;//(*)
    if(Vkey == 109) return 86;//-
    if(Vkey == 107) return 87;//+
    if(key == Qt::Key_Enter) return 88;//Enter
    
    if(Vkey >= 97 && Vkey <= 105) return Vkey - 8;//NUM1~NUM9(89~97)
    if(Vkey == 96) return 98;//NUM0
    
    if(Vkey == 110) return 99;//.
    
    if(key == Qt::Key_Control) return 249;//ctrl
    if(key == Qt::Key_Shift) return 250;//shift
    if(key == Qt::Key_Alt) return 251;//alt
    if(key == 16777250) return 252;//win
    
    return 0;
}

QString USB_to_str(uint8_t key, bool shift)//USB键值转按键名
{
    switch (key) {
        case 0:return "[None]";
        case 4:return "[A]";
        case 5:return "[B]";
        case 6:return "[C]";
        case 7:return "[D]";
        case 8:return "[E]";
        case 9:return "[F]";
        case 10:return "[G]";
        case 11:return "[H]";
        case 12:return "[I]";
        case 13:return "[J]";
        case 14:return "[K]";
        case 15:return "[L]";
        case 16:return "[M]";
        case 17:return "[N]";
        case 18:return "[O]";
        case 19:return "[P]";
        case 20:return "[Q]";
        case 21:return "[R]";
        case 22:return "[S]";
        case 23:return "[T]";
        case 24:return "[U]";
        case 25:return "[V]";
        case 26:return "[W]";
        case 27:return "[X]";
        case 28:return "[Y]";
        case 29:return "[Z]";
        case 30:return shift ? "[!]" : "[1]";
        case 31:return shift ? "[@]" : "[2]";
        case 32:return shift ? "[#]" : "[3]";
        case 33:return shift ? "[$]" : "[4]";
        case 34:return shift ? "[%]" : "[5]";
        case 35:return shift ? "[^]" : "[6]";
        case 36:return shift ? "[&]" : "[7]";
        case 37:return shift ? "[*]" : "[8]";
        case 38:return shift ? "[(]" : "[9]";
        case 39:return shift ? "[)]" : "[0]";
        case 40:return "[Return]";
        case 41:return "[Escape]";
        case 42:return "[Backspace]";
        case 43:return "[Tab]";
        case 44:return "[Space]";
        case 45:return shift ? "[_]" : "[-]";
        case 46:return shift ? "[+]" : "[=]";
        case 47:return shift ? "[{]" : "[ [ ]";
        case 48:return shift ? "[}]" : "[ ] ]";
        case 49:return shift ? "[|]" : "[\\]";
        case 50:return shift ? "[**|]" : "[**\\]";
        case 51:return shift ? "[:]" : "[;]";
        case 52:return shift ? "[\"]" : "[']";
        case 53:return shift ? "[~]" : "[`]";
        case 54:return shift ? "[<]" : "[,]";
        case 55:return shift ? "[>]" : "[.]";
        case 56:return shift ? "[?]" : "[/]";
        case 57:return "[CapsLock]";
        case 58:return "[F1]";
        case 59:return "[F2]";
        case 60:return "[F3]";
        case 61:return "[F4]";
        case 62:return "[F5]";
        case 63:return "[F6]";
        case 64:return "[F7]";
        case 65:return "[F8]";
        case 66:return "[F9]";
        case 67:return "[F10]";
        case 68:return "[F11]";
        case 69:return "[F12]";
        case 70:return "[PrintScreen]";
        case 71:return "[ScrollLock]";
        case 72:return "[Pause]";
        case 73:return "[Insert]";
        case 74:return "[Home]";
        case 75:return "[PageUp]";
        case 76:return "[Delete]";
        case 77:return "[End]";
        case 78:return "[PageDown]";
        case 79:return "[Right]";
        case 80:return "[Left]";
        case 81:return "[Down]";
        case 82:return "[Up]";
        case 83:return "[NumLock]";
        case 84:return "[NUM /]";
        case 85:return "[NUM *]";
        case 86:return "[NUM -]";
        case 87:return "[NUM +]";
        case 88:return "[Enter]";
        case 89:return "[NUM 1]";
        case 90:return "[NUM 2]";
        case 91:return "[NUM 3]";
        case 92:return "[NUM 4]";
        case 93:return "[NUM 5]";
        case 94:return "[NUM 6]";
        case 95:return "[NUM 7]";
        case 96:return "[NUM 8]";
        case 97:return "[NUM 9]";
        case 98:return "[NUM 0]";
        case 99:return "[NUM .]";
        case 100:return "[Non-US\and|]";
        case 101:return "[Application]";
        case 102:return "[Power]";
        case 103:return "[NUM =]";
        case kv_wheel_up:return "[滚轮向上]";
        case kv_wheel_down:return "[滚轮向下]";
        case kv_vol_up:return "[音量加]";
        case kv_vol_down:return "[音量减]";
        case kv_vol_mute:return "[静音]";
        case kv_vol_stop:return "[播放/暂停]";
        case kv_vol_next:return "[下一个]";
        case kv_vol_prev:return "[上一个]";
        case kv_mouse_l:return "[鼠标左键]";
        case kv_mouse_m:return "[鼠标中键]";
        case kv_mouse_r:return "[鼠标右键]";
        case kv_ctrl:return "[Ctrl]";
        case kv_shift:return "[Shift]";
        case kv_alt:return "[Alt]";
        case kv_win:return "[Win]";
        default:return "[]";
    }
}

WORD USB_to_wVK(uint8_t usbKV) // USB键值转Windows虚拟键盘键值
{
    if(usbKV >= 4 && usbKV <= 29) return usbKV - 4 + 'A'; // A~Z
    else if(usbKV >= 30 && usbKV <= 38) return usbKV - 30 + '1'; // 1~9
    else if(usbKV == 39) return '0'; // 0
    else if(usbKV >= 58 && usbKV <= 69) return usbKV - 58 + VK_F1; // F1~F12
    else if(usbKV >= 89 && usbKV <= 97) return usbKV - 89 + VK_NUMPAD1; // NUM1~NUM9
    else if(usbKV == 98) return VK_NUMPAD0; // NUM0
    switch (usbKV) {
        case 0:return 0;
        case 40:return VK_RETURN; // Return
        case 41:return VK_ESCAPE;
        case 42:return VK_BACK;
        case 43:return VK_TAB;
        case 44:return VK_SPACE;
        case 45:return VK_OEM_MINUS;
        case 46:return VK_OEM_PLUS;
        case 47:return VK_OEM_4;
        case 48:return VK_OEM_6;
        case 49:return VK_OEM_5;
//        case 50:return shift ? "[**|]" : "[**\\]";
        case 51:return VK_OEM_1;
        case 52:return VK_OEM_7;
        case 53:return VK_OEM_3;
        case 54:return VK_OEM_COMMA;
        case 55:return VK_OEM_PERIOD;
        case 56:return VK_OEM_2;
        case 57:return VK_CAPITAL;
        case 70:return VK_SNAPSHOT;
        case 71:return VK_SCROLL;
        case 72:return VK_PAUSE;
        case 73:return VK_INSERT;
        case 74:return VK_HOME;
        case 75:return VK_PRIOR;
        case 76:return VK_DELETE;
        case 77:return VK_END;
        case 78:return VK_NEXT;
        case 79:return VK_RIGHT;
        case 80:return VK_LEFT;
        case 81:return VK_DOWN;
        case 82:return VK_UP;
        case 83:return VK_NUMLOCK;
        case 84:return VK_DIVIDE;
        case 85:return VK_MULTIPLY;
        case 86:return VK_SUBTRACT;
        case 87:return VK_ADD;
        case 88:return VK_RETURN; // Enter 临时代码
        case 99:return VK_DECIMAL;
//        case 100:return "[Non-US\and|]";
//        case 101:return "[Application]";
//        case 102:return "[Power]";
//        case 103:return "[NUM =]";
//        case kv_wheel_up:return "[滚轮向上]";
//        case kv_wheel_down:return "[滚轮向下]";
        case kv_vol_up:return VK_VOLUME_UP;
        case kv_vol_down:return VK_VOLUME_DOWN;
        case kv_vol_mute:return VK_VOLUME_MUTE;
        case kv_vol_stop:return VK_MEDIA_STOP;
        case kv_vol_next:return VK_MEDIA_NEXT_TRACK;
        case kv_vol_prev:return VK_MEDIA_PREV_TRACK;
        case kv_mouse_l:return VK_LBUTTON;
        case kv_mouse_m:return VK_MBUTTON;
        case kv_mouse_r:return VK_RBUTTON;
        case kv_ctrl:return VK_LCONTROL;
        case kv_shift:return VK_LSHIFT;
        case kv_alt:return VK_LMENU;
        case kv_win:return VK_LWIN;
        default:return 0;
    }
}

uint8_t key_to_report(uint8_t *report, uint8_t *keyArray) // 按键数值转键盘报文
{
    memset(report, 0, 8); // 清空报文
    
    if(keyArray[249]) report[0] |= 0x01; // Ctrl
    if(keyArray[250]) report[0] |= 0x02; // Shift
    if(keyArray[251]) report[0] |= 0x04; // Alt
    if(keyArray[252]) report[0] |= 0x08; // Win
    
    int j = 2;
    for(int i = 1; i < 249; i++){
        if(keyArray[i]){ // 此按键按下
            report[j++] = i; // 填入键值
        }
        if(j >= 8) return 1; // 溢出
    }
    return 0;
}

void SendInputKey(uint8_t usbKV, bool down) // 按USB键值发送虚拟按键
{
    WORD vk = USB_to_wVK(usbKV);
    INPUT input[1] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vk;
    input[0].ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP; // 0 表示按下
    
    SendInput(sizeof(input) / sizeof(INPUT), input, sizeof(INPUT)); // 向系统发送
}

uint16_t endianConvert16(uint16_t num){//16位大小端转换
    return (num << 8) | (num >> 8);//交换两个字节
}




























