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
	//������"qrc:/*.html"
	if (url.scheme() == QString("qrc"))     //�ж�url����
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

	//��ǰ�����������촰�ڵ�ID��QQ�ţ�
	QString strTalkID = WindowManager::getInstance()->getCreatingTalkID();

	QSqlQueryModel queryEmployeeModel;
	QString strEmployeeID, strPicturePath;
	QString strExternal;
	bool isGroupTalk = false;

	//��ȡ��˾ȺID
	queryEmployeeModel.setQuery(QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(QStringLiteral("��˾Ⱥ")));
	QModelIndex companyIndex = queryEmployeeModel.index(0, 0);
	QString strCompanyID = queryEmployeeModel.data(companyIndex).toString();

	if (strTalkID == strCompanyID)//��˾Ⱥ��
	{
		isGroupTalk = true;
		queryEmployeeModel.setQuery("SELECT employeeID, picture FROM tab_employees WHERE status = 1");
	}
	else
	{
		if (strTalkID.length() == 4)//����Ⱥ��
		{
			isGroupTalk = true;
			queryEmployeeModel.setQuery(QString("SELECT employeeID, picture FROM tab_employees WHERE status = 1 AND departmentID = %1").arg(strTalkID));
		}
		else//��������
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
	//��ʼ��������Ϣ��ҳ��
	this->load(QUrl("qrc:/Resources/MainWindow/MsgHtml/msgTmpl.html"));
}

MsgWebView::~MsgWebView()
{
}

void MsgWebView::appendMsg(const QString& html, QString strObj)
{
	QJsonObject msgObj;
	QString qsMsg;
	const QList<QStringList> msgList = parseHtml(html);   //����html

	int msgType = 1;    //��Ϣ���ͣ�0-���� 1-�ı� 2-�ļ�
	int imageNum = 0;   //��������
	bool isImageMsg = false;
	QString strData;    //055 008   �������Ʋ�����λ

	for (int i = 0; i < msgList.size(); i++)
	{
		if (msgList.at(i).at(0) == "img")
		{
			QString imagePath = msgList.at(i).at(1);    //ͼƬ·��
			QPixmap pixmap;

			//��ȡ�������Ƶ�λ��
			QString strEmotionPath = "qrc:/Resources/MainWindow/emotion/";   //qrc��ʾ��Դ�ļ�·��
			int pos = strEmotionPath.size();      //����
			isImageMsg = true;

			//��ȡ��������
			QString strEmotionName = imagePath.mid(pos);
			strEmotionName.replace(".png", "");    //�滻����"png"�滻Ϊ""
			
			//���ݱ������Ƶĳ��Ƚ������ñ������ݣ�����3Ϊ����3λ  55-055
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

			msgType = 0;    //������Ϣ
			imageNum++;

			if (imagePath.left(3) == "qrc")    //qrc:/MainWindow/xxx    ȥ��qrc
			{
				pixmap.load(imagePath.mid(3));    // :/MainWindow/xxx
			}
			else
			{
				pixmap.load(imagePath);
			}

			//����ͼƬhtml��ʽ�ı����
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

	const QString& Msg = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);    //�Ӷ��󴴽�һ��QJsonDocument��ת��Ϊָ����ʽ��JSON�ĵ�
	if (strObj == "0")    //����Ϣ
	{
		this->page()->runJavaScript(QString("appendHtml0(%1)").arg(Msg));
		
		if (isImageMsg)
		{
			strData = QString::number(imageNum) + "images" + strData;
		}
		emit signalSendMsg(strData, msgType);
	}
	else    //������Ϣ
	{
		this->page()->runJavaScript(QString("recvHtml_%1(%2)").arg(strObj).arg(Msg));
	}
}

QList<QStringList> MsgWebView::parseHtml(const QString& html)
{
	QDomDocument doc;     //�ڵ��ĵ�
	doc.setContent(html);
	const QDomElement& root = doc.documentElement();        //�ڵ�Ԫ��
	const QDomNode& node = root.firstChildElement("body");  //����ڵ�

	return parseDocNode(node);
}

QList<QStringList> MsgWebView::parseDocNode(const QDomNode& node)
{
	QList<QStringList> attribute;
	const QDomNodeList& list = node.childNodes();   //���������ӽڵ�

	for (int i = 0; i < list.count(); i++)
	{
		const QDomNode& node = list.at(i);

		if (node.isElement())      //�Ƿ�ΪԪ��
		{
			//ת��Ԫ��
			const QDomElement& element = node.toElement();
			if (element.tagName() == "img")   //���ر�ǩ��   ͼƬ
			{
				QStringList attributeList;    //��������
				attributeList << "img" << element.attribute("src");//ͼƬ·��
				attribute << attributeList;
			}

			if (element.tagName() == "span")   //�ı�
			{
				QStringList attributeList;
				attributeList << "text" << element.text();//�����ı�
				attribute << attributeList;
			}

			if (node.hasChildNodes())       //���нڵ�����
			{
				attribute << parseDocNode(node);
			}
		}
	}

	return attribute;
}


