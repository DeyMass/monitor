#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

struct mySettings{
	int		updateDelay;

	int		noticeFontSize;
	int		noticeTime;
	QString noticeColor;

	int		destPort;
	QString destIp;
	QString destUrl;
	QString destHost;
}settingsPack;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFile *settingsF = new QFile("settings.ini");
	QSettings *sett = new QSettings("settings.ini", QSettings::IniFormat);
	if (settingsF->exists()){


	}
	else{
		sett->setValue("main/updateDelay", 5000);
		sett->setValue("notice/FontSize", 122);
		sett->setValue("notice/Color","#FFFFFF");
		sett->setValue("notice/Color","#FFFFFF");
		sett->setValue("notice/Time", 3);
		sett->setValue("server/Ip","109.202.0.226");
		sett->setValue("server/Port","80");
		sett->setValue("server/Url","http://reg154.point.at-sibir.ru/rest/monitoring");
		sett->setValue("server/Host","reg154.point.at-sibir.ru");



	}
	MainWindow w;
	w.show();

	return a.exec();
}
