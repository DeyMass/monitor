#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableWidget>

#define BORDER		70
#define OR			||
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
    ~MainWindow();

private:
	QPixmap *stopped;
	QPixmap *working;
	QPixmap *updatin;
	int serverStatus;
	QTcpSocket *request;

    Ui::MainWindow *ui;
    QString readName(int *pos, QString response);
    QString readStatus(int *pos, QString response);
    void skipData(int *pos, QString response);
    bool checkKeyWordInPos(int *pos, QString text, QString key);
    void parse(QString response);
	void messageCreate(QString msg);
	void playSound(QUrl src);

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
