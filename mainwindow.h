#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QSqlRecord>
#include <QSqlTableModel>

#define BORDER 70
#define OR ||

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
    QTcpSocket *request;
    Ui::MainWindow *ui;
    QString readName(int *pos, QString response);
    QString readStatus(int *pos, QString response);
    void skipData(int *pos, QString response);
    bool checkKeyWordInPos(int *pos, QString text, QString key);
    void parse(QString response);

private slots:
    void reconnect();
    void connected();
    void readyRead();
    void disconnected();
    void refresh();
};

#endif // MAINWINDOW_H
