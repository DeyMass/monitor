#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <QString>

struct mySettings{
	//main
	int		updateDelay;
	//notice
	int		noticeFontSize;
	int		noticeTime;
	QString noticeFontColor;
	QString noticeFontFamily;
	//server
	int		destPort;
	QString destIp;
	QString destUrl;
	QString destHost;
};

#endif // STRUCTURES_H
