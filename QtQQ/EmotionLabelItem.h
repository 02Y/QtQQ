#pragma once

#include "QClickLabel.h"
#include <QMovie>

class EmotionLabelItem : public QClickLabel
{
	Q_OBJECT

public:
	EmotionLabelItem(QWidget *parent);
	~EmotionLabelItem();
	void setEmotionName(int emotionNum);      //设置表情名称

private:
	void initControl();

signals:
	void emotionClicked(int emotionNum);

private:
	int m_emotionName;
	QMovie* m_apngMovie;       //png动图
};
