#include "statistics.h"
#include "ui_statistics.h"
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QDateTime>

Statistics::Statistics(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Statistics)
{
	ui->setupUi(this);
	QDir *dir = new QDir;
	dir->cd("logs");
	for (unsigned int i = 2; i < dir->count(); i++){
		QString name = dir->entryList().at(i);
		name.chop(4);
		ui->moduleName->addItem(name);
	}
	ui->timeInOkStatePb->setStyleSheet("QProgressBar::chunk{ background-color: green} QProgressBar{text-align: right; margin-right: 30px}");
	ui->timeInWrongStatePb->setStyleSheet("QProgressBar::chunk{ background-color: yellow} QProgressBar{text-align: right; margin-right: 30px}");
	ui->timeInFailStatePb->setStyleSheet("QProgressBar::chunk{ background-color: red} QProgressBar{text-align: right; margin-right: 30px}");

}

Statistics::~Statistics()
{
	delete ui;
}

#define SEC_MS	1000
#define MIN_MS	(SEC_MS * 60)
#define HOUR_MS	(MIN_MS * 60)

QTime operator - ( const QTime t1, const QTime t2 )
{
	int diff;
	if (t2 < t1)
		diff = t2.msecsTo(t1);
	else
		diff = t1.msecsTo(t2);
	return QTime(diff / HOUR_MS,
			  (diff % HOUR_MS) / MIN_MS,
			  (diff % MIN_MS) / SEC_MS,
			   diff % SEC_MS);
}

//return QTime(diff / 60*60,
//		  (diff % 60*60) / 60,
//		  (diff % 60) / 1,
//		   diff % 1);

void Statistics::on_moduleName_currentIndexChanged(const QString &arg1)
{
	QFile *f;
	f = new QFile("./logs/" + arg1 + ".txt");

	if (!f->open(QIODevice::ReadOnly)){
		QMessageBox::critical(NULL, "Error", "Unable to create or read file: log.txt");
		return;
	}

	QString status;

	QString str = f->readLine();
	QDate startD = parseDate(str);
	QTime startT = parseTime(str);
	QDate stopD;
	QTime stopT;

	QDateTime timeStart, timeEnd;
	QDateTime infoStart, infoEnd;
	timeStart.setDate(startD);
	timeStart.setTime(startT);

	infoStart.setDate(startD);
	infoStart.setTime(startT);

	QString prevStatus = parseState(str);

	int totalSecs = 0;
	int upSecs;
	int unknSecs;
	int failSecs;
	int wrongSecs;
	upSecs = failSecs = wrongSecs = unknSecs = 0;
	while(!f->atEnd()){
		str = f->readLine();
		stopD = parseDate(str);
		stopT = parseTime(str);
		status = parseState(str);
		if (stopD < ui->dateFrom->date() && stopD <= ui->dateTill->date()){
			infoStart.setDate(stopD);
			infoStart.setTime(stopT);
			continue;
		}

		if (stopD > ui->dateFrom->date() && stopD > ui->dateTill->date()){
			infoStart.setDate(QDate::currentDate());
			infoStart.setTime(QTime::currentTime());
			break;
		}

		if (prevStatus != status){
			if (prevStatus == "DOWN" || prevStatus == "FAIL"){
				infoEnd.setDate(stopD);
				infoEnd.setTime(stopT);
				failSecs += infoStart.secsTo(infoEnd);
				infoStart.setDate(stopD);
				infoStart.setTime(stopT);
			}
			if (prevStatus == "UP" || prevStatus == "OK"){
				qDebug() << prevStatus;
				infoEnd.setDate(stopD);
				infoEnd.setTime(stopT);
				qDebug() << "prev time: " << infoStart.time() << "new time: " << infoEnd.time();
				upSecs += infoStart.secsTo(infoEnd);
				infoStart.setDate(stopD);
				infoStart.setTime(stopT);
			}
			if (prevStatus == "UML_SYNC" || prevStatus == "Something wrong"){
				infoEnd.setDate(stopD);
				infoEnd.setTime(stopT);
				wrongSecs += infoStart.secsTo(infoEnd);
				infoStart.setDate(stopD);
				infoStart.setTime(stopT);
			}
			if (prevStatus == "UNKNOWN"){
				infoEnd.setDate(stopD);
				infoEnd.setTime(stopT);
				unknSecs += infoStart.secsTo(infoEnd);
				infoStart.setDate(stopD);
				infoStart.setTime(stopT);
			}
			prevStatus = status;
		}
		if (str.contains("session is over")){
			timeEnd.setDate(stopD);
			timeEnd.setTime(stopT);
			totalSecs += timeStart.secsTo(timeEnd);
			infoStart.setDate(startD);
			infoStart.setTime(startT);

		}
		if (str.contains("session started")){
			timeStart.setDate(stopD);
			timeStart.setTime(stopT);
			infoStart.setDate(stopD);
			infoStart.setTime(stopT);
		}
	}
	timeEnd.setDate(QDate::currentDate());
	timeEnd.setTime(QTime::currentTime());
	totalSecs += timeStart.secsTo(timeEnd);
	stopD = QDate::currentDate();
	stopT = QTime::currentTime();
	prevStatus = status;
	if (prevStatus == "DOWN" || prevStatus == "FAIL"){
		infoEnd.setDate(stopD);
		infoEnd.setTime(stopT);
		failSecs += infoStart.secsTo(infoEnd);
		infoStart.setDate(stopD);
		infoStart.setTime(stopT);
	}
	if (prevStatus == "UP" || prevStatus == "OK"){
		qDebug() << prevStatus;
		infoEnd.setDate(stopD);
		infoEnd.setTime(stopT);
		qDebug() << "prev time: " << infoStart.time() << "new time: " << infoEnd.time();
		upSecs += infoStart.secsTo(infoEnd);
		infoStart.setDate(stopD);
		infoStart.setTime(stopT);
	}
	if (prevStatus == "UML_SYNC" || prevStatus == "Something wrong"){
		infoEnd.setDate(stopD);
	infoEnd.setTime(stopT);
		wrongSecs += infoStart.secsTo(infoEnd);
		infoStart.setDate(stopD);
		infoStart.setTime(stopT);
	}
	if (prevStatus == "UNKNOWN"){
		infoEnd.setDate(stopD);
		infoEnd.setTime(stopT);
		unknSecs += infoStart.secsTo(infoEnd);
		infoStart.setDate(stopD);
		infoStart.setTime(stopT);
	}
	qDebug() << "total secs: " << totalSecs;

	QTime result(0, 0, 0);
	result = result.addSecs(upSecs);
	//ui->timeInOkState->setText(ui->timeInOkState->text().append(" " + result.toString("hh:mm:ss")));
	ui->timeOk->setText(result.toString("hh:mm:ss"));
	ui->timeInOkStatePb->setMaximum(totalSecs);
	ui->timeInOkStatePb->setValue(upSecs);
	qDebug() << "OK secs: " << upSecs;

	result = QTime(0,0,0);
	result = result.addSecs(failSecs);
	//ui->timeInFailState->setText(ui->timeInFailState->text() + " " + result.toString("hh:mm:ss"));
	ui->timeFail->setText(result.toString("hh:mm:ss"));
	ui->timeInFailStatePb->setMaximum(totalSecs);
	ui->timeInFailStatePb->setValue(failSecs);
	qDebug() << "FAIL secs: " << failSecs;

	result = QTime(0,0,0);
	result = result.addSecs(unknSecs);
	//ui->timeInWrongState->setText(ui->timeInWrongState->text().append(" " + result.toString("hh:mm:ss")));
	ui->timeUnkn->setText(result.toString("hh:mm:ss"));
	ui->timeInWrongStatePb->setMaximum(totalSecs);
	ui->timeInWrongStatePb->setValue(unknSecs);
	qDebug() << "UNKN secs: " << unknSecs;

	result = QTime(0,0,0);
	result = result.addSecs(unknSecs + failSecs + upSecs);
	ui->totalValue->setText(result.toString("hh:mm:ss"));

	//ui->totalValue->setText(QString::number(days));
	f->close();
}

QTime Statistics::parseTime(QString str){
	str.truncate(19);
	str.remove(0, 11);
	QString h = str;
	QString m = str;
	QString s = str;
	h.resize(2);
	m.remove(0,3);
	m.remove(2, 10);
	s.remove(0,6);
	QTime time = QTime(h.toInt(), m.toInt(), s.toInt());
	return time;
}

QDate Statistics::parseDate(QString str){
	str.resize(10);
	QString day = str;
	QString mon = str;
	QString yea = str;
	day.resize(2);
	mon.remove(0,3);
	mon.remove(2, 10);
	yea.remove(0,6);
	QDate date = QDate(yea.toInt(), mon.toInt(), day.toInt());
	return date;
}

QString Statistics::parseState(QString str)
{
	if (str.contains("OK"))				return QString("OK");
	if (str.contains("FAIL"))			return QString("FAIL");
	if (str.contains("UNKNOWN") ||
		str.contains("Something going wrong"))return QString("UNKNOWN");

	if (str.contains("UP"))				return QString("UP");
	if (str.contains("DOWN"))			return QString("DOWN");
	if (str.contains("UML"))			return QString("UML_SYNC");

	return "";
}

void Statistics::on_pushButton_clicked()
{
	on_moduleName_currentIndexChanged(ui->moduleName->currentText());
}
