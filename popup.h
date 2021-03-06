#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QTimer>

class PopUp : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

	void setPopupOpacity(float opacity);
	float getPopupOpacity() const;

public:
	explicit PopUp(QWidget *parent = nullptr);
	void setDelay(int time);

protected:
	void paintEvent(QPaintEvent *event);    // Фон будет отрисовываться через метод перерисовки

public slots:
	void setTextStyle(QString style);
	void setPopupText(const QString& text); // Установка текста в уведомление
	void show();                            /* Собственный метод показа виджета
											 * Необходимо для преварительной настройки анимации
											 * */
private slots:
	void hideAnimation();                   // Слот для запуска анимации скрытия
	void hide();                            /* По окончании анимации, в данном слоте делается проверка,
											 * виден ли виджет, или его необходимо скрыть
											 * */
private:
	int	popupDelay;
	QLabel label;           // Label с сообщением
	QGridLayout layout;     // Размещение для лейбла
	QPropertyAnimation animation;   // Свойство анимации для всплывающего сообщения
	float popupOpacity;     // Свойства полупрозрачности виджета
	QTimer *timer;          // Таймер, по которому виджет будет скрыт
};

#endif // POPUP_H
