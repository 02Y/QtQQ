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
QString gfileName;    //文件名称
QString gfileData;    //文件内容

TalkWindowShell::TalkWindowShell(QWidget* parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);			// 窗口关闭时，进行资源回收
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
			QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("更新js文件数据失败"));
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

	// 把新聊天项，插入到映射链表中
	// 一项对应一个聊天窗口
	m_talkwindowItemMap.insert(aItem, talkWindow);

	aItem->setSelected(true);		// 被选中

	//判断是群聊还是单聊
	QSqlQueryModel sqlDepModel;
	QString	strQuery = QString("SELECT picture FROM tab_department WHERE departmentID = %1").arg(uid);
	sqlDepModel.setQuery(strQuery);
	int rows = sqlDepModel.rowCount();

	if (rows == 0)  //单聊
	{
		strQuery = QString("SELECT picture FROM tab_employees WHERE employeeID = %1").arg(uid);
		sqlDepModel.setQuery(strQuery);
	}
	QModelIndex index;
	index = sqlDepModel.index(0, 0);   //行，列
	QImage img;
	img.load(sqlDepModel.data(index).toString());
	talkWindowItem->setHeadPixmap(QPixmap::fromImage(img));    // 设置头像

	ui.listWidget->addItem(aItem);
	ui.listWidget->setItemWidget(aItem, talkWindowItem);

	onTalkWindowItemClicked(aItem);

	// 需要捕获的信号非常多，窗口关闭之后，要进行资源回收
	// 1）聊天窗口要关闭，2）当前聊天项要移除，3）talkWindow 资源要释放
	// 4）右边多页面的显示，也要删除
	connect(talkWindowItem, &TalkWindowItem::signalCloseClicked, [talkWindowItem, talkWindow, aItem, this]()
	{
			// 先处理左边部分
		m_talkwindowItemMap.remove(aItem);		// 删除项
		talkWindow->close();									// 聊天窗口关闭
		// 通过takeItem方法移除，参数是行号，
		// 调用row方法，将 aItem 项传进去
		ui.listWidget->takeItem(ui.listWidget->row(aItem));
		delete talkWindowItem;

		// 再处理右边部分
		ui.rightStackedWidget->removeWidget(talkWindow);

		// 做一个判断，如果删除的是最后一个窗口
		// 就将窗口关闭
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
	// 直接将映射进行返回
	return m_talkwindowItemMap;
}

//文本数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型 + 数据长度 + 数据（hello）
//表情数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型 + 表情个数 + images + 数据
//msgType 0-表情信息 1-文本信息 2-文件信息
void TalkWindowShell::updateSendTcpMsg(QString& strData, int& msgType, QString fileName)
{
	//获取当前活动聊天窗口
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*> (ui.rightStackedWidget->currentWidget());
	QString talkID = curTalkWindow->getTalkId();

	QString strGroupFlag;
	QString strSend;
	if (talkID.length() == 4)    //群QQ号长度
	{
		strGroupFlag = "1";
	}
	else
	{
		strGroupFlag = "0";
	}

	int nstrDataLength = strData.length();//如果是 10
	int dataLength = QString::number(nstrDataLength).length();    //这里是"10", 所以dataLength = 2
	//const int sourceDataLength = dataLength;     
	QString strdataLength;

	if (msgType == 1)   //文本信息
	{
		//文本信息的长度约定为5位  "hello" 5个字符占1位      50个字符占2位
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
			QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("不合理的数据长度！"));
		}

		//文本数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型 + 数据长度 + 数据（hello）
		strSend = strGroupFlag + gLoginEmployeeID + talkID + "1" + strdataLength + strData;
	}
	else if (msgType == 0)   //表情信息
	{
		//表情数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型 + 表情个数 + images + 表情名称
		strSend = strGroupFlag + gLoginEmployeeID + talkID + "0" + strData;
	}
	else if (msgType == 2)   //文件信息
	{
		//文件数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型(2) + 文件长度 + "bytes" + 文件名称 + \
		"data_begin" + 文件内容
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
	setWindowTitle(QString::fromLocal8Bit("聊天窗口1"));

	m_emotionWindow = new EmotionWindow;
	m_emotionWindow->hide();				// 隐藏表情窗口

	QList<int> leftWidgetSize;					// 尺寸
	// 左边宽度，右边宽度，width() 窗体的总宽度
	leftWidgetSize << 154 << width() - 154;
	ui.splitter->setSizes(leftWidgetSize);			// 分类器设置尺寸

	ui.listWidget->setStyle(new CustomProxyStyle(this));		// 设置自定义风格

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

	for (quint16 port = gudpPort; port < gudpPort + 200; ++port)      //需要进行优化，有些浪费资源
	{
		if (m_udpReceiver->bind(port, QUdpSocket::ShareAddress)) break;    //绑定端口
	}

	connect(m_udpReceiver, &QUdpSocket::readyRead, this, &TalkWindowShell::processPendingData);
}

void TalkWindowShell::getEmployeesID(QStringList& employeesIDList)
{
	QSqlQueryModel queryModel;
	queryModel.setQuery(QString("SELECT employeeID FROM tab_employees WHERE status = 1"));

	int employeesNum = queryModel.rowCount();       //所有记录的数量（总行数：员工总数）
	QModelIndex index;

	for (int i = 0; i < employeesNum; ++i)
	{
		index = queryModel.index(i, 0);    //行，列
		employeesIDList << queryModel.data(index).toString();
	}
}

bool TalkWindowShell::createJsFile(QStringList& employeesList)
{
	//读取txt文件数据
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
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("读取 msgtmpl.txt 失败！"));
		return false;
	}

	//替换
	//注：external0、appendHtml0 用于自己发信息使用，把external替换为其他用户的内容
	QFile fileWrite("./Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (fileWrite.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		//处理msgtmpl.txt中的部分js代码
		QString strSourceInitNull = "var external = null;";//变量初始化部分
		QString strSourceInit = "external = channel.objects.external;";//处理函数部分1
		QString strSourceNew =
			"new QWebChannel(qt.webChannelTransport,\
				function(channel) {\
					external0 = channel.objects.external0;\
					external = channel.objects.external;\
				}\
			);\
			";//处理函数部分2

		//处理recvHtml代码文本
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
				QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("读取 recvHtml.txt 失败！"));
			return false;
		}

		//保存替换后的脚本
		QString strReplaceInitNull;
		QString strReplaceInit;
		QString strReplaceNew;
		QString strReplaceRecvHtml;

		for (int i = 0; i < employeesList.length(); i++)
		{
			//编辑替换后的空值
			QString strInitNull = strSourceInitNull;
			strInitNull.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInitNull += strInitNull;
			strReplaceInitNull += "\n";

			//编辑替换后的初始值
			QString strInit = strSourceInit;
			strInit.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInit += strInit;
			strReplaceInit += "\n";

			//编辑替换后的newWebChannel
			QString strNew = strSourceNew;
			strNew.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceNew += strNew;
			strReplaceNew += "\n";

			//编辑替换后的recvHtml
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
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("写 msgtmp1.js 失败！"));
		return false;
	}
}

void TalkWindowShell::handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg)
{
	QMsgTextEdit msgTextEdit;
	msgTextEdit.setText(strMsg);

	if (msgType == 1)   //文本信息
	{
		msgTextEdit.document()->toHtml();
	}
	else if (msgType == 0)   //表情信息
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

	//文本html如果没有字体则添加字体	    
	if (!html.contains(".png") && !html.contains("</span>"))     //不包含".png"是文本
	{
		QString fontHtml;
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			fontHtml.replace("%1", strMsg);    //替换文本
			file.close();
		}
		else
		{
			// this，当前聊天部件，作为父窗体
			QMessageBox::information(this, QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("文件 msgFont.txt 不存在！"));
			return;
		}

		// 判断转换后，有没有包含 fontHtml
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
	// 表情窗口，点击之后，要隐藏起来
	// 设置成【不可见】
	m_emotionWindow->setVisible(!m_emotionWindow->isVisible());

	// 将当前控件的相对位置，转换为屏幕的绝对位置
	QPoint emotionPoint = this->mapToGlobal(QPoint(0, 0));

	emotionPoint.setX(emotionPoint.x() + 170);
	emotionPoint.setY(emotionPoint.y() + 220);
	m_emotionWindow->move(emotionPoint);			// 移动

}

void TalkWindowShell::onTalkWindowItemClicked(QListWidgetItem* item)
{
	// 取得窗体地址
	QWidget* talkWindowWidget = m_talkwindowItemMap.find(item).value();

	// 将右边的窗体更新
	ui.rightStackedWidget->setCurrentWidget(talkWindowWidget);
}

void TalkWindowShell::onEmotionItemClicked(int emotionNum)
{
	// 返回的指针类型是 Widget部件类型，需要转换成 TalkWindow* 指针类型
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());
	if (curTalkWindow)
	{
		// 对聊天窗口，添加图片
		// 表情序号传进去，进行加载
		curTalkWindow->addEmotionImage(emotionNum);
	}
}


/*                   *******规则太浪费字节数，需优化    解析规则需要优化（不能信息+表情一起发）*******
	数据包的格式
	文本数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型(1) + 数据长度 + 数据（hello）
	表情数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型(0) + 表情个数 + images + 表情名称
	文件数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号）+ 信息类型(2) + 文件长度 + "bytes" + 文件名称 + \
				 "data_begin" + 文件内容

	群聊标志占1位，0表示单聊，1表示群聊
	信息类型占1位，0表示表情信息，1表示文本信息，2表示文件信息

	QQ号占5位，群QQ号占4位，数据长度占5位（5000个字节长度是4位），表情名称占3位
	   （注意：当群聊标志为1时，则数据包没有收信息员工QQ号，而是收信息群QQ号
	   　　　　当群聊标志为０时，则数据包没有收信息群QQ号，而是收信息员工QQ号）

	群聊文本信息如：1100012001100005hello   表示QQ-10001向群-2001发送文本信息，数据长度为00005，内容为hello
	单聊图片信息如：0100091000201images060         
	群聊文件信息如：1100052000210bytestest.textdata_beginhelloworld

	群聊文件信息解析：1 10001 2001 1 00005 hello
	单聊图片信息解析：0 10009 10002 0 1 images 60
*/
void TalkWindowShell::processPendingData()
{
	while (m_udpReceiver->hasPendingDatagrams())      //端口种是否有未处理的数据
	{
		const static int groupFlagWidth = 1;     //群聊标志占位
		const static int groupWidth = 4;         //群QQ号宽度
		const static int employeeWidth = 5;      //员工QQ号宽度
		const static int msgTypeWidth = 1;       //信息类型宽度
		const static int msgLengthWidth = 5;     //文本信息长度的宽度
		const static int pictureWidth = 3;       //表情图片的宽度

		QByteArray btData;
		btData.resize(m_udpReceiver->pendingDatagramSize());//未处理数据大小
		m_udpReceiver->readDatagram(btData.data(), btData.size());      //读取udp数据

		QString strData = btData.data();
		QString strWindowID;    //聊天窗口ID，群聊是群号，单聊是员工QQ号
		QString strSendEmployeeID, strReceiveEmployeeID;   //发送及接受端的员工QQ号
		QString strMsg;   //数据
		int msgLen;       //数据长度
		int msgType;      //数据类型

		strSendEmployeeID = strData.mid(groupFlagWidth, employeeWidth);    //发送端ID

		//自己发的信息不做处理
		if (strSendEmployeeID == gLoginEmployeeID) return;

		if (btData[0] == '1') //群聊
		{
			strWindowID = strData.mid(groupFlagWidth + employeeWidth, groupWidth);   //群聊窗口ID（群QQ号）

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + groupWidth];    //获取信息类型
			if (cMsgType == '1')   //文本信息
			{
				msgType = 1;
				msgLen = strData.mid(groupFlagWidth + employeeWidth + groupWidth + msgTypeWidth, msgLengthWidth).toInt();
				strMsg = strData.mid(groupFlagWidth + employeeWidth + groupWidth + msgTypeWidth + msgLengthWidth, msgLen);
			}
			else if (cMsgType == '0')   //表情信息
			{
				msgType = 0;
				int posImages = strData.indexOf("images");    //返回子字符串的位置
				strMsg = strData.right(strData.length() - posImages - QString("images").length());  //截取images后面的表情名称
			}
			else if (cMsgType == '2')   //文件信息
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int posData_begin = strData.indexOf("data_begin");
				
				//文件名称
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);   
				gfileName = fileName;
				
				//文件内容
				int dataLengthWidth;
				int posData = posData_begin + QString("data_begin").length();
				strMsg = strData.mid(posData);
				gfileData = strMsg;

				//根据employeeID获取发送者姓名
				QString Sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1")
					.arg(employeeID));
				querySenderName.exec();
				if (querySenderName.first())
				{
					Sender = querySenderName.value(0).toString();
				}

				//接收文件
				ReceiveFile* recvFile = new ReceiveFile(this);
				connect(recvFile, &ReceiveFile::refuseFile, [this]() { return; });

				QString msgLabel = QString::fromLocal8Bit("收到来自") + Sender + QString::fromLocal8Bit("发来的文件, 是否接收");
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		else  //单聊
		{
			strReceiveEmployeeID = strData.mid(groupFlagWidth + employeeWidth, employeeWidth);
			strWindowID = strSendEmployeeID;

			//不是发给我的信息不做处理
			if (strReceiveEmployeeID != gLoginEmployeeID)
			{
				return;
			}

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + employeeWidth];   //获取信息类型
			if (cMsgType == '1')  //文本信息
			{
				msgType = 1;
				//文本信息长度
				msgLen = strData.mid(groupFlagWidth + employeeWidth + employeeWidth + msgTypeWidth, msgLengthWidth).toInt();

				strMsg = strData.mid(groupFlagWidth + employeeWidth + employeeWidth + msgTypeWidth + msgLengthWidth, msgLen);
			}
			else if (cMsgType == '0')   //表情信息
			{
				msgType = 0;
				int posImages = strData.indexOf("images");  //返回子字符串的位置
				int imagesWidth = QString("images").length();
				strMsg = strData.mid(posImages + imagesWidth);
			}
			else if (cMsgType == '2')   //文件信息
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int data_beginWidth = QString("data_begin").length();
				int posData_begin = strData.indexOf("data_begin");

				//文件名称
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);
				gfileName = fileName;

				//文件内容
				strMsg = strData.mid(posData_begin + data_beginWidth);
				gfileData = strMsg;

				//根据employeeID获取发送者姓名
				QString Sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1")
					.arg(employeeID));
				querySenderName.exec();
				if (querySenderName.first())
				{
					Sender = querySenderName.value(0).toString();
				}

				//接收文件
				ReceiveFile* recvFile = new ReceiveFile(this);
				connect(recvFile, &ReceiveFile::refuseFile, [this]() { return; });

				QString msgLabel = QString::fromLocal8Bit("收到来自") + Sender + QString::fromLocal8Bit("发来的文件, 是否接收");
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}

		//将聊天窗口设为活动的窗口
		QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);
		if (widget)    //聊天窗口存在
		{
			this->setCurrentWidget(widget);

			//同步激活左侧聊天窗口
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);      //设为选中
		}
		else    //聊天窗口未打开
		{
			return; 
		}

		//文件信息另作处理
		if (msgType != 2)
		{
			int sendEmployeeID = strSendEmployeeID.toInt();
			handleReceivedMsg(sendEmployeeID, msgType, strMsg);
		}
	}
}
