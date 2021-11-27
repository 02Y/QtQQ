#pragma once

#include <QWidget>
#include "ui_ContactItem.h"

class ContactItem : public QWidget
{
	Q_OBJECT

public:
	ContactItem(QWidget *parent = Q_NULLPTR);
	~ContactItem();

	void setUserName(const QString& userName);         //设置用户名
	void setSignName(const QString& signName);         //设置签名
	void setHeadPixmap(const QPixmap& headPath);       //设置头像
	QString getUserName() const;                       //获取用户名
	QSize getHeadLabelSize() const;                    //获取头像尺寸

private:
	void initControl();

private:
	Ui::ContactItem ui;
};
