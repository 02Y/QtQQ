#include "MsgWebView.h"
#include <QFile>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWebChannel>
#include "TalkWindowShell.h"
#include "WindowManager.h"
#include <QSqlQueryModel>

extern QString gstrLoginHeadPath;

MsgHtmlObj::MsgHtmlObj(QObject* parent, QString msgLPicPath): QObject(parent)
{
	m_msgLPicPath = msgLPicPath;
	initHtmlTmpl();
}

void MsgHtmlObj::initHtmlTmpl()
{
	m_msgLHtmlTmpl = getMsgTmplHtml("msgleftTmpl");
	m_msgLHtmlTmpl.replace("%1", m_msgLPicPath);

	m_msgRHtmlTmpl = getMsgTmplHtml("msgrightTmpl");
	m_msgRHtmlTmpl.replace("%1", gstrLoginHeadPath);
}

QString MsgHtmlObj::getMsgTmplHtml(const QString& code)
{
	QFile file(":/Resources/MainWindow/MsgHtml/" + code + ".html");
	file.open(QFile::ReadOnly);
	QString strData;
	if (file.isOpen())
	{
		strData = QLatin1String(file.readAll());
	}
	else
	{
		QMessageBox::information(nullptr, "Tips", "Faild to init html!");
		return QString("");
	}
	file.close();

	return strData;
}

bool MsgWebPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
	//仅接受"qrc:/*.html"
	if (url.scheme() == QString("qrc"))     //判断url类型
	{
		return true;
	}
	return false;
}

MsgWebView::MsgWebView(QWidget* parent)
	: QWebEngineView(parent), m_channel(new QWebChannel(this))
{
	MsgWebPage* page = new MsgWebPage(this);
	setPage(page);

	//QWebChannel* channel = new QWebChannel(this);
	m_msgHtmlObj = new MsgHtmlObj(this);
	m_channel->registerObject("external0", m_msgHtmlObj);
	//this->page()->setWebChannel(m_channel);

	TalkWindowShell* talkWindowShell = WindowManager::getInstance()->getTalkWindowShell();
	connect(this, &MsgWebView::signalSendMsg, talkWindowShell, &TalkWindowShell::updateSendTcpMsg);

	//当前正构建的聊天窗口的ID（QQ号）
	QString strTalkID = WindowManager::getInstance()->getCreatingTalkID();

	QSqlQueryModel queryEmployeeModel;
	QString strEmployeeID, strPicturePath;
	QString strExternal;
	bool isGroupTalk = false;

	//获取公司群ID
	queryEmployeeModel.setQuery(QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(QStringLiteral("公司群")));
	QModelIndex companyIndex = queryEmployeeModel.index(0, 0);
	QString strCompanyID = queryEmployeeModel.data(companyIndex).toString();

	if (strTalkID == strCompanyID)//公司群聊
	{
		isGroupTalk = true;
		queryEmployeeModel.setQuery("SELECT employeeID, picture FROM tab_employees WHERE status = 1");
	}
	else
	{
		if (strTalkID.length() == 4)//其它群聊
		{
			isGroupTalk = true;
			queryEmployeeModel.setQuery(QString("SELECT employeeID, picture FROM tab_employees WHERE status = 1 AND departmentID = %1").arg(strTalkID));
		}
		else//单独聊天
		{
			queryEmployeeModel.setQuery(QString("SELECT picture FROM tab_employees WHERE status = 1 AND employeeID = %1").arg(strTalkID));

			QModelIndex index = queryEmployeeModel.index(0, 0);
			strPicturePath = queryEmployeeModel.data(index).toString();

			strExternal = "external_" + strTalkID;
			MsgHtmlObj* msgHtmlObj = new MsgHtmlObj(this, strPicturePath);
			m_channel->registerObject(strExternal, msgHtmlObj);
		}
	}

	if (isGroupTalk)
	{
		QModelIndex employeeModelIndex, pictureModelIndex;
		int rows = queryEmployeeModel.rowCount();
		for (int i = 0; i < rows; i++)
		{
			employeeModelIndex = queryEmployeeModel.index(i, 0);
			pictureModelIndex = queryEmployeeModel.index(i, 1);

			strEmployeeID = queryEmployeeModel.data(employeeModelIndex).toString();
			strPicturePath = queryEmployeeModel.data(pictureModelIndex).toString();

			strExternal = "external_" + strEmployeeID;

			MsgHtmlObj* msgHtmlObj = new MsgHtmlObj(this, strPicturePath);
			m_channel->registerObject(strExternal, msgHtmlObj);
		}
	}

	this->page()->setWebChannel(m_channel);
	//初始化接收信息的页面
	this->load(QUrl("qrc:/Resources/MainWindow/MsgHtml/msgTmpl.html"));
}

MsgWebView::~MsgWebView()
{
}

void MsgWebView::appendMsg(const QString& html, QString strObj)
{
	QJsonObject msgObj;
	QString qsMsg;
	const QList<QStringList> msgList = parseHtml(html);   //解析html

	int msgType = 1;    //信息类型：0-表情 1-文本 2-文件
	int imageNum = 0;   //表情数量
	bool isImageMsg = false;
	QString strData;    //055 008   表情名称补足三位

	for (int i = 0; i < msgList.size(); i++)
	{
		if (msgList.at(i).at(0) == "img")
		{
			QString imagePath = msgList.at(i).at(1);    //图片路径
			QPixmap pixmap;

			//获取表情名称的位置
			QString strEmotionPath = "qrc:/Resources/MainWindow/emotion/";   //qrc表示资源文件路径
			int pos = strEmotionPath.size();      //长度
			isImageMsg = true;

			//获取表情名称
			QString strEmotionName = imagePath.mid(pos);
			strEmotionName.replace(".png", "");    //替换，把"png"替换为""
			
			//根据表情名称的长度进行设置表情数据，不足3为则补足3位  55-055
			int emotionNameL = strEmotionName.length();
			if (emotionNameL == 1)
			{
				strData = strData + "00" + strEmotionName;
			}
			else if (emotionNameL == 2)
			{
				strData = strData + "0" + strEmotionName;
			}
			else
			{
				strData = strData + strEmotionName;
			}

			msgType = 0;    //表情信息
			imageNum++;

			if (imagePath.left(3) == "qrc")    //qrc:/MainWindow/xxx    去掉qrc
			{
				pixmap.load(imagePath.mid(3));    // :/MainWindow/xxx
			}
			else
			{
				pixmap.load(imagePath);
			}

			//表情图片html格式文本组合
			QString imgPath = QString("<img src=\"%1\" width=\"%2\" heigth=\"%3\"/>")
				.arg(imagePath).arg(pixmap.width()).arg(pixmap.height());
			qsMsg += imgPath;
		}
		else if (msgList.at(i).at(0) == "text")
		{
			qsMsg += msgList.at(i).at(1);
			strData = qsMsg;
		}
	}

	msgObj.insert("MSG", qsMsg);   

	const QString& Msg = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);    //从对象创建一个QJsonDocument再转换为指定格式的JSON文档
	if (strObj == "0")    //发信息
	{
		this->page()->runJavaScript(QString("appendHtml0(%1)").arg(Msg));
		
		if (isImageMsg)
		{
			strData = QString::number(imageNum) + "images" + strData;
		}
		emit signalSendMsg(strData, msgType);
	}
	else    //接受信息
	{
		this->page()->runJavaScript(QString("recvHtml_%1(%2)").arg(strObj).arg(Msg));
	}
}

QList<QStringList> MsgWebView::parseHtml(const QString& html)
{
	QDomDocument doc;     //节点文档
	doc.setContent(html);
	const QDomElement& root = doc.documentElement();        //节点元素
	const QDomNode& node = root.firstChildElement("body");  //身体节点

	return parseDocNode(node);
}

QList<QStringList> MsgWebView::parseDocNode(const QDomNode& node)
{
	QList<QStringList> attribute;
	const QDomNodeList& list = node.childNodes();   //返回所有子节点

	for (int i = 0; i < list.count(); i++)
	{
		const QDomNode& node = list.at(i);

		if (node.isElement())      //是否为元素
		{
			//转换元素
			const QDomElement& element = node.toElement();
			if (element.tagName() == "img")   //返回标签名   图片
			{
				QStringList attributeList;    //属性链表
				attributeList << "img" << element.attribute("src");//图片路径
				attribute << attributeList;
			}

			if (element.tagName() == "span")   //文本
			{
				QStringList attributeList;
				attributeList << "text" << element.text();//返回文本
				attribute << attributeList;
			}

			if (node.hasChildNodes())       //还有节点的情况
			{
				attribute << parseDocNode(node);
			}
		}
	}

	return attribute;
}


