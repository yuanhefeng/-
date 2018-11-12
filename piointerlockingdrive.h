#ifndef PIOINTERLOCKINGDRIVE_H
#define PIOINTERLOCKINGDRIVE_H

#include <QObject>
#include "./Inc/bdaqctrl.h"
#include <Windows.h>

using namespace Automation::BDaq;
class PioInterlockingDrive : public QObject
{
    Q_OBJECT
public:
    explicit PioInterlockingDrive(QObject *parent = 0);
public:
    void DOConfigureDevice();
    void GetDOPortState();
    void SetDOPortSate(int portNum,byte data);

    void DIConfigureDevice();
    void CheckError(ErrorCode errorCode);
public:
    quint8 *portStates;
public slots:
    quint8* ReadDiPortState();
private:
    InstantDoCtrl* instantDoCtrl;
    InstantDiCtrl* instantDICtrl;
    int portDoCount;
    int portDiCount;
    byte* portDiData;
    //QTimer *timer;
};

#endif // PIOINTERLOCKINGDRIVE_H
