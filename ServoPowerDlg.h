#ifndef SERVOPOWERDLG_H
#define SERVOPOWERDLG_H

#include <QDialog>
#include "CServoPower.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServoPowerDlg; }
QT_END_NAMESPACE

class ServoPowerDlg : public QDialog
{
    Q_OBJECT

public:
    ServoPowerDlg(QWidget *parent = nullptr);
    ~ServoPowerDlg();


protected slots:
    void sltSetVoltage();
    void sltSetCurrent();
    void sltRefreshVoltage();
    void sltRefreshCurrent();
    void sltRefreshPower();
    void sltPowerOn();
    void sltPowerOff();
    void sltRefreshRatedVoltage();
    void sltRefreshRatedCurrent();
    void sltPowerMsg(QString msg);
	void sltRefreshStatus();
	void sltShowPowerStatus(int);
	void sltShowPowerCurrent(int);
	void sltShowPowerVoltage(int);
	void sltShowPowerRatedCurrent(int);
	void sltShowPowerRatedVoltage(int);
    void sltShowPowerPower(unsigned short thousand, unsigned short walt, unsigned short mwalt);
	void sltSliderCurrentChanged(int);
	void sltSliderVoltageChanged(int);

protected:
	void timerEvent(QTimerEvent *event);

private:
    Ui::ServoPowerDlg *ui;

    CServoPower servoPower;
	int myTimerId;

	bool needSwitchOn;
	bool needSwitchOff;
	bool needSetCurrent;
	bool needSetVoltage;
};
#endif // SERVOPOWERDLG_H
