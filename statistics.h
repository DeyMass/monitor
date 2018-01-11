#ifndef STATISTICS_H
#define STATISTICS_H

#include <QWidget>
#include <QDebug>
#include <QTime>

namespace Ui {


class Statistics;
}

class Statistics : public QWidget
{
	Q_OBJECT

public:
	explicit Statistics(QWidget *parent = 0);
	~Statistics();

private slots:
	void on_moduleName_currentIndexChanged(const QString &arg1);

	void on_pushButton_clicked();

private:
	Ui::Statistics *ui;

	QTime parseTime(QString str);
	QDate parseDate(QString str);
	QString parseState(QString str);
};

#endif // STATISTICS_H
