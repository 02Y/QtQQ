#include "TalkWindowShell.h"
#include "CommonUtils.h"
#include <QListWidget>
#include "TalkWindow.h"
#include "EmotionWindow.h"
#include "TalkWindowItem.h"
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QFile>
#include <QSqlQuery>
#include "WindowManager.h"
#include "ReceiveFile.h"

const int gtcpPort = 8888;
const int gudpPort = 6666;

extern QString gLoginEmployeeID;
QString gfileName;    //�ļ�����
QString gfileData;    //�ļ�����

TalkWindowShell::TalkWindowShell(QWidget* parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);			// ���ڹر�ʱ��������Դ����
	initControl();
	initTcpSocket();
	initUdpSocket();

	QFile file(":/Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (!file.size())
	{
		QStringList employeesIDList;
		getEmployeesID(employeesIDList);
		if (!createJsFile(employeesIDList))
		{
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����js�ļ�����ʧ��"));
		}
	}
	
}

TalkWindowShell::~TalkWindowShell()
{
	delete m_emotionWindow;
	m_emotionWindow = nullptr;
}

void TalkWindowShell::addTalkWindow(TalkWindow* talkWindow, TalkWindowItem* talkWindowItem, const QString uid/*, GroupType groupType*/)
{
	ui.rightStackedWidget->addWidget(talkWindow);

	//connect(m_emotionWindow, SIGNAL(signalEmotionWindowHide()), talkWindow, SLOT(onSetEmotionBtnStatus()));		

	QListWidgetItem* aItem = new QListWidgetItem(ui.listWidget);

	// ������������뵽ӳ��������
	// һ���Ӧһ�����촰��
	m_talkwindowItemMap.insert(aItem, talkWindow);

	aItem->setSelected(true);		// ��ѡ��

	//�ж���Ⱥ�Ļ��ǵ���
	QSqlQueryModel sqlDepModel;
	QString	strQuery = QString("SELECT picture FROM tab_department WHERE departmentID = %1").arg(uid);
	sqlDepModel.setQuery(strQuery);
	int rows = sqlDepModel.rowCount();

	if (rows == 0)  //����
	{
		strQuery = QString("SELECT picture FROM tab_employees WHERE employeeID = %1").arg(uid);
		sqlDepModel.setQuery(strQuery);
	}
	QModelIndex index;
	index = sqlDepModel.index(0, 0);   //�У���
	QImage img;
	img.load(sqlDepModel.data(index).toString());
	talkWindowItem->setHeadPixmap(QPixmap::fromImage(img));    // ����ͷ��

	ui.listWidget->addItem(aItem);
	ui.listWidget->setItemWidget(aItem, talkWindowItem);

	onTalkWindowItemClicked(aItem);

	// ��Ҫ������źŷǳ��࣬���ڹر�֮��Ҫ������Դ����
	// 1�����촰��Ҫ�رգ�2����ǰ������Ҫ�Ƴ���3��talkWindow ��ԴҪ�ͷ�
	// 4���ұ߶�ҳ�����ʾ��ҲҪɾ��
	connect(talkWindowItem, &TalkWindowItem::signalCloseClicked, [talkWindowItem, talkWindow, aItem, this]()
	{
			// �ȴ�����߲���
		m_talkwindowItemMap.remove(aItem);		// ɾ����
		talkWindow->close();									// ���촰�ڹر�
		// ͨ��takeItem�����Ƴ����������кţ�
		// ����row�������� aItem ���ȥ
		ui.listWidget->takeItem(ui.listWidget->row(aItem));
		delete talkWindowItem;

		// �ٴ����ұ߲���
		ui.rightStackedWidget->removeWidget(talkWindow);

		// ��һ���жϣ����ɾ���������һ������
		// �ͽ����ڹر�
		if (ui.rightStackedWidget->count() < 1)
		{
			close();
		}
	});
}

void TalkWindowShell::setCurrentWidget(QWidget* widget)
{
	ui.rightStackedWidget->setCurrentWidget(widget);
}

const QMap<QListWidgetItem*, QWidget*>& TalkWindowShell::getTalkWindowItemMap() const
{
	// ֱ�ӽ�ӳ����з���
	return m_talkwindowItemMap;
}

//�ı����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ���� + ���ݳ��� + ���ݣ�hello��
//�������ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ���� + ������� + images + ����
//msgType 0-������Ϣ 1-�ı���Ϣ 2-�ļ���Ϣ
void TalkWindowShell::updateSendTcpMsg(QString& strData, int& msgType, QString fileName)
{
	//��ȡ��ǰ����촰��
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*> (ui.rightStackedWidget->currentWidget());
	QString talkID = curTalkWindow->getTalkId();

	QString strGroupFlag;
	QString strSend;
	if (talkID.length() == 4)    //ȺQQ�ų���
	{
		strGroupFlag = "1";
	}
	else
	{
		strGroupFlag = "0";
	}

	int nstrDataLength = strData.length();//����� 10
	int dataLength = QString::number(nstrDataLength).length();    //������"10", ����dataLength = 2
	//const int sourceDataLength = dataLength;     
	QString strdataLength;

	if (msgType == 1)   //�ı���Ϣ
	{
		//�ı���Ϣ�ĳ���Լ��Ϊ5λ  "hello" 5���ַ�ռ1λ      50���ַ�ռ2λ
		if (dataLength == 1)
		{
			strdataLength = "0000" + QString::number(nstrDataLength);
		}
		else if(dataLength == 2)
		{
			strdataLength = "000" + QString::number(nstrDataLength);
		}
		else if (dataLength == 3)
		{
			strdataLength = "00" + QString::number(nstrDataLength);
		}
		else if (dataLength == 4)
		{
			strdataLength = "0" + QString::number(nstrDataLength);
		}
		else if (dataLength == 5)
		{
			strdataLength = QString::number(nstrDataLength);
		}
		else
		{
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����������ݳ��ȣ�"));
		}

		//�ı����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ���� + ���ݳ��� + ���ݣ�hello��
		strSend = strGroupFlag + gLoginEmployeeID + talkID + "1" + strdataLength + strData;
	}
	else if (msgType == 0)   //������Ϣ
	{
		//�������ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ���� + ������� + images + ��������
		strSend = strGroupFlag + gLoginEmployeeID + talkID + "0" + strData;
	}
	else if (msgType == 2)   //�ļ���Ϣ
	{
		//�ļ����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ����(2) + �ļ����� + "bytes" + �ļ����� + \
		"data_begin" + �ļ�����
		QByteArray bt = strData.toUtf8();
		QString strLength = QString::number(bt.length());
		strSend = strGroupFlag + gLoginEmployeeID + talkID + "2" + strLength + "bytes" + fileName + "data_begin" + strData;
	}

	QByteArray dataBt;
	dataBt.resize(strSend.length());
	dataBt = strSend.toUtf8();
	m_tcpClientSocket->write(dataBt);
}

void TalkWindowShell::initControl()
{
	loadStyleSheet("TalkWindow");
	setWindowTitle(QString::fromLocal8Bit("���촰��1"));

	m_emotionWindow = new EmotionWindow;
	m_emotionWindow->hide();				// ���ر��鴰��

	QList<int> leftWidgetSize;					// �ߴ�
	// ��߿�ȣ��ұ߿�ȣ�width() ������ܿ��
	leftWidgetSize << 154 << width() - 154;
	ui.splitter->setSizes(leftWidgetSize);			// ���������óߴ�

	ui.listWidget->setStyle(new CustomProxyStyle(this));		// �����Զ�����

	connect(ui.listWidget, &QListWidget::itemClicked, this, &TalkWindowShell::onTalkWindowItemClicked);
	connect(m_emotionWindow, SIGNAL(signalEmotionItemClicked(int)), this, SLOT(onEmotionItemClicked(int)));

}

void TalkWindowShell::initTcpSocket()
{
	m_tcpClientSocket = new QTcpSocket(this);
	m_tcpClientSocket->connectToHost("127.0.0.1", gtcpPort);
}

void TalkWindowShell::initUdpSocket()
{
	m_udpReceiver = new QUdpSocket(this);

	for (quint16 port = gudpPort; port < gudpPort + 200; ++port)      //��Ҫ�����Ż�����Щ�˷���Դ
	{
		if (m_udpReceiver->bind(port, QUdpSocket::ShareAddress)) break;    //�󶨶˿�
	}

	connect(m_udpReceiver, &QUdpSocket::readyRead, this, &TalkWindowShell::processPendingData);
}

void TalkWindowShell::getEmployeesID(QStringList& employeesIDList)
{
	QSqlQueryModel queryModel;
	queryModel.setQuery(QString("SELECT employeeID FROM tab_employees WHERE status = 1"));

	int employeesNum = queryModel.rowCount();       //���м�¼����������������Ա��������
	QModelIndex index;

	for (int i = 0; i < employeesNum; ++i)
	{
		index = queryModel.index(i, 0);    //�У���
		employeesIDList << queryModel.data(index).toString();
	}
}

bool TalkWindowShell::createJsFile(QStringList& employeesList)
{
	//��ȡtxt�ļ�����
	QString strFileTxt = ":/Resources/MainWindow/MsgHtml/msgtmpl.txt";
	QFile fileRead(strFileTxt);
	QString strFile;

	if (fileRead.open(QIODevice::ReadOnly))
	{
		strFile = fileRead.readAll();
		fileRead.close();
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("��ȡ msgtmpl.txt ʧ�ܣ�"));
		return false;
	}

	//�滻
	//ע��external0��appendHtml0 �����Լ�����Ϣʹ�ã���external�滻Ϊ�����û�������
	QFile fileWrite("./Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (fileWrite.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		//����msgtmpl.txt�еĲ���js����
		QString strSourceInitNull = "var external = null;";//������ʼ������
		QString strSourceInit = "external = channel.objects.external;";//����������1
		QString strSourceNew =
			"new QWebChannel(qt.webChannelTransport,\
				function(channel) {\
					external0 = channel.objects.external0;\
					external = channel.objects.external;\
				}\
			);\
			";//����������2

		//����recvHtml�����ı�
		QString strSourceRecvHtml;
		QFile fileRecvHtml(":/Resources/MainWindow/MsgHtml/recvHtml.txt");
		if (fileRecvHtml.open(QIODevice::ReadOnly))
		{
			strSourceRecvHtml = fileRecvHtml.readAll();
			fileRecvHtml.close();
		}
		else
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("��ȡ recvHtml.txt ʧ�ܣ�"));
			return false;
		}

		//�����滻��Ľű�
		QString strReplaceInitNull;
		QString strReplaceInit;
		QString strReplaceNew;
		QString strReplaceRecvHtml;

		for (int i = 0; i < employeesList.length(); i++)
		{
			//�༭�滻��Ŀ�ֵ
			QString strInitNull = strSourceInitNull;
			strInitNull.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInitNull += strInitNull;
			strReplaceInitNull += "\n";

			//�༭�滻��ĳ�ʼֵ
			QString strInit = strSourceInit;
			strInit.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInit += strInit;
			strReplaceInit += "\n";

			//�༭�滻���newWebChannel
			QString strNew = strSourceNew;
			strNew.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceNew += strNew;
			strReplaceNew += "\n";

			//�༭�滻���recvHtml
			QString strRecvHtml = strSourceRecvHtml;
			strRecvHtml.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strRecvHtml.replace("recvHtml", QString("recvHtml_%1").arg(employeesList.at(i)));
			strReplaceRecvHtml += strRecvHtml;
			strReplaceRecvHtml += "\n";
		}

		strFile.replace(strSourceInitNull, strReplaceInitNull);
		strFile.replace(strSourceInit, strReplaceInit);
		strFile.replace(strSourceNew, strReplaceNew);
		strFile.replace(strSourceRecvHtml, strReplaceRecvHtml);

		QTextStream stream(&fileWrite);
		stream << strFile;
		fileWrite.close();

		return true;
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("д msgtmp1.js ʧ�ܣ�"));
		return false;
	}
}

void TalkWindowShell::handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg)
{
	QMsgTextEdit msgTextEdit;
	msgTextEdit.setText(strMsg);

	if (msgType == 1)   //�ı���Ϣ
	{
		msgTextEdit.document()->toHtml();
	}
	else if (msgType == 0)   //������Ϣ
	{
		const int emotionWidth = 3;
		int emotionNum = strMsg.length() / emotionWidth;
		for (int i = 0; i < emotionNum; i++)
		{
			msgTextEdit.addEmotionUrl(strMsg.mid(i * emotionWidth, emotionWidth).toInt());
		}
	}

	QString html = msgTextEdit.document()->toHtml();

	if (html.contains("&quot;"))
	{
		html.replace("&quot;", "\"");
	}

	//�ı�html���û���������������	    
	if (!html.contains(".png") && !html.contains("</span>"))     //������".png"���ı�
	{
		QString fontHtml;
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			fontHtml.replace("%1", strMsg);    //�滻�ı�
			file.close();
		}
		else
		{
			// this����ǰ���첿������Ϊ������
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("�ļ� msgFont.txt �����ڣ�"));
			return;
		}

		// �ж�ת������û�а��� fontHtml
		if (!html.contains(fontHtml) && !CommonUtils::IsDigitString(strMsg) && !CommonUtils::IsEnglish(strMsg))
		{
			html.replace(strMsg, fontHtml);
		}
		else
		{
			if (CommonUtils::IsDigitString(strMsg))
			{
				int pos = html.lastIndexOf(strMsg);
				html.replace(pos, strMsg.size(), fontHtml);
			}
			else if(CommonUtils::IsEnglish(strMsg))
			{
				html.replace(430, strMsg.size(), fontHtml);
			}
		}
	}

	TalkWindow* talkWindow = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());
	talkWindow->ui.msgWidget->appendMsg(html, QString::number(senderEmployeeID));
}

void TalkWindowShell::onEmotionBtnClicked(bool)
{
	// ���鴰�ڣ����֮��Ҫ��������
	// ���óɡ����ɼ���
	m_emotionWindow->setVisible(!m_emotionWindow->isVisible());

	// ����ǰ�ؼ������λ�ã�ת��Ϊ��Ļ�ľ���λ��
	QPoint emotionPoint = this->mapToGlobal(QPoint(0, 0));

	emotionPoint.setX(emotionPoint.x() + 170);
	emotionPoint.setY(emotionPoint.y() + 220);
	m_emotionWindow->move(emotionPoint);			// �ƶ�

}

void TalkWindowShell::onTalkWindowItemClicked(QListWidgetItem* item)
{
	// ȡ�ô����ַ
	QWidget* talkWindowWidget = m_talkwindowItemMap.find(item).value();

	// ���ұߵĴ������
	ui.rightStackedWidget->setCurrentWidget(talkWindowWidget);
}

void TalkWindowShell::onEmotionItemClicked(int emotionNum)
{
	// ���ص�ָ�������� Widget�������ͣ���Ҫת���� TalkWindow* ָ������
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());
	if (curTalkWindow)
	{
		// �����촰�ڣ����ͼƬ
		// ������Ŵ���ȥ�����м���
		curTalkWindow->addEmotionImage(emotionNum);
	}
}


/*                   *******����̫�˷��ֽ��������Ż�    ����������Ҫ�Ż���������Ϣ+����һ�𷢣�*******
	���ݰ��ĸ�ʽ
	�ı����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ����(1) + ���ݳ��� + ���ݣ�hello��
	�������ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ����(0) + ������� + images + ��������
	�ļ����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ�+ ��Ϣ����(2) + �ļ����� + "bytes" + �ļ����� + \
				 "data_begin" + �ļ�����

	Ⱥ�ı�־ռ1λ��0��ʾ���ģ�1��ʾȺ��
	��Ϣ����ռ1λ��0��ʾ������Ϣ��1��ʾ�ı���Ϣ��2��ʾ�ļ���Ϣ

	QQ��ռ5λ��ȺQQ��ռ4λ�����ݳ���ռ5λ��5000���ֽڳ�����4λ������������ռ3λ
	   ��ע�⣺��Ⱥ�ı�־Ϊ1ʱ�������ݰ�û������ϢԱ��QQ�ţ���������ϢȺQQ��
	   ����������Ⱥ�ı�־Ϊ��ʱ�������ݰ�û������ϢȺQQ�ţ���������ϢԱ��QQ�ţ�

	Ⱥ���ı���Ϣ�磺1100012001100005hello   ��ʾQQ-10001��Ⱥ-2001�����ı���Ϣ�����ݳ���Ϊ00005������Ϊhello
	����ͼƬ��Ϣ�磺0100091000201images060         
	Ⱥ���ļ���Ϣ�磺1100052000210bytestest.textdata_beginhelloworld

	Ⱥ���ļ���Ϣ������1 10001 2001 1 00005 hello
	����ͼƬ��Ϣ������0 10009 10002 0 1 images 60
*/
void TalkWindowShell::processPendingData()
{
	while (m_udpReceiver->hasPendingDatagrams())      //�˿����Ƿ���δ���������
	{
		const static int groupFlagWidth = 1;     //Ⱥ�ı�־ռλ
		const static int groupWidth = 4;         //ȺQQ�ſ��
		const static int employeeWidth = 5;      //Ա��QQ�ſ��
		const static int msgTypeWidth = 1;       //��Ϣ���Ϳ��
		const static int msgLengthWidth = 5;     //�ı���Ϣ���ȵĿ��
		const static int pictureWidth = 3;       //����ͼƬ�Ŀ��

		QByteArray btData;
		btData.resize(m_udpReceiver->pendingDatagramSize());//δ�������ݴ�С
		m_udpReceiver->readDatagram(btData.data(), btData.size());      //��ȡudp����

		QString strData = btData.data();
		QString strWindowID;    //���촰��ID��Ⱥ����Ⱥ�ţ�������Ա��QQ��
		QString strSendEmployeeID, strReceiveEmployeeID;   //���ͼ����ܶ˵�Ա��QQ��
		QString strMsg;   //����
		int msgLen;       //���ݳ���
		int msgType;      //��������

		strSendEmployeeID = strData.mid(groupFlagWidth, employeeWidth);    //���Ͷ�ID

		//�Լ�������Ϣ��������
		if (strSendEmployeeID == gLoginEmployeeID) return;

		if (btData[0] == '1') //Ⱥ��
		{
			strWindowID = strData.mid(groupFlagWidth + employeeWidth, groupWidth);   //Ⱥ�Ĵ���ID��ȺQQ�ţ�

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + groupWidth];    //��ȡ��Ϣ����
			if (cMsgType == '1')   //�ı���Ϣ
			{
				msgType = 1;
				msgLen = strData.mid(groupFlagWidth + employeeWidth + groupWidth + msgTypeWidth, msgLengthWidth).toInt();
				strMsg = strData.mid(groupFlagWidth + employeeWidth + groupWidth + msgTypeWidth + msgLengthWidth, msgLen);
			}
			else if (cMsgType == '0')   //������Ϣ
			{
				msgType = 0;
				int posImages = strData.indexOf("images");    //�������ַ�����λ��
				strMsg = strData.right(strData.length() - posImages - QString("images").length());  //��ȡimages����ı�������
			}
			else if (cMsgType == '2')   //�ļ���Ϣ
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int posData_begin = strData.indexOf("data_begin");
				
				//�ļ�����
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);   
				gfileName = fileName;
				
				//�ļ�����
				int dataLengthWidth;
				int posData = posData_begin + QString("data_begin").length();
				strMsg = strData.mid(posData);
				gfileData = strMsg;

				//����employeeID��ȡ����������
				QString Sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1")
					.arg(employeeID));
				querySenderName.exec();
				if (querySenderName.first())
				{
					Sender = querySenderName.value(0).toString();
				}

				//�����ļ�
				ReceiveFile* recvFile = new ReceiveFile(this);
				connect(recvFile, &ReceiveFile::refuseFile, [this]() { return; });

				QString msgLabel = QString::fromLocal8Bit("�յ�����") + Sender + QString::fromLocal8Bit("�������ļ�, �Ƿ����");
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		else  //����
		{
			strReceiveEmployeeID = strData.mid(groupFlagWidth + employeeWidth, employeeWidth);
			strWindowID = strSendEmployeeID;

			//���Ƿ����ҵ���Ϣ��������
			if (strReceiveEmployeeID != gLoginEmployeeID)
			{
				return;
			}

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + employeeWidth];   //��ȡ��Ϣ����
			if (cMsgType == '1')  //�ı���Ϣ
			{
				msgType = 1;
				//�ı���Ϣ����
				msgLen = strData.mid(groupFlagWidth + employeeWidth + employeeWidth + msgTypeWidth, msgLengthWidth).toInt();

				strMsg = strData.mid(groupFlagWidth + employeeWidth + employeeWidth + msgTypeWidth + msgLengthWidth, msgLen);
			}
			else if (cMsgType == '0')   //������Ϣ
			{
				msgType = 0;
				int posImages = strData.indexOf("images");  //�������ַ�����λ��
				int imagesWidth = QString("images").length();
				strMsg = strData.mid(posImages + imagesWidth);
			}
			else if (cMsgType == '2')   //�ļ���Ϣ
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int data_beginWidth = QString("data_begin").length();
				int posData_begin = strData.indexOf("data_begin");

				//�ļ�����
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);
				gfileName = fileName;

				//�ļ�����
				strMsg = strData.mid(posData_begin + data_beginWidth);
				gfileData = strMsg;

				//����employeeID��ȡ����������
				QString Sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1")
					.arg(employeeID));
				querySenderName.exec();
				if (querySenderName.first())
				{
					Sender = querySenderName.value(0).toString();
				}

				//�����ļ�
				ReceiveFile* recvFile = new ReceiveFile(this);
				connect(recvFile, &ReceiveFile::refuseFile, [this]() { return; });

				QString msgLabel = QString::fromLocal8Bit("�յ�����") + Sender + QString::fromLocal8Bit("�������ļ�, �Ƿ����");
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}

		//�����촰����Ϊ��Ĵ���
		QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);
		if (widget)    //���촰�ڴ���
		{
			this->setCurrentWidget(widget);

			//ͬ������������촰��
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);      //��Ϊѡ��
		}
		else    //���촰��δ��
		{
			return; 
		}

		//�ļ���Ϣ��������
		if (msgType != 2)
		{
			int sendEmployeeID = strSendEmployeeID.toInt();
			handleReceivedMsg(sendEmployeeID, msgType, strMsg);
		}
	}
}
