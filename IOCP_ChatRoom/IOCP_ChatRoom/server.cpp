
#include "pch.h"
#include "resource.h"
#include "ChatRoomDlg.h"
#include "Inc.h"


//socket_select函数定义
bool Socket_Select(SOCKET hsocket, int nTimeout, bool bread)
{
	fd_set fdset;
	timeval tv;
	FD_ZERO(&fdset);
	FD_SET(hsocket, &fdset);
	nTimeout > 1000 ? 1000 : nTimeout;
	tv.tv_sec = 0;
	tv.tv_usec = nTimeout;

	int iRet = 0;
	if (bread)
	{
		iRet = select(0, &fdset, nullptr, nullptr, &tv);
	}
	else
	{
		iRet = select(0, nullptr, &fdset, nullptr, &tv);
	}
	if (iRet <= 0)
		return false;
	else if (FD_ISSET(hsocket, &fdset))
		return true;
	return false;
}



DWORD WINAPI ListenThreadFunc(LPVOID pParam)
{
	CChatRoomDlg* pchatroom = static_cast<CChatRoomDlg*>(pParam);
	ASSERT(pchatroom != nullptr);

	//新建socket
	pchatroom->m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pchatroom->m_ListenSock == INVALID_SOCKET)
	{
		AfxMessageBox(_T("新建socket失败！"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//读取文本输入的服务器端口号
	UINT uport = pchatroom->GetDlgItemInt(IDC_port_client);
	if (uport <= 0 || uport > 65535)
	{
		AfxMessageBox(_T("端口号应大于0且小于65536"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//新建一个sockaddr_in的结构体，用于将本地端口和IP地址与socket绑定
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(uport);

	//bind绑定
	int iResult = 0;
	void* p = static_cast<void*>(&service);//显示类型转换，由于sockaddr*和sockaddr_in结构体不同，必须经过void*作为媒介
	sockaddr* a = static_cast<sockaddr*>(p);
	iResult = bind(pchatroom->m_ListenSock, a, sizeof(service));
	if (iResult == SOCKET_ERROR)
	{
		AfxMessageBox(_T("绑定端口失败！"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//listen端口
	if (listen(pchatroom->m_ListenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("监听失败！"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//判断为服务端
	pchatroom->bIsServer = 1;

	//使开启服务器选项变灰
	pchatroom->Enable(IDC_B_servicestartservice, false);

	//利用select模型accept客户端端口,返回socket存储客户端端口数据
	while (!pchatroom->bShutDown)
	{
		if (Socket_Select(pchatroom->m_ListenSock, 100, true))
		{
			sockaddr_in clientaddr;
			int iLen = sizeof(clientaddr);
			SOCKET AcceptSock = accept(pchatroom->m_ListenSock, static_cast<sockaddr*>(static_cast<void*>(&clientaddr)), &iLen);
			if (AcceptSock == INVALID_SOCKET)
			{
				AfxMessageBox(_T("接收客户端失败，正在重试！"));
				closesocket(AcceptSock);
				continue;
			}

			//新建socket用于随时从客户端接收消息
			CClientItem tItem;
			tItem.m_Socket = AcceptSock;
			tItem.m_strIP = inet_ntoa(clientaddr.sin_addr);
			tItem.m_pMain_Wnd = pchatroom;
			INT_PTR idx = pchatroom->m_ClientArray.Add(tItem);
			tItem.hThread = CreateThread(nullptr, 0, ClientThreadProc, &pchatroom->m_ClientArray.GetAt(idx), CREATE_SUSPENDED, nullptr);
			if (!tItem.hThread)
			{
				pchatroom->m_ClientArray.RemoveAt(idx);
			}
			else
			{
				pchatroom->m_ClientArray.GetAt(idx).hThread = tItem.hThread;
				ResumeThread(tItem.hThread);
				pchatroom->ShowMsg(_T("客户端：") + tItem.m_strIP + _T("进入了聊天室！"));
				pchatroom->SendClientMsg(_T("客户端：") + tItem.m_strIP + _T("进入了聊天室！"), &tItem);
			}
			Sleep(100);
		}
	}
	closesocket(pchatroom->m_ListenSock);
	pchatroom->m_ListenSock = INVALID_SOCKET;
	ExitThread(-1);
	pchatroom->m_hListenThread = nullptr;
	pchatroom->bIsServer = -1;
	pchatroom->bShutDown = false;
	return true;
}

DWORD WINAPI ClientThreadProc(LPVOID pParam)
{
	CString strMsg;
	CClientItem m_ClientItem = *static_cast<CClientItem*>(pParam);
	while(!m_ClientItem.m_pMain_Wnd->bShutDown)
	{
		if (Socket_Select(m_ClientItem.m_Socket, 100, true))
		{
			TCHAR szbuf[MAX_BUF_SIZE] = { 0 };
			if (isSocketAlive(m_ClientItem.m_Socket) && recv(m_ClientItem.m_Socket, static_cast<char*>(static_cast<void*>(szbuf)), MAX_BUF_SIZE, 0))
			{
				strMsg = _T("客户端：") + m_ClientItem.m_strIP + _T("<<") + szbuf;
				m_ClientItem.m_pMain_Wnd->ShowMsg(strMsg);
				m_ClientItem.m_pMain_Wnd->SendClientMsg(strMsg, &m_ClientItem);
			}
			else			
			{
				strMsg = _T("客户端：") + m_ClientItem.m_strIP + _T("离开了聊天室！");
				m_ClientItem.m_pMain_Wnd->ShowMsg(strMsg);
				m_ClientItem.m_pMain_Wnd->SendClientMsg(strMsg);
				SOCKET temp = m_ClientItem.m_Socket;
				m_ClientItem.m_pMain_Wnd->RemoveClientFromArray(m_ClientItem);
				temp = INVALID_SOCKET;
				break;
			}
			
			Sleep(500);
		}
	}
	ExitThread(-1);
	return true;
}

bool isSocketAlive(SOCKET s)
{
	if (send(s, "", 0, 0) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}