#pragma once

#include <QtWidgets/QDialog>
#include "ui_QtQQ_Server.h"
#include "TcpServer.h"
#include <QSqlQueryModel>
#include <QTimer>
#include <QUdpSocket>

class QtQQ_Server : public QDialog
{
	Q_OBJECT

public:
	QtQQ_Server(QWidget *parent = Q_NULLPTR);

private:
	void initTcpSocket();   //��ʼ��Tcp
	void initUdpSocket();   //��ʼ��Udp
	bool connectMySql();
	int getComDepID();     //��ȡ��˾ȺQQ��
	void updateTableData(int depID = 0, int employeeID = 0);    //���±������
	void setDepNameMap();
	void setStatusMap();
	void setOnlineMap();
	void initComboBoxData();      //��ʼ����Ͽ�����

private slots:
	void onUDPbroadMsg(QByteArray& btData);
	void onRefresh();    //��ʱˢ������
	void on_queryDepartmentBtn_clicked();   //����ȺQQ�Ų���Ա��(����ź���ۺ����Զ�����)
	void on_queryIDBtn_clicked();           //����Ա��QQ�Ų���
	void on_logoutBtn_clicked();           //ע��Ա��QQ��
	void on_selectPictureBtn_clicked();    //����Ա��ʱѡ����Ƭ��ť
	void on_addBtn_clicked();     //�������Ա��

private:
	Ui::QtQQ_ServerClass ui;

	QTimer* m_timer;    //��ʱˢ������
	int m_comDepID;     //��˾ȺQQ��
	int m_depID;        //����QQ��
	int m_employeeID;   //Ա��QQ��
	QString m_pixPath;  //��Ƭ·��

	QMap<QString, QString> m_statusMap;     //״̬ӳ��
	QMap<QString, QString> m_depNameMap;    //��������ӳ��
	QMap<QString, QString> m_onlineMap;     //����״̬ӳ��

	QSqlQueryModel m_queryInfoModel;   //��ѯ����Ա������Ϣģ��
	TcpServer* m_tcpServer;      //tcp�����
	QUdpSocket* m_udpSender;      //udp�㲥
};
