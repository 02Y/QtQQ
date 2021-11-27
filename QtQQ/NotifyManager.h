#pragma once

#include <QObject>

class NotifyManager : public QObject
{
	Q_OBJECT

public:
	NotifyManager();
	~NotifyManager();

signals:
	void signalSkinChanged(const QColor& color);    //改变颜色信号

public:
	static NotifyManager* getInstance();          //获取实例   单例

	void notifyOtherWindowChangeSkin(const QColor& color);       //通知其他窗口进行皮肤颜色的改变

private:
	static NotifyManager* instance;
};
