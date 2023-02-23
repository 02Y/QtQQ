#include "RootContactItem.h"
#include <QPainter>

RootContactItem::RootContactItem(bool hasArrow, QWidget* parent)
	: QLabel(parent), m_rotation(0), m_hasArrow(hasArrow)
{
	setFixedHeight(32);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_animation = new QPropertyAnimation(this, "rotation");      //初始化属性动画, 绑定“rotation”属性
	m_animation->setDuration(30);       //30ms内动画完成实现
	m_animation->setEasingCurve(QEasingCurve::InQuad);      //动画缓和曲线类型

}

RootContactItem::~RootContactItem()
{
}

void RootContactItem::setText(const QString& title)
{
	m_titleText = title;
	update();      //界面重新绘制
}

void RootContactItem::setExpanded(bool expand)
{
	if (expand)
	{
		m_animation->setEndValue(90);        //设置属性动画的结束值
	}
	else
	{
		m_animation->setEndValue(0);
	}

	m_animation->start();       //动画启动
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
	painter.setRenderHint(QPainter::TextAntialiasing, true);      //设置渲染 文本反锯齿

	QFont font;
	font.setPointSize(10);
	painter.setFont(font);
	painter.drawText(24, 0, width() - 24, height(), Qt::AlignLeft | Qt::AlignVCenter, m_titleText);  //坐标 宽度 高度 左对齐居中对齐 绘制文本
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);        //图片质量更高
	painter.save();            //保存画家设置

	if (m_hasArrow)
	{
		QPixmap pixmap;
		pixmap.load(":/Resources/MainWindow/arrow.png");   //加载图片

		QPixmap tmpPixmap(pixmap.size());
		tmpPixmap.fill(Qt::transparent);        //填充透明

		QPainter p(&tmpPixmap);
		p.setRenderHint(QPainter::SmoothPixmapTransform, true);         //图片质量更高

		p.translate(pixmap.width() / 2, pixmap.height() / 2);           //坐标系偏移(x方向偏移，y方向偏移)
		p.rotate(m_rotation);           //旋转坐标系（顺时针）
		p.drawPixmap(0 - pixmap.width() / 2, 0 - pixmap.height() / 2, pixmap);
		p.end();

		painter.drawPixmap(6, (height() - pixmap.height()) / 2, tmpPixmap);
		painter.restore();      //恢复画家设置
	}

	QLabel::paintEvent(event);
}
