#include "EmotionWindow.h"
#include "CommonUtils.h"
#include "EmotionLabelItem.h"
#include <QStyleOption>
#include <QPainter>

const int emotionColumn = 14;
const int emotionRow = 12;

EmotionWindow::EmotionWindow(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);      //无边框 子窗口
	setAttribute(Qt::WA_TranslucentBackground);     //透明
	setAttribute(Qt::WA_DeleteOnClose);       //最后一个窗口关闭时进行资源回收

	ui.setupUi(this);

	initControl();
}

EmotionWindow::~EmotionWindow()
{
}

void EmotionWindow::addEmotion(int emotionNum)
{
	hide();
	emit signalEmotionWindowHide();
	emit signalEmotionItemClicked(emotionNum);
}

void EmotionWindow::initControl()
{
	CommonUtils::loadStyleSheet(this, "EmotionWindow");

	for (int row = 0; row < emotionRow; row++)
	{
		for (int column = 0; column < emotionColumn; column++)
		{
			EmotionLabelItem* label = new EmotionLabelItem(this);
			label->setEmotionName(row * emotionColumn + column);

			connect(label, &EmotionLabelItem::emotionClicked, this, &EmotionWindow::addEmotion);
			ui.gridLayout->addWidget(label, row, column);
		}
	}
}

void EmotionWindow::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.init(this);

	QPainter painter(this);

	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	__super::paintEvent(event);
}
