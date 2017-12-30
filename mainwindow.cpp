#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popup.h"
#include <QMediaPlayer>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	serverStatus = SERVER_DOWN;
	ui->setupUi(this);

	stopped = new QPixmap(":/img/rsrc/img/serverDown.jpg");
	working = new QPixmap(":/img/rsrc/img/serverUp.jpg");
	updatin = new QPixmap(":/img/rsrc/img/serverSync.jpg");

	request = new QTcpSocket();
	connect(request, SIGNAL(readyRead()),this,SLOT(readyRead()));
	connect(request, SIGNAL(connected()), this, SLOT(connected()));
	connect(request, SIGNAL(disconnected()), this, SLOT(disconnected()));

	QTimer *reconnect = new QTimer;
	connect(reconnect, SIGNAL(timeout()), this, SLOT(reconnect()));
	reconnect->start(5000);
	QTimer *resize = new QTimer();
	connect(resize, SIGNAL(timeout()), this, SLOT(resize()));
	resize->start(125);

	MainWindow::reconnect();
}

void MainWindow::resize(){
	if (ui->mainTable->columnWidth(0) > this->width() - BORDER){
		ui->mainTable->setColumnWidth(0, this->width() - BORDER);
		ui->mainTable->setColumnWidth(1, BORDER - 25);
	}
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

	if (response.contains("Workflow")){
		ui->statusPic->setText("OK");
		ui->statusPic->setStyleSheet("color: green");
		if (serverStatus != SERVER_UP){
			serverStatus = SERVER_UP;
			messageCreate("Server is back online");
			playSound(QUrl::fromLocalFile("start.mp3"));
		}
		parse(response);
	}
	else
		if (response.contains("UML")){
			if (serverStatus != SERVER_UML){
				serverStatus = SERVER_UML;
				ui->statusPic->setText("UML_SYNC");
				ui->statusPic->setStyleSheet("color: yellow");
				messageCreate("UML Synchronization started");
				playSound(QUrl::fromLocalFile("uml.mp3"));
			}
			ui->statusPic->setPixmap(updatin->scaled(25,25));
		}
		else{
			if (serverStatus != SERVER_DOWN){
				serverStatus = SERVER_DOWN;
				ui->statusPic->setText("DOWN");
				ui->statusPic->setStyleSheet("color: red");
				messageCreate("Server is shutted down");
				playSound(QUrl::fromLocalFile("stop.mp3"));
			}
		}
}

void MainWindow::playSound(QUrl src){
	QMediaPlayer *player = new QMediaPlayer();
	player->setMedia(src);
	player->play();
	qDebug() << "played" << src;
}

void MainWindow::messageCreate(QString msg){
	PopUp *field = new PopUp;
	field->setPopupText(msg);
	field->setTextStyle("font-size: 144px; color: #FFFFFF; font-family: Arial");
	field->show();
}

void MainWindow::parse(QString response){
	int isFirst = 0;
	if (ui->mainTable->columnCount() == 0){
		ui->mainTable->insertColumn(0);
		ui->mainTable->setColumnWidth(0, 300);
		ui->mainTable->insertColumn(1);
		isFirst = 1;
	}
	int counter = 0;
	QString name;
	//ui->mainTable->setColumnWidth(0, this->width() - 150);
	QString status;
	for (int i = 0; i < response.length()-24; i++){
		if (checkKeyWordInPos(&i, response, "<br/>"))
		{

			if (checkKeyWordInPos(&i, response, "Сборка")){
				skipData(&i, response);
				qDebug() << "skip";
				continue;
			}
			if (ui->mainTable->rowCount() == counter)
				ui->mainTable->insertRow(counter);
			QTableWidgetItem* nameItem = new QTableWidgetItem;
			QTableWidgetItem* statusItem = new QTableWidgetItem;
			name = readName(&i, response);
			status = readStatus(&i, response);
			if (!isFirst && ui->mainTable->item(counter, 1)->text() == status)
			{
				counter++;
				continue;
			}

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
	while(response.at(temp) != "<"){
		if (checkKeyWordInPos(&temp, response, ":Connections")){
			temp += 13;
		}
		txt += response.at(temp++);
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
		qDebug() << text.at(temp+i) << ":" << key.at(i);
		if (temp+i > text.length() || text.at(temp + i) != key.at(i)) return false;
	}
	return true;
}

void MainWindow::disconnected(){
	qDebug()<<"disconnect";
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_mainTable_cellChanged(int row, int column)
{

}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
	for (int i = 0; i < ui->mainTable->rowCount(); i++){
		if (!ui->mainTable->item(i, 0)->text().contains(arg1)) {
			ui->mainTable->hideRow(i);
		}
		else
			ui->mainTable->showRow(i);
	}
}
