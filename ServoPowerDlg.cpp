#include "ServoPowerDlg.h"
#include "ui_ServoPowerDlg.h"
#include <QTimer>

ServoPowerDlg::ServoPowerDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServoPowerDlg)
    , servoPower(this)
	, myTimerId(-1)
	, needSwitchOn(false)
	, needSwitchOff(false)
	, needSetCurrent(false)
	, needSetVoltage(false)
{
    ui->setupUi(this);
    connect(&servoPower,SIGNAL(error(QString)),this,SLOT(sltPowerMsg(QString)));
    connect(&servoPower,SIGNAL(powerMsg(QString)),this,SLOT(sltPowerMsg(QString)));
	connect(&servoPower, SIGNAL(powerState(int)), this, SLOT(sltShowPowerStatus(int)));

	connect(&servoPower, SIGNAL(powerCurrent(int)), this, SLOT(sltShowPowerCurrent(int)));
	connect(&servoPower, SIGNAL(powerVoltage(int)), this, SLOT(sltShowPowerVoltage(int)));

	connect(&servoPower, SIGNAL(powerRatedCurrent(int)), this, SLOT(sltShowPowerRatedCurrent(int)));
	connect(&servoPower, SIGNAL(powerRatedVoltage(int)), this, SLOT(sltShowPowerRatedVoltage(int)));

    connect(&servoPower, SIGNAL(powerPower(unsigned short , unsigned short , unsigned short )),
            this, SLOT(sltShowPowerPower(unsigned short , unsigned short , unsigned short )));
	servoPower.start();

	myTimerId = startTimer(250);
	/*QTimer::singleShot(100, this, [=] {
		sltRefreshStatus();
	});*/

}

void ServoPowerDlg::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == myTimerId) {
		if (needSwitchOn) {

			needSwitchOn = false;
			servoPower.sltSwitchPower(1);
			return;
		}
		if (needSwitchOff) {

			needSwitchOff = false;
			servoPower.sltSwitchPower(0);
			return;
		}
		if (needSetCurrent) {
			needSetCurrent = false;
			double current = ui->dsbCurrent->value();
			qDebug() << "Current:" << QString::number(current);
			servoPower.sltChangeCurrent((unsigned int)(round(current * 100)));
			return;
		}
		if (needSetVoltage) {
			needSetVoltage = false;
			double voltage = ui->dsbVoltage->value();
			qDebug() << "Voltage:" << QString::number(voltage);
			servoPower.sltChangeVoltage((unsigned int)(round(voltage * 10)));
			return;
		}
		static int timerTask = 0;
		int curtask = timerTask++ % 4;
		switch (curtask) {
		case 0:
			sltRefreshStatus();
			break;
		case 1:
			sltRefreshVoltage();
			break;
		case 2:
			sltRefreshRatedVoltage();
			break;
		case 3:
			sltRefreshPower();
			break;

		}
	}
}

void ServoPowerDlg::sltSliderCurrentChanged(int value)
{
	ui->dsbCurrent->setValue(value);
}
void ServoPowerDlg::sltSliderVoltageChanged(int value)
{
	ui->dsbVoltage->setValue(value);
}

void ServoPowerDlg::sltShowPowerCurrent(int value)
{
    double current = value / 100.0;
    ui->lnCurrent->display(current);
}

void ServoPowerDlg::sltShowPowerPower(unsigned short thousand, unsigned short walt, unsigned short mwalt)
{
    double power = 0.0;
    power = thousand * 1000.0;
    power += walt;
    power += mwalt * 0.001;
    ui->lnPower->display(power);
}

void ServoPowerDlg::sltShowPowerVoltage(int value)
{
	//ui->lnVoltage
    double voltage = value / 10.0;
    ui->lnVoltage->display(voltage);
}

void ServoPowerDlg::sltShowPowerStatus(int onoff)
{
	ui->lpStatus->setTriggered(onoff);
	ui->rbTurnOff->setChecked(onoff == 0);
	ui->rbTurnOn->setChecked(onoff != 0);
}

ServoPowerDlg::~ServoPowerDlg()
{
	killTimer(myTimerId);
	servoPower.try_quit();
	servoPower.wait();
    delete ui;
}


void ServoPowerDlg::sltSetVoltage()
{
	needSetVoltage = true;
}

void ServoPowerDlg::sltSetCurrent()
{
	needSetCurrent = true;
}

void ServoPowerDlg::sltRefreshVoltage()
{
	servoPower.sltGetVoltage();
}

void ServoPowerDlg::sltRefreshCurrent()
{
	servoPower.sltGetCurrent();
}

void ServoPowerDlg::sltRefreshPower()
{
	servoPower.sltGetPower();
}

void ServoPowerDlg::sltRefreshStatus()
{
	servoPower.sltRefreshStatus();
}

void ServoPowerDlg::sltPowerOn()
{
	needSwitchOn = true;
}

void ServoPowerDlg::sltPowerOff()
{
	needSwitchOff = true;
}

void ServoPowerDlg::sltRefreshRatedVoltage()
{
    servoPower.sltGetRatedVoltage();
}

void ServoPowerDlg::sltRefreshRatedCurrent()
{
    servoPower.sltGetRatedCurrent();
}

void ServoPowerDlg::sltPowerMsg(QString msg)
{

}

void ServoPowerDlg::sltShowPowerRatedCurrent(int value)
{
	ui->lnRatedCurrent->display(value / 100.0);
}
void ServoPowerDlg::sltShowPowerRatedVoltage(int value)
{
	ui->lnRatedVoltage->display(value / 10.0);
}

