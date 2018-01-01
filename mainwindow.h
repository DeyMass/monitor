#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "popup.h"
#include "structures.h"
#include <QMediaPlayer>
#include <QFile>
#include <QDir>
#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableWidget>
#include <QSystemTrayIcon>

#define BORDER		70
#define OR			||

#define SERVER_UNKN -1
#define SERVER_DOWN 0
#define SERVER_UP	1
#define SERVER_UML	2

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
	void	setSettings(mySettings set);

    ~MainWindow();

private:
	//settings
	int		updDelay = 5000;
	int		noticeTime = 3;
	int		noticeFontS = 122;
	QString	noticeFontC = "#FFFFFF";
	QString	noticeFontF = "Arial";
	QString serverIp = "109.202.0.226";
	QString serverUrl = "http://reg154.point.at-sibir.ru/rest/monitoring";
	QString serverHost = "reg154.point.at-sibir.ru";
	int serverPort = 80;

	int serverStatus;
	QTcpSocket *request;
	PopUp *field;
	QMediaPlayer *player;
	QSystemTrayIcon *tray;
	QIcon *icoUp;
	QIcon *icoDown;
	QIcon *icoUml;
	QIcon *icoUnknown;
	Ui::MainWindow *ui;

	void	skipData(int *pos, QString response);
	void	parse(QString response);
	void	messageCreate(QString msg);
	void	playSound(QUrl src);
	void	stateChanged(int newState);
	bool	checkKeyWordInPos(int *pos, QString text, QString key);
	QString readName(int *pos, QString response);
	QString readStatus(int *pos, QString response);

private slots:
    void reconnect();
    void connected();
    void readyRead();
    void disconnected();
    void refresh();
	void resize();
	void on_mainTable_cellChanged(int row, int column);
	void on_lineEdit_textChanged(const QString &arg1);
};

#endif // MAINWINDOW_H
