#pragma once

#include <QTcpServer>

class TcpServer : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();

public:
	bool run();   //�����˿���û������

protected:
	//�ͻ������µ�����ʱ
	void incomingConnection(qintptr socketDescriptor);   //void incomingConnection(int socketDescriptor) �ϰ汾д��

signals:
	void signalTcpMsgComes(QByteArray&);

private slots:
	//��������
	void SocketDataProcessing(QByteArray& SendData, int descriptor);
	//�Ͽ����Ӵ���
	void SocketDisconnected(int descriptor);

private:
	int m_port;   //�˿ں�
	QList<QTcpSocket*> m_tcpSocketConnectList;
};
