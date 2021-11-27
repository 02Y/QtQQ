#pragma once

#include <QObject>
#include "TalkWindowShell.h"

class WindowManager : public QObject
{
	Q_OBJECT

public:
	WindowManager();
	~WindowManager();

public:
	QWidget* findWindowName(const QString& qsWindowName);         //查找窗口名
	void deleteWindowName(const QString& qsWindowName);           //删除窗口名
	void addWindowName(const QString& qsWindowName, QWidget* qWidget);          //添加窗口名
	
	static WindowManager* getInstance();          //单例

	TalkWindowShell* getTalkWindowShell();
	
	void addNewTalkWindow(const QString& uid/*, GroupType groupType = COMPANY, const QString& strPeople = ""*/);   //添加新窗口
	QString getCreatingTalkID();

private:
	TalkWindowShell* m_talkwindowshell;
	QMap<QString, QWidget*> m_windowMap;
	QString m_strCreatingTalkID = "";    //正在创建的聊天窗口（QQ号）
};
