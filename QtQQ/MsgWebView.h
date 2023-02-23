#pragma once

#include <QWebEngineView>
#include <QDomNode>

class MsgHtmlObj : public QObject
{
	Q_OBJECT
	//两个动态属性  
	Q_PROPERTY(QString msgLHtmlTmpl MEMBER m_msgLHtmlTmpl NOTIFY signalMsgHtml)     //别人发来的信息
	Q_PROPERTY(QString msgRHtmlTmpl MEMBER m_msgRHtmlTmpl NOTIFY signalMsgHtml)     //我发的信息

public:
	MsgHtmlObj(QObject* parent, QString msgLPicPath = "");  

signals:
	void signalMsgHtml(const QString& html);

private:
	void initHtmlTmpl();        //初始化聊天网页
	QString getMsgTmplHtml(const QString& code);    //读网页信息

private:
	QString m_msgLPicPath;     //发信息来的人的头像路径
	QString m_msgLHtmlTmpl;    //别人发来的信息
	QString m_msgRHtmlTmpl;    //我发的信息
};



class MsgWebPage : public QWebEnginePage
{
	Q_OBJECT
public:
	MsgWebPage(QObject* parent = nullptr) : QWebEnginePage(parent){}
protected:
	bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;     //接受导航请求
};



//QWebEngineView 视图用于显示网页内容
class MsgWebView : public QWebEngineView
{
	Q_OBJECT

public:
	MsgWebView(QWidget *parent);
	~MsgWebView();     //external0 external_xxx
	void appendMsg(const QString& html, QString strObj = "0");       //追加信息

signals:
	void signalSendMsg(QString& strData, int& msgType, QString sFile = "");

private:
	QList<QStringList> parseHtml(const QString& html);              //解析html
	
	/*Qt中所有Dom（属性、说明、文本等）节点都可以使用QDomNode进行表示*/
	QList<QStringList> parseDocNode(const QDomNode& node);    //解析节点

private:
	MsgHtmlObj* m_msgHtmlObj;
	QWebChannel* m_channel;    //网络通道
};
