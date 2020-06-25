
#include "pch.h"
#include "Inc.h"
#include "ChatRoomDlg.h"
#include "resource.h"


DWORD WINAPI ConnectThreadFunc(LPVOID pParam)
{
	CChatRoomDlg* pchatroom = static_cast<CChatRoomDlg*>(pParam);
	ASSERT(pchatroom != nullptr);

	//新建socket
	pchatroom->m_ConnectSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pchatroom->m_ConnectSock == INVALID_SOCKET)
	{
		AfxMessageBox(_T("新建socket失败，请重试！"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//将socket设置为非阻塞
	u_long mode = 1;
	if (ioctlsocket(pchatroom->m_ConnectSock, FIONBIO, &mode) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("socket设置为non_blocking失败！"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//设置tcp_nodelay
	char flag = 1;
	if (setsockopt(pchatroom->m_ConnectSock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("socket设置TCP_NOCELAY失败！"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//抓取用户端口并判断
	UINT uport = pchatroom->GetDlgItemInt(IDC_port_service);
	if (uport <= 0 || uport > 65536)
	{
		AfxMessageBox(_T("端口号错误：1-65535"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//抓取IP地址并转换
	CString strSerIP;
	pchatroom->GetDlgItemTextW(IDC_IP_service, strSerIP);
	/*char temp[16] = { 0 };   //ATL3.0写法
	USES_CONVERSION;
	strcpy_s(temp, 16, T2A(strSerIP));*/

	//新建一个sockaddr_in的结构体，用于将本地端口和IP地址与socket绑定
	sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(CT2A(strSerIP));
	client.sin_port = htons(uport);

	//connect连接服务器
	connect(pchatroom->m_ConnectSock, static_cast<sockaddr*>(static_cast<void*>(&client)), sizeof(client));
	if (!isSocketAlive(pchatroom->m_ConnectSock))
	{
		AfxMessageBox(_T("连接服务器失败，请重试！"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}
	pchatroom->ShowMsg(_T("系统提示：连接服务器成功！"));

	//判断为客户端
	pchatroom->bIsServer = 0;

	//使服务器选项变灰
	pchatroom->Enable(IDC_B_clientconnectservice, false);

	//利用select模型与服务器收发信息
	while (!pchatroom->bShutDown)
	{
		if (Socket_Select(pchatroom->m_ConnectSock, 30))
		{
			TCHAR szbuf[MAX_BUF_SIZE] = { 0 };
			if (isSocketAlive(pchatroom->m_ConnectSock) && recv(pchatroom->m_ConnectSock, static_cast<char*>(static_cast<void*>(szbuf)), MAX_BUF_SIZE, 0))
			{
				pchatroom->ShowMsg(szbuf);
			}
			else
			{
				pchatroom->ShowMsg(_T("连接已断开，请重试！"));
				pchatroom->OnBnClickedRclient();
				break;
			}
		}
		Sleep(10);
	}
	closesocket(pchatroom->m_ConnectSock);
	pchatroom->bShutDown = false;
	pchatroom->bIsServer = -1;
	pchatroom->m_ConnectSock = INVALID_SOCKET;
	ExitThread(-1);
	return true;
}