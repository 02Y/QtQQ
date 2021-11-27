#pragma once

#include <QWidget>
#include "ui_TalkWindowShell.h"
#include "BasicWindow.h"
#include <QMap>
#include <QTcpSocket>
#include <QUdpSocket>

class TalkWindow;
class TalkWindowItem;
class QListWidgetItem;
class EmotionWindow;

enum GroupType
{
	COMPANY = 0,         //��˾Ⱥ
	PERSONELGROUP,       //���²�
	DEVELOPMENTGROUP,   //�з���
	MARKETGROUP,         //�г���
	PTOP                 //ͬ��֮�䵥������
};

class TalkWindowShell : public BasicWindow
{
	Q_OBJECT

public:
	TalkWindowShell(QWidget* parent = Q_NULLPTR);
	~TalkWindowShell();

private:
	void initControl();       //��ʼ������
	void initTcpSocket();     //��ʼ��Tcp�׽���
	void initUdpSocket();     //��ʼ��Udp�׽���
	void getEmployeesID(QStringList& employeesIDList);        //��ȡ����Ա��QQ��
	bool createJsFile(QStringList& employeesList);
	void handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg);   //���������Ϣ��ӵ���ҳ(�����ı�+���飬�ļ���������)

public:
	void addTalkWindow(TalkWindow* talkWindow, TalkWindowItem* talkWindowItem, const QString uid/*, GroupType grouptype*/);       //����µ����촰��
	void setCurrentWidget(QWidget* widget);            //���õ�ǰ���촰��
	const QMap<QListWidgetItem*, QWidget*>& getTalkWindowItemMap() const;

public slots:
	void onEmotionBtnClicked(bool);       //���鰴ť�������ִ�еĲۺ���
	void updateSendTcpMsg(QString& strData, int& msgType, QString fileName = "");   //�ͻ��˷���TCP���ݣ����ݣ��������ͣ��ļ���

private slots:
	void onTalkWindowItemClicked(QListWidgetItem* item);      //����б�����ִ�еĲۺ�����ѡ�����һ��ڣ�
	void onEmotionItemClicked(int emotionNum);         //���鱻ѡ��
	void processPendingData();     //����UDP�㲥�յ�������(����)

private:
	Ui::TalkWindowClass ui;
	QMap<QListWidgetItem*, QWidget*> m_talkwindowItemMap;     //�򿪵����촰��
	EmotionWindow* m_emotionWindow;       //���鴰��

private:
	QTcpSocket* m_tcpClientSocket;    //TCP�ͻ���
	QUdpSocket* m_udpReceiver;        //udp���ն�
};
