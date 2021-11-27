#include "TcpSocket.h"

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::run()
{
	m_descriptor = this->socketDescriptor();//当前socket描述符

	connect(this, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
	connect(this, SIGNAL(disconnected()), this, SLOT(onClientDisconnect()));
}

void TcpSocket::onReceiveData()
{
	QByteArray buffer = this->readAll();
	if (!buffer.isEmpty())
	{
		QString strData = QString::fromLocal8Bit(buffer);

		//发射接受到了客户端数据的信号
		emit signalGetDataFromClient(buffer, m_descriptor);
	}
}

void TcpSocket::onClientDisconnect()
{
	emit signalClientDisconnect(m_descriptor);
}
