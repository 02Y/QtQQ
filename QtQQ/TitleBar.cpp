#include "TitleBar.h"

#include <QHBoxLayout> 
#include <QPainter>
#include <QMouseEvent>
#include <QFile>

#define BUTTON_HEIGHT 27     //按钮高度
#define BUTTON_WIDTH 27      //按钮宽度
#define TITLE_HEIGHT 27      //标题栏高度

TitleBar::TitleBar(QWidget* parent)
	: QWidget(parent), m_isPressed(false), m_buttonType(MIN_MAX_BUTTON)
{
	//setStyleSheet("color:yellow;");
	initControl();               
	initConnections();           
	loadStyleSheet("Title");     
}

TitleBar::~TitleBar()
{
}

//设置标题栏图标
void TitleBar::setTitleIcon(const QString& filePath)
{
	QPixmap titleIcon(filePath);
	m_pIcon->setFixedSize(titleIcon.size());					// 以titleIcon的大小,来设置图标大小
	m_pIcon->setPixmap(titleIcon);
}

//设置标题栏内容
void TitleBar::setTitleContent(const QString& titleContent)
{
	m_pTitleContent->setText(titleContent);				// 设置文本
	m_titleContent = titleContent;					   // 内容更新
}

//设置标题栏长度
void TitleBar::setTitleWidth(int width)
{
	setFixedWidth(width);
}

//设置标题栏按钮类型
void TitleBar::setButtonType(ButtonType buttonType)
{
	m_buttonType = buttonType;			// 更新

	switch (buttonType)
	{
	case MIN_BUTTON:
	{
		m_pButtonRestore->setVisible(false);				// setVisible ,是否可见
		m_pButtonMax->setVisible(false);
	}
	break;
	case MIN_MAX_BUTTON:
	{
		m_pButtonRestore->setVisible(false);
	}
	break;
	case ONLY_CLOSE_BUTTON:
	{
		m_pButtonRestore->setVisible(false);
		m_pButtonMax->setVisible(false);
		m_pButtonMin->setVisible(false);
	}
	break;

	default:
		break;
	}
}

//保存窗口最大化前窗口的位置及大小
void TitleBar::saveRestoreInfo(const QPoint& point, const QSize& size)
{
	m_restorePos = point;
	m_restoreSize = size;
}

//获取窗口最大化前窗口的位置及大小
void TitleBar::getRestoreInfo(QPoint& point, QSize& size)
{
	point = m_restorePos;
	size = m_restoreSize;
}

//绘制标题栏
void TitleBar::paintEvent(QPaintEvent* event)
{
	// 设置背景色
	QPainter painter(this);
	QPainterPath pathBack;										// 绘图路径
	pathBack.setFillRule(Qt::WindingFill);					// 设置填充规则
	pathBack.addRoundedRect(QRect(0, 0, width(), height()), 3, 3);				// 添加圆角矩形, 到绘图路径
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);			// 平滑

	// 当窗口最大化或还原后, 窗口长度改变, 标题栏相应做出改变
	// parentWidget() ,返回 父部件
	if (width() != parentWidget()->width())				// 父部件 parentWidget , 调用宽度
	{
		setFixedWidth(parentWidget()->width());
	}

	//把标题栏重绘事件，添加到默认中
	QWidget::paintEvent(event);
}

//鼠标双击事件，主要实现双击标题栏进行最大化或最小化操作
void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	//只有存在最大化、或最小化按钮才有效
	if (m_buttonType == MIN_MAX_BUTTON)
	{
		if (m_pButtonMax->isVisible())
			onButtonMaxClicked();
		else
			onButtonRestoreClicked();
	}

	return QWidget::mouseDoubleClickEvent(event);
}

//通过鼠标按下、鼠标移动、鼠标释放事件实现拖动标题栏达到移动窗口效果
void TitleBar::mousePressEvent(QMouseEvent* event)
{
	if (m_buttonType == MIN_MAX_BUTTON)
	{
		//在窗口最大化时禁止拖动窗口
		if (m_pButtonMax->isVisible())
		{
			m_isPressed = true;
			m_startMovePos = event->globalPos();   //globalPos() 返回事件发生时鼠标所在的全局位置
		}
	}
	else
	{
		m_isPressed = true;
		m_startMovePos = event->globalPos();
	}

	return QWidget::mousePressEvent(event);
}

//鼠标移动事件
void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPressed)
	{
		QPoint movePoint = event->globalPos() - m_startMovePos;		// 返回当前的鼠标位置 ,减去 之前的位置
		QPoint widgetPos = parentWidget()->pos();									// 返回部件的位置
		m_startMovePos = event->globalPos();

		// 调用父部件的移动方法函数 , 移动前后坐标相加
		parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
	}

	return QWidget::mouseMoveEvent(event);
}

//鼠标释放事件
void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
	m_isPressed = false;

	return QWidget::mouseReleaseEvent(event);
}

//初始化控件
void TitleBar::initControl()       
{
	// 生成对象, 部件
	m_pIcon = new QLabel(this);
	m_pTitleContent = new QLabel(this);
	m_pButtonMin = new QPushButton(this);
	m_pButtonRestore = new QPushButton(this);
	m_pButtonMax = new QPushButton(this);
	m_pButtonClose = new QPushButton(this);

	// 对按钮,设置固定大小
	m_pButtonMin->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));				// 最小化
	m_pButtonRestore->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));		// 最大化还原按钮
	m_pButtonMax->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));				// 最大化
	m_pButtonClose->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));			// 关闭按钮

	// 设置对象名
	m_pTitleContent->setObjectName("TitleContent");					// 标题内容
	m_pButtonMin->setObjectName("ButtonMin");
	m_pButtonRestore->setObjectName("ButtonRestore");
	m_pButtonMax->setObjectName("ButtonMax");
	m_pButtonClose->setObjectName("ButtonClose");

	// 设置布局
	QHBoxLayout* mylayout = new QHBoxLayout(this);
	mylayout->addWidget(m_pIcon);										// 添加各种按钮
	mylayout->addWidget(m_pTitleContent);
	mylayout->addWidget(m_pButtonMin);
	mylayout->addWidget(m_pButtonRestore);
	mylayout->addWidget(m_pButtonMax);
	mylayout->addWidget(m_pButtonClose);

	mylayout->setContentsMargins(5, 0, 0, 0);						// 设置布局, 间隙
	mylayout->setSpacing(0);												// 设置布局里部件之间, 间隙

	// 标题的尺寸策略
	// Expanding, 垂直方向 , Fixed , 固定
	m_pTitleContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(TITLE_HEIGHT);								// 标题栏高度
	setWindowFlags(Qt::FramelessWindowHint);			// 无边框
}

//初始化信号与槽的连接
void TitleBar::initConnections()
{
	connect(m_pButtonMin, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
	connect(m_pButtonRestore, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
	connect(m_pButtonMax, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
	connect(m_pButtonClose, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
}

//加载样式表
void TitleBar::loadStyleSheet(const QString& sheetName)
{
	QFile file(":/Resources/QSS/" + sheetName + ".css");
	file.open(QFile::ReadOnly);

	if (file.isOpen())
	{
		QString styleSheet = this->styleSheet();				// 保存样式表
		// 将文件中的样式表全部合进来 , 用 += 操作 , 对所有的文件内容进行读取
		// QLatin1String 保存的是字符串的地址, 资源开销小
		styleSheet += QLatin1String(file.readAll());
		setStyleSheet(styleSheet);									// 对样式表进行设置
	}
}

//最小化按钮响应的槽
void TitleBar::onButtonMinClicked()
{
	emit signalButtonMinClicked();
}

//最大化还原按钮
void TitleBar::onButtonRestoreClicked()
{
	m_pButtonRestore->setVisible(false);
	m_pButtonMax->setVisible(true);
	emit signalButtonRestoreClicked();
}

void TitleBar::onButtonMaxClicked()
{
	m_pButtonMax->setVisible(false);
	m_pButtonRestore->setVisible(true);
	emit signalButtonMaxClicked();
}

void TitleBar::onButtonCloseClicked()
{
	emit signalButtonCloseClicked();
}

