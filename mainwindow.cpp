#include "mainwindow.h"
#include <QMessageBox>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	serverStatus = SERVER_UNKN;
	ui->setupUi(this);

	request = new QTcpSocket();
	connect(request, SIGNAL(readyRead()),this,SLOT(readyRead()));
	connect(request, SIGNAL(connected()), this, SLOT(connected()));
	connect(request, SIGNAL(disconnected()), this, SLOT(disconnected()));

	QTimer *reconnect = new QTimer;
	connect(reconnect, SIGNAL(timeout()), this, SLOT(reconnect()));
	reconnect->start(updDelay);
	QTimer *resize = new QTimer();
	connect(resize, SIGNAL(timeout()), this, SLOT(resize()));
	resize->start(125);

	MainWindow::reconnect();
	field = new PopUp;
	player = new QMediaPlayer();
	tray = new QSystemTrayIcon();
	QPixmap *pic;
	pic = new QPixmap(":/img/rsrc/img/trayUNKNOWN.jpg");
	icoUnknown = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayOK.jpg");
	icoUp = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayUML.jpg");
	icoUml = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayDOWN.jpg");
	icoDown = new QIcon(pic->scaled(25,25));

	tray->setIcon(*icoUnknown);
	tray->show();
	stateChanged(SERVER_UNKN);
	delete pic;
}

void MainWindow::setSettings(mySettings set)
{
	updDelay = set.updateDelay;

	noticeFontS = set.noticeFontSize;
	noticeFontF = set.noticeFontFamily;
	noticeFontC = set.noticeFontColor;
	noticeTime = set.noticeTime;

	serverIp = set.destIp;
	serverHost = set.destHost;
	serverPort = set.destPort;
	serverUrl = set.destUrl;
}

void MainWindow::resize(){
	if (ui->mainTable->columnWidth(0) > this->width() - BORDER){
		ui->mainTable->setColumnWidth(0, this->width() - BORDER);
		ui->mainTable->setColumnWidth(1, BORDER - 25);
	}
}

void MainWindow::reconnect(){
	request->connectToHost(serverIp, serverPort);
}

void MainWindow::refresh(){
	//refresh
}

void MainWindow::connected(){
	QString reqStr = "GET " + serverUrl + "\r\nHost: " + serverHost;
	QByteArray sendStr;
	for (int i = 0; i < reqStr.length(); i++){
		sendStr.append(reqStr.at(i));
	}
	request->write(sendStr);
}

void MainWindow::stateChanged(int newState)
{
	switch (newState) {
	case SERVER_UNKN:
		tray->setIcon(*icoUnknown);
		ui->statusPic->setText("UNKNOWN");
		ui->statusPic->setStyleSheet("color: #0000AA");
		break;
	case SERVER_UP:
		tray->setIcon(*icoUp);
		ui->statusPic->setText("OK");
		ui->statusPic->setStyleSheet("color: #00AA00");
		messageCreate("Server is back online");
		playSound(QUrl::fromLocalFile(QDir::currentPath() + "/start.mp3"));
		break;
	case SERVER_UML:
		tray->setIcon(*icoUml);
		ui->statusPic->setText("UML_SYNC");
		ui->statusPic->setStyleSheet("color: #AAAA00; background: #999999");
		messageCreate("UML Synchronization started");
		playSound(QUrl::fromLocalFile(QDir::currentPath() + "/uml.mp3"));
		break;
	case SERVER_DOWN:
		tray->setIcon(*icoDown);
		ui->statusPic->setText("DOWN");
		ui->statusPic->setStyleSheet("color: #AA0000");
		messageCreate("Server is shutted down");
		playSound(QUrl::fromLocalFile(QDir::currentPath() + "/stop.mp3"));
	default:
		break;
	}
	serverStatus = newState;
}

void MainWindow::readyRead(){
	QString response = request->readAll();
	if (response.contains("Workflow")){
		if (serverStatus != SERVER_UP)
			stateChanged(SERVER_UP);
		parse(response);
	}
	else{
		if (response.contains("UML")){
			if (serverStatus != SERVER_UML)
				stateChanged(SERVER_UML);
		}
		else{
			if (serverStatus != SERVER_DOWN)
				stateChanged(SERVER_DOWN);
		}
	}
}

void MainWindow::playSound(QUrl src){
	player->setMedia(src);
	player->play();
	qDebug() << "played" << src;
}

void MainWindow::messageCreate(QString msg){
	delete field;
	field = new PopUp;
	field->setPopupText(msg);
	//добавить setTime!
	field->setTextStyle("font-size: " + QString::number(noticeFontS) + "px; color: " + noticeFontC + "; font-family: " + noticeFontF);
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
		if (temp+i > text.length() || text.at(temp + i) != key.at(i)) return false;
	}
	return true;
}

void MainWindow::disconnected(){
}

MainWindow::~MainWindow()
{
	request->close();
	delete tray;
	delete player;
	delete icoDown;
	delete icoUp;
	delete icoUml;
	delete icoUnknown;
	delete ui;
	delete request;
}

void MainWindow::on_mainTable_cellChanged(int row, int column)
{

}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
	for (int i = 0; i < ui->mainTable->rowCount(); i++){
		if (!ui->mainTable->item(i, 0)->text().contains(arg1, Qt::CaseInsensitive)) {
			ui->mainTable->hideRow(i);
		}
		else
			ui->mainTable->showRow(i);
	}
}
