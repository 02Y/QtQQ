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
	COMPANY = 0,         //公司群
	PERSONELGROUP,       //人事部
	DEVELOPMENTGROUP,   //研发部
	MARKETGROUP,         //市场部
	PTOP                 //同事之间单独聊天
};

class TalkWindowShell : public BasicWindow
{
	Q_OBJECT

public:
	TalkWindowShell(QWidget* parent = Q_NULLPTR);
	~TalkWindowShell();

private:
	void initControl();       //初始化部件
	void initTcpSocket();     //初始化Tcp套接字
	void initUdpSocket();     //初始化Udp套接字
	void getEmployeesID(QStringList& employeesIDList);        //获取所有员工QQ号
	bool createJsFile(QStringList& employeesList);
	void handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg);   //解析后把信息添加到网页(处理文本+表情，文件单独处理)

public:
	void addTalkWindow(TalkWindow* talkWindow, TalkWindowItem* talkWindowItem, const QString uid/*, GroupType grouptype*/);       //添加新的聊天窗口
	void setCurrentWidget(QWidget* widget);            //设置当前聊天窗口
	const QMap<QListWidgetItem*, QWidget*>& getTalkWindowItemMap() const;

public slots:
	void onEmotionBtnClicked(bool);       //表情按钮被点击后执行的槽函数
	void updateSendTcpMsg(QString& strData, int& msgType, QString fileName = "");   //客户端发送TCP数据（数据，数据类型，文件）

private slots:
	void onTalkWindowItemClicked(QListWidgetItem* item);      //左侧列表点击后执行的槽函数（选择的哪一项窗口）
	void onEmotionItemClicked(int emotionNum);         //表情被选中
	void processPendingData();     //处理UDP广播收到的数据(解析)

private:
	Ui::TalkWindowClass ui;
	QMap<QListWidgetItem*, QWidget*> m_talkwindowItemMap;     //打开的聊天窗口
	EmotionWindow* m_emotionWindow;       //表情窗口

private:
	QTcpSocket* m_tcpClientSocket;    //TCP客户端
	QUdpSocket* m_udpReceiver;        //udp接收端
};
