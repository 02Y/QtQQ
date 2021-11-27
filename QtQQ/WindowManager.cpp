#include "WindowManager.h"
#include "TalkWindow.h"
#include "TalkWindowItem.h"
#include <QSqlQueryModel>

Q_GLOBAL_STATIC(WindowManager, theInstance);      //单例模式， 创建全局静态对象

WindowManager::WindowManager()
	:QObject(nullptr), m_talkwindowshell(nullptr)
{
	
}

WindowManager::~WindowManager()
{
}

QWidget* WindowManager::findWindowName(const QString& qsWindowName)
{
	if (m_windowMap.contains(qsWindowName))           //查找是否包含窗口名
	{
		return m_windowMap.value(qsWindowName);       //返回值
	}
}

void WindowManager::deleteWindowName(const QString& qsWindowName)
{
	m_windowMap.remove(qsWindowName);
}

void WindowManager::addWindowName(const QString& qsWindowName, QWidget* qWidget)
{
	if (!m_windowMap.contains(qsWindowName))
	{
		m_windowMap.insert(qsWindowName, qWidget);
	}
}

WindowManager* WindowManager::getInstance()
{
	return theInstance();
}

TalkWindowShell* WindowManager::getTalkWindowShell()
{
	return m_talkwindowshell;
}

void WindowManager::addNewTalkWindow(const QString& uid/*, GroupType groupType, const QString& strPeople*/)
{
	if (m_talkwindowshell == nullptr)
	{
		m_talkwindowshell = new TalkWindowShell;
		connect(m_talkwindowshell, &TalkWindowShell::destroyed, [this](QObject* obj) {
			m_talkwindowshell = nullptr;
		});
	}

	QWidget* widget = findWindowName(uid);
	if(!widget)      //没找到窗口
	{
		m_strCreatingTalkID = uid;
		TalkWindow* talkwindow = new TalkWindow(m_talkwindowshell, uid/*, groupType*/);
		TalkWindowItem* talkwindowItem = new TalkWindowItem(talkwindow);

		m_strCreatingTalkID = "";
		//判断是群聊还是单聊
		QSqlQueryModel sqlDepModel;
		QString strSql = QString("SELECT department_name,sign FROM tab_department WHERE departmentID = %1").arg(uid);
		sqlDepModel.setQuery(strSql);
		int rows = sqlDepModel.rowCount();

		QString strWindowName, strMsgLabel;
		if (rows == 0)   //单聊
		{
			QString sql = QString("SELECT employee_name, employee_sign FROM tab_employees WHERE employeeID = %1").arg(uid);
			sqlDepModel.setQuery(sql);	
		}

		QModelIndex indexDepIndex, signIndex;
		indexDepIndex = sqlDepModel.index(0, 0);   //部门索引
		signIndex = sqlDepModel.index(0, 1);       //签名索引
		strWindowName = sqlDepModel.data(signIndex).toString();
		strMsgLabel = sqlDepModel.data(indexDepIndex).toString();
		
		talkwindow->setWindowName(strWindowName);          //窗口名称
		talkwindowItem->setMsgLabelContent(strMsgLabel);   //左侧联系人文本显示
		m_talkwindowshell->addTalkWindow(talkwindow, talkwindowItem, uid);

		/*
		switch (groupType)
		{
		case COMPANY:
		{
			talkwindow->setWindowName(QStringLiteral("巢枝"));
			talkwindowItem->setMsgLabelContent(QStringLiteral("公司群"));
			break;
		}
		case PERSONELGROUP:
		{
			talkwindow->setWindowName(QStringLiteral("人才招纳"));
			talkwindowItem->setMsgLabelContent(QStringLiteral("人事群"));
			break;
		}
		case MARKETGROUP:
		{
			talkwindow->setWindowName(QStringLiteral("精准把握用户需求"));
			talkwindowItem->setMsgLabelContent(QStringLiteral("市场群"));
			break;
		}
		case DEVELOPMENTGROUP:
		{
			talkwindow->setWindowName(QStringLiteral("实用且高效"));
			talkwindowItem->setMsgLabelContent(QStringLiteral("研发群"));
			break;
		}
		case PTOP:
		{
			talkwindow->setWindowName(QStringLiteral(""));
			talkwindowItem->setMsgLabelContent(strPeople);
			break;
		}
		default:
			break;
		}

		m_talkwindowshell->addTalkWindow(talkwindow, talkwindowItem, groupType);
		*/
	}
	else      //找到窗口
	{
		//左侧聊天列表设为选中
		QListWidgetItem* item = m_talkwindowshell->getTalkWindowItemMap().key(widget);
		item->setSelected(true);

		//设置右侧当前聊天窗口
		m_talkwindowshell->setCurrentWidget(widget);
	}

	m_talkwindowshell->show();
	m_talkwindowshell->activateWindow();
}

QString WindowManager::getCreatingTalkID()
{
	return m_strCreatingTalkID;
}
