#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popup.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	request = new QTcpSocket();
	connect(request, SIGNAL(readyRead()),this,SLOT(readyRead()));
	connect(request, SIGNAL(connected()), this, SLOT(connected()));
	connect(request, SIGNAL(disconnected()), this, SLOT(disconnected()));
	QTimer *reconnect = new QTimer;
	connect(reconnect, SIGNAL(timeout()), this, SLOT(reconnect()));
	reconnect->start(5000);
	PopUp* msg = new PopUp();
	msg->setPopupText("event1");
	msg->show();
	MainWindow::reconnect();
}

void MainWindow::reconnect(){
	request->connectToHost("109.202.0.226",80);
}

void MainWindow::refresh(){
	//refresh
}

void MainWindow::connected(){
	request->write("GET http://reg154.point.at-sibir.ru/rest/monitoring\r\n"
				   "Host: reg154.point.at-sibir.ru");
}

void MainWindow::readyRead(){
	QString response = request->readAll();
	parse(response);
}

void MainWindow::parse(QString response){
	if (ui->mainTable->columnCount() == 0){
		ui->mainTable->insertColumn(0);
		ui->mainTable->insertColumn(1);
	}
	int counter = 0;
	QString name;
	ui->mainTable->setColumnWidth(0, this->width() - 150);
	QString status;
	for (int i = 0; i < response.length()-24; i++){
		if (response.at(i) == "<" &&
				response.at(i+1) == "b" &&
				response.at(i+2) == "r" &&
				response.at(i+3) == "/" &&
				response.at(i+4) == ">")
		{
			int temp = i + 5;
			if (checkKeyWordInPos(&temp, response, "Сборка")){
				skipData(&temp, response);
				qDebug() << "skip";
				continue;
			}
			if (ui->mainTable->rowCount() == counter)
				ui->mainTable->insertRow(counter);
			QTableWidgetItem* nameItem = new QTableWidgetItem;
			QTableWidgetItem* statusItem = new QTableWidgetItem;
			name = readName(&i, response);
			status = readStatus(&i, response);
			if(status.contains("OK")) statusItem->setBackgroundColor(QColor(0,255,0));
			else if(status.contains("FAIL")) statusItem->setBackgroundColor(QColor(255,0,0));
			else statusItem->setBackground(QColor(255,255,0));

			nameItem->setText(name);
			statusItem->setText(status);
			ui->mainTable->setItem(counter,0,nameItem);
			ui->mainTable->setItem(counter++,1,statusItem);
		}
	}
}

QString MainWindow::readName(int *pos, QString response){
	*pos += 4;
	QString txt = "";
	int temp = *pos;
	while(temp++){
		if (checkKeyWordInPos(&temp, response, "OK")
			OR checkKeyWordInPos(&temp, response, "Someth")
			OR checkKeyWordInPos(&temp, response, "FAIL")
			OR checkKeyWordInPos(&temp, response, ":Connections"))
			break;
		txt += response.at(temp);
	}

	*pos = temp;
	return txt;
}

QString MainWindow::readStatus(int *pos, QString response)
{
	QString txt = "";
	int temp = *pos;
	for (; response.at(temp) != "<"; temp++){
		if (checkKeyWordInPos(&temp, response, ":Connections")){
			temp += 13;
		}

		txt += response.at(temp);
	}
	*pos = temp - 1;
	return txt;
}

void MainWindow::skipData(int *pos, QString response){
	int temp = *pos;
	while(response.at(temp++) != "<");
}

bool MainWindow::checkKeyWordInPos(int *pos, QString text, QString key)
{
	int temp = *pos;
	for (int i = 0; i < key.length(); i++){
		if (text.at(temp + i) != key.at(i)){return false;}
	}
	//*pos = temp + key.length();
	return true;
}

void MainWindow::disconnected(){
	qDebug()<<"disconnect";
}

MainWindow::~MainWindow()
{
	delete ui;
}
