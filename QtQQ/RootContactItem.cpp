#include "RootContactItem.h"
#include <QPainter>

RootContactItem::RootContactItem(bool hasArrow, QWidget* parent)
	: QLabel(parent), m_rotation(0), m_hasArrow(hasArrow)
{
	setFixedHeight(32);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_animation = new QPropertyAnimation(this, "rotation");      //��ʼ�����Զ���, �󶨡�rotation������
	m_animation->setDuration(30);       //30ms�ڶ������ʵ��
	m_animation->setEasingCurve(QEasingCurve::InQuad);      //����������������

}

RootContactItem::~RootContactItem()
{
}

void RootContactItem::setText(const QString& title)
{
	m_titleText = title;
	update();      //�������»���
}

void RootContactItem::setExpanded(bool expand)
{
	if (expand)
	{
		m_animation->setEndValue(90);        //�������Զ����Ľ���ֵ
	}
	else
	{
		m_animation->setEndValue(0);
	}

	m_animation->start();       //��������
}

int RootContactItem::rotation()
{
	return m_rotation;
}

void RootContactItem::setRotation(int rotation)
{
	m_rotation = rotation;
	update();
}

void RootContactItem::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::TextAntialiasing, true);      //������Ⱦ �ı������

	QFont font;
	font.setPointSize(10);
	painter.setFont(font);
	painter.drawText(24, 0, width() - 24, height(), Qt::AlignLeft | Qt::AlignVCenter, m_titleText);  //���� ��� �߶� �������ж��� �����ı�
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);        //ͼƬ��������
	painter.save();            //���滭������

	if (m_hasArrow)
	{
		QPixmap pixmap;
		pixmap.load(":/Resources/MainWindow/arrow.png");   //����ͼƬ

		QPixmap tmpPixmap(pixmap.size());
		tmpPixmap.fill(Qt::transparent);        //���͸��

		QPainter p(&tmpPixmap);
		p.setRenderHint(QPainter::SmoothPixmapTransform, true);         //ͼƬ��������

		p.translate(pixmap.width() / 2, pixmap.height() / 2);           //����ϵƫ��(x����ƫ�ƣ�y����ƫ��)
		p.rotate(m_rotation);           //��ת����ϵ��˳ʱ�룩
		p.drawPixmap(0 - pixmap.width() / 2, 0 - pixmap.height() / 2, pixmap);
		p.end();

		painter.drawPixmap(6, (height() - pixmap.height()) / 2, tmpPixmap);
		painter.restore();      //�ָ���������
	}

	QLabel::paintEvent(event);
}
