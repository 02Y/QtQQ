#pragma once

#include <QLabel>
#include <QPropertyAnimation>

class RootContactItem : public QLabel
{
	Q_OBJECT

	//��ͷ�Ƕ�   //��̬����ϵͳ
	Q_PROPERTY(int rotation READ rotation WRITE setRotation)

public:
	RootContactItem(bool hasArrow = true, QWidget* parent = nullptr);
	~RootContactItem();

public:
	void setText(const QString& title);
	void setExpanded(bool expand);

private:
	int rotation();
	void setRotation(int rotation);

protected:
	void paintEvent(QPaintEvent* event);    //�ػ��¼�

private:
	QPropertyAnimation* m_animation;
	QString m_titleText;         //��ʾ�ı�
	int m_rotation;              //��ͷ�Ƕ�
	bool m_hasArrow;             //�Ƿ��м�ͷ
};
