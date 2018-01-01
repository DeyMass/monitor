#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <structures.h>
#include <QMessageBox>

void writeDefaults(QSettings *sett){
	sett->setValue("main/updateDelay", 5000);
	sett->setValue("notice/FontSize", 122);
	sett->setValue("notice/FontColor","#FFFFFF");
	sett->setValue("notice/FontFamily","Arial");
	sett->setValue("notice/Time", 3);
	sett->setValue("server/Ip","109.202.0.226");
	sett->setValue("server/Port",80);
	sett->setValue("server/Url","http://reg154.point.at-sibir.ru/rest/monitoring");
	sett->setValue("server/Host","reg154.point.at-sibir.ru");
	sett->sync();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	mySettings settings;
	QFile *settingsF = new QFile("settings.ini");
	QSettings *sett = new QSettings("settings.ini", QSettings::IniFormat);
	if (settingsF->exists()){
		settings.updateDelay = sett->value("main/updateDelay").toInt();

		settings.noticeFontSize = sett->value("notice/FontSize").toInt();
		settings.noticeFontFamily = sett->value("notice/FontFamily").toString();
		settings.noticeFontColor = sett->value("notice/FontColor").toString();
		settings.noticeTime = sett->value("notice/Time").toInt();


		settings.destPort = sett->value("/server/Port").toInt();
		settings.destHost = sett->value("/server/Host").toString();
		settings.destUrl = sett->value("/server/Url").toString();
		settings.destIp = sett->value("/server/Ip").toString();
		if (settings.destPort <= 0 || settings.destHost == "" ||
				settings.destIp == "" || settings.destUrl == "" ||
				settings.noticeFontColor == "" || settings.noticeFontSize == 0 ||
				settings.noticeTime <= 0 || settings.updateDelay <= 0 ||
				settings.noticeFontFamily == ""){
			QMessageBox::critical(NULL, "Load settings error", "Error while loading settings. Setttings is set to default values");
			writeDefaults(sett);
		}
	}
	else{
		writeDefaults(sett);
	}
	MainWindow w;
	w.setSettings(settings);
	w.show();

	return a.exec();
}
