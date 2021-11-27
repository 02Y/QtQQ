#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class SysTray : public QSystemTrayIcon
{
	Q_OBJECT

public:
	SysTray(QWidget *parent);
	~SysTray();

public slots:
	void onIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
	void initSystemTray();    //初始化系统托盘
	void addSystrayMenu();

private:
	QWidget* m_parent;
};
