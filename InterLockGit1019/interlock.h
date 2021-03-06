#ifndef INTERLOCK_H
#define INTERLOCK_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QUdpSocket>
#include <QTimer>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>

typedef unsigned char byte;
struct SectionData
{
    byte sectionId;                             //区段编号
    byte sectionName;                           //区段名字
    byte LockStatus;                            //锁闭状态
    byte sectionStatus;                         //占用状态
    byte SectionWhiteObstacle;                  //白光带故障 01为故障 02为正常
    byte SectionRedObstacle;                    //红光带故障 01为故障 02为正常
    byte blockStatus;                           //封锁状态 01为封锁 02为正常
    byte PoorStatus;                           //分路不良 01为不良 02为正常
    byte First1;
};//区段信息封装

struct SignalData
{
    byte signalId;                              //信号机ID
    byte signalStatus;                          //信号灯颜色（0灰色禁用，1位单红日常，2为双黄，3为双黄闪，4为单黄，5为绿黄，6为单绿,7为单蓝，8为单白,9为红闪，10为单黄闪，11双黄前灯闪，12蓝闪，13白闪，14绿闪）
    byte blockStatus;                           //信号灯封锁 01为封锁 02为解锁
    byte signalType;                            //信号机类型 (1正线进站，2侧线进站，3列车出站，4调车，5调车出站)
    byte DSStatus;                              //红蓝主灯丝断 01为灯丝断 02为正常
    byte RedAllDSStatus;                        //红蓝灯丝全断 01为灯丝断 02为正常
    byte GreenDSStatus;                         //绿白黄主灯丝断 01为灯丝断 02为正常
    byte GreenAllDSStatus;                      //绿白黄灯丝全断 01为灯丝断 02为正常
    byte signalLockStatus;                      //信号机开放和锁闭 01为关闭 02为开放
};//信号机信息封装

struct SwitchData
{
    byte switchID;                              //道岔id
    byte switchStates;                          //道岔定反位启用状态
    byte switchLock;                            //道岔锁闭状态
    byte switchPos;                             //道岔，0为正位，1为反位
    byte switchName;                            //道岔名称
    byte switchNameID;                          //道岔名称id
    byte blockStatus;                           //封锁状态 01为封锁 02为解锁
    byte SwitchLoss;                            //道岔失表 01为失表 02位正常
    byte switchOccupy;                          //占用状态 01为占用 02为空闲
};//道岔信息封装

struct LineRuleData
{
    byte lineruleID;
    QString beginSignalName;
    QString endSignalName;
    byte Type;
    byte First;
    QList<SectionData> sections;
    QList<SwitchData> switchs;
};//进路规则封装



enum line
{
    IN = 0,                               //进站
    OUT = 1,                              //出站
    INOUT = 2,                            //引导
    TONG = 3,                             //通路
    DIAO = 4                              //调车
};

enum signalColor
{
    Dark,
    Red,
    Double_yellow,
    Double_yellow_flicker,
    Yellow,
    Yellow_flicker,
    Green_yellow,
    Green,
    Red_white,
    White,
    Blue
};
namespace Ui {
class InterLock;
}

class InterLock : public QWidget
{
    Q_OBJECT

public:
    explicit InterLock(QWidget *parent = 0);
    ~InterLock();
public:

   // byte X_Direction,XF_Direction,XD_Direction,S_Direction,SF_Direction;
    byte beginSignalID,endSignalID;

    byte direction;

    void SectionDataCache();
    void UnlockState(byte beginSignalID,byte endSignalID);//实现三点检查逐条解锁
    void SignalDataCache();
    void SwitchDataCache();
    void RuleDataCache();
    QByteArray SectionEncapsalutation();
    QByteArray SignalEncapsalutation();
    QByteArray SwitchEncapsalutation();
    QByteArray RuleEncapsalutation();
    QByteArray switchesStrextract(QString switchesStr);

    //功能方法
    void InLine(byte beginSignalID,byte endSignalID,int type);//进路设置:type=1正常进路;type=2引导进路
    void RemoveRoute(byte beginSignalID,int type); //取消进路:type=1正常进路取消;type=2引导进路取消【总人解】
    void YinDaoJL(byte Direction);//引导进路
    void YinDaoZS(byte Dircetion);//引导总锁
    void ZongDingFanDanJieSuo(byte Snum,byte Sstatus);//道岔总定反，单解锁
    void FengSuo(byte Id,byte Type);//封锁按钮
    void SwitchLoss(byte SwitchName);//道岔失表
    void SwitchNoLoss(byte SwitchName);//道岔取消失表
    void XinHaoCK(byte SignalID);//信号重开
    void DSDS(byte SignalID, byte Status);//灯丝断丝
    void DSFY(byte Status);//灯丝复原

    void ShangDian();//上电解锁
    void SetupRoute(byte beginSignalID,byte endSignalID);
    void RenGong(byte beginSignalID);//总人解    
    void FenLu(int Snum);//分路不良
    void QuGJ(byte Snum);//区解锁
    void BiSe(byte Direction);//闭塞操作
    void DiaoChe();//调车进路
    void ZhanYong(byte Snum);//占用
    void GuZhang(byte Nameid,byte Sstatus);//设置故障
    void SetupSignal(byte SignalID,byte signalStatus);
   // void InterEncapsalutation(byte XDirection,byte XFDirection,byte XDDirection,byte SDirection,byte SFDirection);
    void InterEncapsalutation(byte beginSignalID,byte endSignalID);

    //QString SqlCunChu(byte beginSignalID,byte endSignalID);

    static int Chance;
    static int First;
    static QList<int> MessageList;
public slots:
    void readPendingDatagrams();
    void TimerTicked();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::InterLock *ui;
    QSystemTrayIcon *mSysTrayIcon;
    QTimer * timer;
    QUdpSocket *udpSocket;
    QUdpSocket *udpSocket_receive;
    quint8 capNr[4];
    QMap<QString,SectionData> SectionsDataMap;
    QMap<int,SignalData> SignalsDataMap;
    QMap<QString,QString> LockRouteMap;//锁闭线路
    QMap<QString,QString> LockSwitchMap;//锁闭道岔
    QMap<QString,QList<QString>> RuleMap;//进路记录

    QMap<int,SwitchData> SwitchDataMap;//道岔信息封装
    QMap<QString,LineRuleData> RuleDataMap;//进路规则封装
    QMap<int,LineRuleData> NewRuleDataMap;//进路规则缓存

    int m_time;//timeEvent对应的time;
    int sectionNum;
    byte sectState;
    byte stateNum;
};

#endif // INTERLOCK_H
