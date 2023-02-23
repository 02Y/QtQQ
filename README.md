# QtQQ
Imitate QQ based on Qt development
*此项目使用 VS2017+Qt5.9.6
 使用到的Qt模块：core;gui;network;sql;webchannel;webengine;webenginewidgets;widgets;xml

需处理优化项：

    1.表情不能与文本一起发送（可以尝试修改解析）：void TalkWindowShell::processPendingData()
  
    2.绑定端口太浪费资源，需优化：客户端——void TalkWindowShell::initUdpSocket()      服务端——void QtQQ_Server::onUDPbroadMsg(QByteArray& btData)
  
    3.解析规则太浪费字节数，需优化：void TalkWindowShell::processPendingData()    可以与问题1.进行综合调整、重新设计等
  
    4.文件发送窗口右上角最大化、最小化按钮没有连接信号与槽，不能使用
  
    5.因聊天信息放在网页，右键信息显示区会显示网页相关信息（如HTMl源码）
  
    6.个性签名未设置（从数据库读，写入数据库）
  
    7.不能在登陆后主界面设置头像（可以通过数据库修改，但没有设计接口）
  
    8.用户等级按时间逐渐递增（如有其他方案可调整）
  
    9.主窗口界面小程序未实现功能（需设计接口）
  
    10.主窗口搜索框未实现功能
  
    11.登陆窗口记住密码功能未实现（可以将账号密码写入配置文件）
  
    12.登陆窗口头像未从数据库读取，始终是qqlogo
    

