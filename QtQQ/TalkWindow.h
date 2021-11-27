#pragma once

#include <QWidget>
#include "ui_TalkWindow.h"
#include "TalkWindowShell.h"


class TalkWindow : public QWidget
{
	Q_OBJECT

public:
	TalkWindow(QWidget *parent, const QString& uid/*, GroupType groupType*/);
	~TalkWindow();

public:
	void addEmotionImage(int emotionNum);
	void setWindowName(const QString& name);
	QString getTalkId();

private slots:
	void onFileOpenBtnClicked(bool);   //文件打开被点击了 
	void onSendBtnClicked(bool);
	void onItemDoubleClicked(QTreeWidgetItem* item, int colunm);

private:
	void initControl();

	//void initCompany();          //初始化公司群聊天
	//void initPersonelTalk();     //初始化人事部聊天
	//void initMarket();           //初始化市场部聊天
	//void initDevelopTalk();      //初始化研发部聊天

	void initGroupTalkStatus();   //初始化群聊状态(群聊 or 单聊)
	void initTalkWindow();        //初始化群聊
	void initPtoPTalk();          //初始化单聊
	int getCompDepID();          //获取公司群ID号

	void addPeopInfo(QTreeWidgetItem* pRootGroupItem, int empolyeeID);

private:
	Ui::TalkWindow ui;
	QString m_talkId;
	//GroupType m_groupType;
	bool m_isGroupTalk;    //是否为群聊
	QMap<QTreeWidgetItem*, QString> m_groupPeopleMap;    //所有分组联系人姓名

	friend class TalkWindowShell;
};
