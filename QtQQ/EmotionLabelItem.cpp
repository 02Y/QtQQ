#include "EmotionLabelItem.h"



EmotionLabelItem::EmotionLabelItem(QWidget *parent)
	: QClickLabel(parent)
{
	initControl();
	connect(this, &QClickLabel::clicked, [this]() {
		emit emotionClicked(m_emotionName);
		});
}

EmotionLabelItem::~EmotionLabelItem()
{
}

void EmotionLabelItem::setEmotionName(int emotionNum)
{
	m_emotionName = emotionNum;
	QString imageName = QString(":/Resources/MainWindow/emotion/%1.png").arg(emotionNum);
	
	m_apngMovie = new QMovie(imageName, "apng", this);
	m_apngMovie->start();
	m_apngMovie->stop();
	setMovie(m_apngMovie);
}

void EmotionLabelItem::initControl()
{
	setAlignment(Qt::AlignCenter);     //…Ë÷√æ”÷–∂‘∆Î
	setObjectName("emotionLabelItem");
	setFixedSize(32, 32);
}
