#include "UserLogin.h"
#include "CCMainWindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

QString gLoginEmployeeID;   //��½��QQ�ţ�Ա���ţ�

UserLogin::UserLogin(QWidget *parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	initTitleBar();
	setTitleBarTitle("", ":/Resources/MainWindow/qqlogoclassic.png");

	loadStyleSheet("UserLogin");
	initControl();
}

UserLogin::~UserLogin()
{
}

void UserLogin::initControl()
{
	QLabel* headLabel = new QLabel(this);
	headLabel->setFixedSize(68, 68);

	QPixmap pix(":/Resources/MainWindow/head_mask.png");
	headLabel->setPixmap(getRoundImage(QPixmap(":/Resources/MainWindow/app/logo.ico"), pix, headLabel->size()));
	headLabel->move(width() / 2 - 34, ui.titleWidget->height() - 34);

	// ��Ϊ�ǿյ�, ���Բ���ֱ������,Ҫ�ȵ��ú��� ת��
	connect(ui.loginBtn, &QPushButton::clicked, this, &UserLogin::onLoginBtnClicked);

	if (!connectMySql())     //�������ݿ�
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�������ݿ�ʧ�ܣ�"));
		close();
	}
}

bool UserLogin::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setDatabaseName("qtqq");
	db.setHostName("localhost");   //������
	db.setUserName("Gusu");        //�û���
	db.setPassword("zzh1009..");   //����
	db.setPort(3306);       //���ݿ�Ĭ�϶˿�

	if (db.open())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UserLogin::veryfyAccountCode(bool& isAccountLogin, QString& strAccount)
{
	QString strAccountInput = ui.editUserAccount->text();
	QString strCodeInput = ui.editPassword->text();

	//����Ա���ţ�QQ�ŵ�½��
	QString strSqlCode = QString("SELECT code FROM tab_account WHERE employeeID = %1").arg(strAccountInput);
	QSqlQuery queryEmployeeID(strSqlCode);
	queryEmployeeID.exec();

	if (queryEmployeeID.first())    //ָ��������һ��
	{
		//���ݿ���QQ�Ŷ�Ӧ������  value(0)   ��0��  ������һ��Ԫ��
		QString strCode = queryEmployeeID.value(0).toString();

		if (strCode == strCodeInput)
		{
			gLoginEmployeeID = strAccountInput;

			isAccountLogin = false;
			strAccount = strAccountInput;

			return true;
		}
		else
		{
			return false;
		}
	}

	//�˺ŵ�½
	strSqlCode = QString("SELECT code,employeeID FROM tab_account WHERE account = '%1'").arg(strAccountInput);
	QSqlQuery queryAccount(strSqlCode);
	queryAccount.exec();

	if (queryAccount.first())
	{
		QString strCode = queryAccount.value(0).toString();

		if (strCode == strCodeInput)
		{
			gLoginEmployeeID = queryAccount.value(1).toString();
			isAccountLogin = true;
			strAccount = strAccountInput;

			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void UserLogin::onLoginBtnClicked()
{
	bool isAccountLogin;
	QString strAccount;     //�˺� or QQ��
	if (!veryfyAccountCode(isAccountLogin, strAccount))
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�˺�������������"));
		//ui.editPassword->setText(" ");
		//ui.editPassword->setText(" ");
		return;
	}

	//���µ�½״̬Ϊ��½
	QString strSqlStatus = QString("UPDATE tab_employees SET online = 1 WHERE employeeID = %1").arg(gLoginEmployeeID);
	QSqlQuery sqlStatus(strSqlStatus);
	sqlStatus.exec();

	close();
	CCMainWindow* mainwindow = new CCMainWindow(strAccount, isAccountLogin);
	mainwindow->show();
}
