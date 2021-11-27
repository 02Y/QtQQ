#pragma once

#include <QWidget>
#include "ui_EmotionWindow.h"

class EmotionWindow : public QWidget
{
	Q_OBJECT

public:
	EmotionWindow(QWidget *parent = Q_NULLPTR);
	~EmotionWindow();

private:
	void initControl();
	
private slots:
	void addEmotion(int emotionNum);

signals:
	void signalEmotionWindowHide();             //表情窗口隐藏
	void signalEmotionItemClicked(int emotionNum);        //表情被点击

private:
	void paintEvent(QPaintEvent* event) override;        //重写绘图事件      

private:
	Ui::EmotionWindow ui;
};
