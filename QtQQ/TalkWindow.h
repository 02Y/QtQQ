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
	void onFileOpenBtnClicked(bool);   //�ļ��򿪱������ 
	void onSendBtnClicked(bool);
	void onItemDoubleClicked(QTreeWidgetItem* item, int colunm);

private:
	void initControl();

	//void initCompany();          //��ʼ����˾Ⱥ����
	//void initPersonelTalk();     //��ʼ�����²�����
	//void initMarket();           //��ʼ���г�������
	//void initDevelopTalk();      //��ʼ���з�������

	void initGroupTalkStatus();   //��ʼ��Ⱥ��״̬(Ⱥ�� or ����)
	void initTalkWindow();        //��ʼ��Ⱥ��
	void initPtoPTalk();          //��ʼ������
	int getCompDepID();          //��ȡ��˾ȺID��

	void addPeopInfo(QTreeWidgetItem* pRootGroupItem, int empolyeeID);

private:
	Ui::TalkWindow ui;
	QString m_talkId;
	//GroupType m_groupType;
	bool m_isGroupTalk;    //�Ƿ�ΪȺ��
	QMap<QTreeWidgetItem*, QString> m_groupPeopleMap;    //���з�����ϵ������

	friend class TalkWindowShell;
};
