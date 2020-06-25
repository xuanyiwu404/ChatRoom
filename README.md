# ChatRoom
c++打造网络聊天室，对并发量优化

----------------------------------------------------------------------------------------------------------------------------------
其中ChatRoom是按照
B站https://www.bilibili.com/video/BV1qJ411c7P5?from=search&seid=15679071508762867539编写；

使用技术：该项目基于Windows平台上的MFC类库，并使用Select模型+多线程，后改进为IOCP模型。
项目描述：该项目类似于QQ或微信的群聊，基于C/S架构。
主要功能：程序集服务器端与客户端于一体，服务器端接收客户端发来的消息，并转发给其他客户端，并能显示是由哪个客户端发来等等。
我的工作：
  利用MFC类库，设计并制作程序的交互界面，实现信息发送与显示，连接服务器端、监听客户端等工作；
  使用Select网络IO模型+多线程，实现服务器端非阻塞地监听并接受客户端消息，并发量在100以内；

----------------------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------------------
IOCP_ChatRoom为改进后版本，利用IOCP模型进行改进，暂实现1500的并发量（接收数据）。

参考链接：https://blog.csdn.net/piggyxp/article/details/6922277
----------------------------------------------------------------------------------------------------------------------------------
第二次更新：
将IOCP_ChatRoom项目中的sln文件更新，通过打开sln文件，可以直接打开项目；
项目中使用了一个winsock以前的函数，现在已不支持，debug没问题，但是release会报错，
解决措施：菜单栏 -> 项目(P) -> “项目名”属性(P)… -> 项目属性页 -> 配置属性 -> C/C++ ->常规 -> SDL 检查（改为否）