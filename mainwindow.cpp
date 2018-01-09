#include "mainwindow.h"
#include <QMessageBox>
#include <QDate>
#include "ui_mainwindow.h"

void MainWindow::connectionHandler()
{
	request = new QTcpSocket();
	connect(request, SIGNAL(readyRead()),this,SLOT(readyRead()));
	connect(request, SIGNAL(connected()), this, SLOT(connected()));
	connect(request, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void MainWindow::resizeTimer()
{
	QTimer *resize = new QTimer();
	connect(resize, SIGNAL(timeout()), this, SLOT(resize()));
	resize->start(125);
}

void MainWindow::loadIconPics()
{
	QPixmap *pic;
	pic = new QPixmap(":/img/rsrc/img/trayUNKNOWN.jpg");
	icoUnknown = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayOK.jpg");
	icoUp = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayUML.jpg");
	icoUml = new QIcon(pic->scaled(25,25));
	pic = new QPixmap(":/img/rsrc/img/trayDOWN.jpg");
	icoDown = new QIcon(pic->scaled(25,25));

	delete pic;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	serverStatus = SERVER_UNKN;
	ui->setupUi(this);
	this->setWindowTitle("Monitor");
	QString log = getTime() + "\tNew monitoring session started\n";
	logger(log);

	connectionHandler();

	resizeTimer();

	MainWindow::reconnect();
	field = new PopUp;
	player = new QMediaPlayer();
	tray = new QSystemTrayIcon();
	loadIconPics();

	tray->setIcon(*icoUnknown);
	this->setWindowIcon(*icoUnknown);
	connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconHandler(QSystemTrayIcon::ActivationReason)));
	tray->show();
	stateChanged(SERVER_UNKN);
}

void MainWindow::changeEvent(QEvent *event){
	switch(event->type()){
	case QEvent::WindowStateChange:
		if (this->isMinimized()){
			this->hide();
		}
		break;
	default:
		break;
	}
}

void MainWindow::trayIconHandler(QSystemTrayIcon::ActivationReason reason){
	switch (reason) {
	case QSystemTrayIcon::Trigger:
		if (this->isHidden()){
			this->show();
			this->setWindowState(Qt::WindowState::WindowActive);
		}
		break;
	default:
		break;
	}
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
	if (noticeTime >= 0)
		field->setDelay(noticeTime);
	else
		field->setDelay(3000);

	QTimer *reconnect = new QTimer;
	connect(reconnect, SIGNAL(timeout()), this, SLOT(reconnect()));
	reconnect->start(updDelay);
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
		this->setWindowIcon(*icoUnknown);
		ui->statusPic->setText("UNKNOWN");
		ui->statusPic->setStyleSheet("color: #0000AA");
		break;
	case SERVER_UP:
		tray->setIcon(*icoUp);
		this->setWindowIcon(*icoUp);
		ui->statusPic->setText("OK");
		ui->statusPic->setStyleSheet("color: #00AA00");
		messageCreate("Server is back online");
		playSound(QUrl::fromLocalFile(QDir::currentPath() + "/start.mp3"));
		break;
	case SERVER_UML:
		tray->setIcon(*icoUml);
		this->setWindowIcon(*icoUml);
		ui->statusPic->setText("UML_SYNC");
		ui->statusPic->setStyleSheet("color: #FFA500");
		messageCreate("UML Synchronization started");
		playSound(QUrl::fromLocalFile(QDir::currentPath() + "/uml.mp3"));
		break;
	case SERVER_DOWN:
		tray->setIcon(*icoDown);
		this->setWindowIcon(*icoDown);
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
	QString log;
	if (response.contains("Monitoring") ||
		response.contains("Not found")){
		if (serverStatus != SERVER_UNKN){
			clearTable();
			stateChanged(SERVER_UNKN);
			QString log = getTime() + "\tMonitoring failed\n";
			logger(log);
			return;
		}
	}
	if (response.contains("Workflow")){
		ui->mainTable->setEnabled(1);
		if (serverStatus != SERVER_UP){
			stateChanged(SERVER_UP);
			log = getTime() + "\tSERVER" + "\tentered state:\tUP\n";
		}
		parse(response);
	}
	else{
		ui->mainTable->setDisabled(1);
		if (response.contains("UML")){
			if (serverStatus != SERVER_UML){
				stateChanged(SERVER_UML);
				log = getTime() + "\tSERVER" + "\tentered state:\tUML_SYNC\n";
			}
		}
		else {
			if (serverStatus != SERVER_DOWN){
				clearTable();
				stateChanged(SERVER_DOWN);
				log = getTime() + "\tSERVER" + "\tentered state:\tDOWN\n";
			}
		}
	}
	logger(log);
}

void MainWindow::clearTable(){
	ui->mainTable->clear();
	for (int i = ui->mainTable->rowCount(); i >= 0; i--)
		ui->mainTable->removeRow(i);
}

void MainWindow::playSound(QUrl src){
	player->setMedia(src);
	player->play();
	qDebug() << "played" << src;
}

void MainWindow::messageCreate(QString msg){
	field->setPopupText(msg);
	field->setTextStyle("font-size: " + QString::number(noticeFontS) + "px; color: " + noticeFontC + "; font-family: " + noticeFontF);
	field->show();
}

void MainWindow::parse(QString response){
	if (ui->mainTable->columnCount() == 0){
		ui->mainTable->insertColumn(0);
		ui->mainTable->setColumnWidth(0, 300);
		ui->mainTable->insertColumn(1);
	}
	int counter = 0;
	QString name;
	QString status;
	for (int i = 0; i < response.length()-24; i++){
		if (checkKeyWordInPos(&i, response, "<br/>")){
			int temp = i + 5;
			if (checkKeyWordInPos(&temp, response, "Сборка")){
				skipData(&i, response);
				continue;
			}
			if (ui->mainTable->rowCount() == counter){
				ui->mainTable->insertRow(counter);
				QTableWidgetItem *empty = new QTableWidgetItem;
				empty->setText("");
				ui->mainTable->setItem(counter, 0, empty);
				empty = new QTableWidgetItem;
				empty->setText("");
				ui->mainTable->setItem(counter, 1, empty);
			}
			QTableWidgetItem* nameItem = new QTableWidgetItem;
			QTableWidgetItem* statusItem = new QTableWidgetItem;
			name = readName(&i, response);
			status = readStatus(&i, response);

			if (ui->mainTable->item(counter, 0)->text() == name &&
				ui->mainTable->item(counter, 1)->text() == status){
				counter++;
				continue;
			}
			if(status.contains("OK")) statusItem->setBackgroundColor(QColor(0,255,0));
			else if(status.contains("FAIL")) statusItem->setBackgroundColor(QColor(255,0,0));
				else if(status.contains("UNKNOWN")) statusItem->setBackground(QColor(0,0,255));
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
			OR checkKeyWordInPos(&temp, response, ":Connections")
			OR checkKeyWordInPos(&temp, response, "UNKNOWN"))
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

void MainWindow::logger(QString event)
{
	QFile *log = new QFile("log.txt");
	if (log->open(QIODevice::Append)){
		QByteArray arr;
		arr.clear();
		arr.append(event);
		log->write(arr);
	}
	else{
		QMessageBox::critical(NULL, "Error", "Unable to create or read file: log.txt");
	}
	log->close();
	delete log;

}

void MainWindow::skipData(int *pos, QString response){
	int temp = *pos + 5;
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
	if (column == 0) return;
	if (ui->mainTable->item(row, column)->text() == "") return;
	//-----
	QString msg = "changed " + QString::number(row) + " : " + QString::number(column);
	qDebug() << msg;
	//-----
	QString stateName = ui->mainTable->item(row, 0)->text();
	QString stateStatus = ui->mainTable->item(row, 1)->text();
	stateName.chop(1);
	msg = getTime() + "\t" + stateName + "\tentered state:\t" + stateStatus + "\n";
	logger(msg);
}

QString MainWindow::getTime(){
	return QDate::currentDate().toString("dd.MM.yyyy") + "\t" + QTime::currentTime().toString("hh:mm:ss");
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
	for (int i = 0; i < ui->mainTable->rowCount(); i++){
		if (!ui->mainTable->item(i, 0)->text().contains(arg1, Qt::CaseInsensitive))
			ui->mainTable->hideRow(i);
		else
			ui->mainTable->showRow(i);
	}
}
