#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>

enum ButtonType
{
	MIN_BUTTON = 0,     //最小化及关闭按钮
	MIN_MAX_BUTTON,     //最小化、最大化及关闭按钮
	ONLY_CLOSE_BUTTON   //只有关闭按钮
};

//自定义标题栏
class TitleBar : public QWidget
{
	Q_OBJECT

public:
	TitleBar(QWidget* parent = nullptr);
	~TitleBar();

	void setTitleIcon(const QString& filePath);            //设置标题栏图标
	void setTitleContent(const QString& titleContent);     //设置标题栏内容
	void setTitleWidth(int width);                   //设置标题栏长度
	void setButtonType(ButtonType buttonType);       //设置标题栏按钮类型

	void saveRestoreInfo(const QPoint& point, const QSize& size);       //保存窗口最大化前窗口的位置及大小
	void getRestoreInfo(QPoint& point, QSize& size);                    //获取窗口最大化前窗口的位置及大小

private:
	//重写事件
	void paintEvent(QPaintEvent* event);                  //绘图事件
	void mouseDoubleClickEvent(QMouseEvent* event);       //鼠标双击事件
	void mousePressEvent(QMouseEvent* event);             //鼠标按下事件
	void mouseMoveEvent(QMouseEvent* event);              //鼠标移动事件
	void mouseReleaseEvent(QMouseEvent* event);           //鼠标释放事件

	void initControl();          //初始化控件
	void initConnections();      //初始化信号与槽的连接
	void loadStyleSheet(const QString& sheetName);        //加载样式表

signals:
	//按钮发射的信号
	void signalButtonMinClicked();          //最小化按钮
	void signalButtonRestoreClicked();      //最大化还原按钮
	void signalButtonMaxClicked();          //最大化按钮
	void signalButtonCloseClicked();        //关闭按钮

private slots:
	//按钮响应的槽
	void onButtonMinClicked();          //最小化按钮响应的槽
	void onButtonRestoreClicked();      //最大化还原按钮响应的槽
	void onButtonMaxClicked();          //最大换按钮响应的槽
	void onButtonCloseClicked();        //关闭按钮响应的槽

private:
	QLabel* m_pIcon;                    //标题栏图标
	QLabel* m_pTitleContent;            //标题栏内容
	QPushButton* m_pButtonMin;          //最小化按钮
	QPushButton* m_pButtonRestore;      //最大化还原按钮
	QPushButton* m_pButtonMax;          //最大化按钮
	QPushButton* m_pButtonClose;        //关闭按钮

	//最大化还原按钮变量（用于保存窗体位置及大小）
	QPoint m_restorePos;            //窗体位置
	QSize m_restoreSize;            //窗体大小

	//移动窗口变量
	bool m_isPressed;               //是否被点击
	QPoint m_startMovePos;           //开始移动的点

	QString m_titleContent;         //标题栏内容
	ButtonType m_buttonType;        //标题栏右上角按钮类型
};