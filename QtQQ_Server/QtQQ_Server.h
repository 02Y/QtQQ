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
	void initTcpSocket();   //初始化Tcp
	void initUdpSocket();   //初始化Udp
	bool connectMySql();
	int getComDepID();     //获取公司群QQ号
	void updateTableData(int depID = 0, int employeeID = 0);    //更新表格数据
	void setDepNameMap();
	void setStatusMap();
	void setOnlineMap();
	void initComboBoxData();      //初始化组合框数据

private slots:
	void onUDPbroadMsg(QByteArray& btData);
	void onRefresh();    //定时刷新数据
	void on_queryDepartmentBtn_clicked();   //根据群QQ号查找员工(点击信号与槽函数自动连接)
	void on_queryIDBtn_clicked();           //根据员工QQ号查找
	void on_logoutBtn_clicked();           //注销员工QQ号
	void on_selectPictureBtn_clicked();    //新增员工时选择照片按钮
	void on_addBtn_clicked();     //添加新增员工

private:
	Ui::QtQQ_ServerClass ui;

	QTimer* m_timer;    //定时刷新数据
	int m_comDepID;     //公司群QQ号
	int m_depID;        //部门QQ号
	int m_employeeID;   //员工QQ号
	QString m_pixPath;  //照片路径

	QMap<QString, QString> m_statusMap;     //状态映射
	QMap<QString, QString> m_depNameMap;    //部门名称映射
	QMap<QString, QString> m_onlineMap;     //在线状态映射

	QSqlQueryModel m_queryInfoModel;   //查询所有员工的信息模型
	TcpServer* m_tcpServer;      //tcp服务端
	QUdpSocket* m_udpSender;      //udp广播
};
