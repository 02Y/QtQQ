#include "TalkWindow.h"
#include "RootContactItem.h"
#include "ContactItem.h"
#include "CommonUtils.h"
#include "WindowManager.h"
#include "QToolTip"
#include <QFile>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include "SendFile.h"

extern QString gLoginEmployeeID;

TalkWindow::TalkWindow(QWidget* parent, const QString& uid/*, GroupType groupType*/)
	: QWidget(parent), m_talkId(uid)/*, m_groupType(groupType)*/
{
	ui.setupUi(this);
	WindowManager::getInstance()->addWindowName(m_talkId, this);
	setAttribute(Qt::WA_DeleteOnClose);

	initGroupTalkStatus();
	initControl();
}

TalkWindow::~TalkWindow()
{
	WindowManager::getInstance()->deleteWindowName(m_talkId);
}

void TalkWindow::onFileOpenBtnClicked(bool)
{
	SendFile* sendFile = new SendFile(this);
	sendFile->show();
}

void TalkWindow::onSendBtnClicked(bool)
{
	if (ui.textEdit->toPlainText().isEmpty())
	{
		QToolTip::showText(this->mapToGlobal(QPoint(630, 660)),
			QString::fromLocal8Bit("���͵���Ϣ����Ϊ�գ�"), this, QRect(0, 0, 120, 100), 2000);
		return;
	}

	QString& html = ui.textEdit->document()->toHtml();

	//�ı�html���û����������������
	if (!html.contains(".png") && !html.contains("</span>"))
	{
		QString fontHtml;
		QString text = ui.textEdit->toPlainText();
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			fontHtml.replace("%1", text);    //�滻�ı�
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
		if (!html.contains(fontHtml))
		{
			html.replace(text, fontHtml);
		}
	}

	ui.textEdit->clear();
	ui.textEdit->deleteAllEmotionImage();

	ui.msgWidget->appendMsg(html);  //����Ϣ����������Ϣ	
}

void TalkWindow::addEmotionImage(int emotionNum)
{
	ui.textEdit->setFocus();    //��ȡ����
	ui.textEdit->addEmotionUrl(emotionNum);   //���ӱ���ͼƬ
}

void TalkWindow::setWindowName(const QString& name)
{
	ui.nameLabel->setText(name);
}

QString TalkWindow::getTalkId()
{
	return m_talkId;
}

void TalkWindow::onItemDoubleClicked(QTreeWidgetItem* item, int colunm)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();

	if (bIsChild)
	{
		QString talkID = item->data(0, Qt::UserRole + 1).toString();
		if (talkID == gLoginEmployeeID)     //���Ʋ������Լ�����
		{
			return;
		}

		QString strPeopleName = m_groupPeopleMap.value(item);      //��ȡ��������ǳ�
		WindowManager::getInstance()->addNewTalkWindow(item->data(0, Qt::UserRole + 1).toString());

	}
}

void TalkWindow::initControl()
{
	QList<int> rightWidgetSize;
	rightWidgetSize << 600 << 138;
	ui.bodySplitter->setSizes(rightWidgetSize);

	ui.textEdit->setFontPointSize(10);      //10��
	ui.textEdit->setFocus();      //���ý���

	connect(ui.sysmin, SIGNAL(clicked(bool)), parent(), SLOT(onShowMin(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.closeBtn, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));

	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmotionBtnClicked(bool)));
	connect(ui.sendBtn, SIGNAL(clicked(bool)), this, SLOT(onSendBtnClicked(bool)));

	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), 
		this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.fileopenBtn, SIGNAL(clicked(bool)), this, SLOT(onFileOpenBtnClicked(bool)));

	if (m_isGroupTalk)     //Ⱥ��
	{
		initTalkWindow();
	}
	else       //����
	{
		initPtoPTalk();
	}

	/*
	switch (m_groupType)
	{
	case COMPANY:
	{
		initCompany();
		break;
	}
	case PERSONELGROUP:
	{
		initPersonelTalk();
		break;
	}
	case MARKETGROUP:
	{
		initMarket();
		break;
	}
	case DEVELOPMENTGROUP:
	{
		initDevelopTalk();
		break;
	}
	default:    //����
	{
		initPtoPTalk();
		break;
	}
	
	}
	*/
}

//void TalkWindow::initCompany()
//{
//	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();      //���ĸ���
//	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);      //ָʾ������
//
//	//����data���������ָ������
//	pRootItem->setData(0, Qt::UserRole, 0);
//	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);
//
//	ui.treeWidget->setFixedHeight(646);   //shell�߶� - shellͷ�ߣ�talkwindow titleWidget��
//
//	int nEmployeeNum = 50;
//	QString qsGroupName = QString::fromLocal8Bit("��˾Ⱥ %1/%2").arg(0).arg(nEmployeeNum);
//	pItemName->setText(qsGroupName);
//
//	//�������ڵ�
//	ui.treeWidget->addTopLevelItem(pRootItem);         //������ĸ���
//	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);    //�������
//
//	//չ��
//	pRootItem->setExpanded(true);
//
//	for (int i = 0; i < nEmployeeNum; ++i)
//	{
//		addPeopInfo(pRootItem);
//	}
//}
//
//void TalkWindow::initPersonelTalk()
//{
//	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();      //���ĸ���
//	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);      //ָʾ������
//
//	//����data���������ָ������
//	pRootItem->setData(0, Qt::UserRole, 0);
//	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);
//
//	ui.treeWidget->setFixedHeight(646);   //shell�߶� - shellͷ�ߣ�talkwindow titleWidget��
//
//	int nEmployeeNum = 5;
//	QString qsGroupName = QString::fromLocal8Bit("���²� %1/%2").arg(0).arg(nEmployeeNum);
//	pItemName->setText(qsGroupName);
//
//	//�������ڵ�
//	ui.treeWidget->addTopLevelItem(pRootItem);         //������ĸ���
//	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);    //�������
//
//	//չ��
//	pRootItem->setExpanded(true);
//
//	for (int i = 0; i < nEmployeeNum; ++i)
//	{
//		addPeopInfo(pRootItem);
//	}
//}
//
//void TalkWindow::initMarket()
//{
//	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();      //���ĸ���
//	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);      //ָʾ������
//
//	//����data���������ָ������
//	pRootItem->setData(0, Qt::UserRole, 0);
//	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);
//
//	ui.treeWidget->setFixedHeight(646);   //shell�߶� - shellͷ�ߣ�talkwindow titleWidget��
//
//	int nEmployeeNum = 8;
//	QString qsGroupName = QString::fromLocal8Bit("�г��� %1/%2").arg(0).arg(nEmployeeNum);
//	pItemName->setText(qsGroupName);
//
//	//�������ڵ�
//	ui.treeWidget->addTopLevelItem(pRootItem);         //������ĸ���
//	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);    //�������
//
//	//չ��
//	pRootItem->setExpanded(true);
//
//	for (int i = 0; i < nEmployeeNum; ++i)
//	{
//		addPeopInfo(pRootItem);
//	}
//}
//
//void TalkWindow::initDevelopTalk()
//{
//	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();      //���ĸ���
//	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);      //ָʾ������
//
//	//����data���������ָ������
//	pRootItem->setData(0, Qt::UserRole, 0);
//	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);
//
//	ui.treeWidget->setFixedHeight(646);   //shell�߶� - shellͷ�ߣ�talkwindow titleWidget��
//
//	int nEmployeeNum = 32;
//	QString qsGroupName = QString::fromLocal8Bit("�з��� %1/%2").arg(0).arg(nEmployeeNum);
//	pItemName->setText(qsGroupName);
//
//	//�������ڵ�
//	ui.treeWidget->addTopLevelItem(pRootItem);         //������ĸ���
//	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);    //�������
//
//	//չ��
//	pRootItem->setExpanded(true);
//
//	for (int i = 0; i < nEmployeeNum; ++i)
//	{
//		addPeopInfo(pRootItem);
//	}
//}

void TalkWindow::initGroupTalkStatus()
{
	QSqlQueryModel sqlDepModel;
	QString strSql = QString("SELECT * FROM tab_department WHERE departmentID = %1").arg(m_talkId);
	sqlDepModel.setQuery(strSql);

	int rows = sqlDepModel.rowCount();
	if (rows == 0)   //����
	{
		m_isGroupTalk = false;
	}
	else
	{
		m_isGroupTalk = true;
	}
}

void TalkWindow::initTalkWindow()
{
	QTreeWidgetItem* pRootItem = new QTreeWidgetItem();      //���ĸ���
	pRootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);      //ָʾ������

	//����data���������ָ������
	pRootItem->setData(0, Qt::UserRole, 0);
	RootContactItem* pItemName = new RootContactItem(false, ui.treeWidget);

	ui.treeWidget->setFixedHeight(646);   //shell�߶� - shellͷ�ߣ�talkwindow titleWidget��

	//��ǰ�����Ⱥ����
	QString strGroupName;
	QSqlQuery queryGroupName(QString("SELECT name FROM tab_department WHERE departmentID = %1").arg(m_talkId));
	queryGroupName.exec();     //ִ�в�ѯ
	if (queryGroupName.first())
	{
		strGroupName = queryGroupName.value(0).toString();
	}

	//��ȡԱ������
	int nEmployeeNum = 0;
	QSqlQueryModel queryEmployeeModel;
	if (getCompDepID() == m_talkId.toInt())   //��˾Ⱥ
	{
		queryEmployeeModel.setQuery(QString("SELECT employeeID FROM tab_employees WHERE status = 1"));
	}
	else
	{
		queryEmployeeModel.setQuery(QString("SELECT employeeID FROM tab_employees WHERE status = 1 AND departmentID = %1")
			.arg(m_talkId));
			
	}
	nEmployeeNum = queryEmployeeModel.rowCount();    //��ȡ����

	QString qsGroupName = QString::fromLocal8Bit("%1 %2/%3").arg(strGroupName).arg(0).arg(nEmployeeNum);
	pItemName->setText(qsGroupName);

	//�������ڵ�
	ui.treeWidget->addTopLevelItem(pRootItem);         //������ĸ���
	ui.treeWidget->setItemWidget(pRootItem, 0, pItemName);    //�������

	//չ��
	pRootItem->setExpanded(true);

	for (int i = 0; i < nEmployeeNum; ++i)
	{
		QModelIndex modelIndex = queryEmployeeModel.index(i, 0);
		int employeeID = queryEmployeeModel.data(modelIndex).toInt();

		//if (employeeID == gLoginEmployeeID.toInt()) continue;     ���Ʋ����Լ�����

		//�����ӽڵ�
		addPeopInfo(pRootItem, employeeID);
	}
}

void TalkWindow::initPtoPTalk()
{
	QPixmap pixSkin;
	pixSkin.load(":/Resources/MainWindow/skin.png");

	ui.widget->setFixedSize(pixSkin.size());
	QLabel* skinLabel = new QLabel(ui.widget);
	skinLabel->setPixmap(pixSkin);
	skinLabel->setFixedSize(ui.widget->size());
}

int TalkWindow::getCompDepID()
{
	QSqlQuery queryDepID(QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'")
		.arg(QString::fromLocal8Bit("��˾Ⱥ")));
	queryDepID.exec();
	queryDepID.next();

	return queryDepID.value(0).toInt();
}

void TalkWindow::addPeopInfo(QTreeWidgetItem* pRootGroupItem, int empolyeeID)
{
	QTreeWidgetItem* pChild = new QTreeWidgetItem();
	

	//�����ӽڵ�
	pChild->setData(0, Qt::UserRole, 1);
	pChild->setData(0, Qt::UserRole + 1, empolyeeID);

	ContactItem* pContactItem = new ContactItem(ui.treeWidget);

	QPixmap pix1;
	pix1.load(":/Resources/MainWindow/head_mask.png");

	//��ȡ����ǩ����ͷ��
	QString strName, strSign, strPicturePath;
	QSqlQueryModel queryInfoModel;
	queryInfoModel.setQuery(QString("SELECT employee_name,employee_sign,picture FROM tab_employees WHERE employeeID = %1")
		.arg(empolyeeID));
	
	QModelIndex nameIndex, signIndex, pictureIndex;
	nameIndex = queryInfoModel.index(0, 0);    //�У���
	signIndex = queryInfoModel.index(0, 1);
	pictureIndex = queryInfoModel.index(0, 2);

	strName = queryInfoModel.data(nameIndex).toString();
	strSign = queryInfoModel.data(signIndex).toString();
	strPicturePath = queryInfoModel.data(pictureIndex).toString();


	QImage imageHead;
	imageHead.load(strPicturePath);

	pContactItem->setHeadPixmap(CommonUtils::getRoundImage(QPixmap::fromImage(imageHead), pix1, 
		pContactItem->getHeadLabelSize()));

	pContactItem->setUserName(strName);    //�û���
	pContactItem->setSignName(strSign);    //ǩ��

	pRootGroupItem->addChild(pChild);
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	QString str = pContactItem->getUserName();
	m_groupPeopleMap.insert(pChild, str);
}