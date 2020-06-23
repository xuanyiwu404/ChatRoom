
#include "pch.h"
#include "resource.h"
#include "ChatRoomDlg.h"
#include "Inc.h"


//socket_select��������
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

	//�½�socket
	pchatroom->m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pchatroom->m_ListenSock == INVALID_SOCKET)
	{
		AfxMessageBox(_T("�½�socketʧ�ܣ�"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//��ȡ�ı�����ķ������˿ں�
	UINT uport = pchatroom->GetDlgItemInt(IDC_port_client);
	if (uport <= 0 || uport > 65535)
	{
		AfxMessageBox(_T("�˿ں�Ӧ����0��С��65536"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//�½�һ��sockaddr_in�Ľṹ�壬���ڽ����ض˿ں�IP��ַ��socket��
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(uport);

	//bind��
	int iResult = 0;
	void* p = static_cast<void*>(&service);//��ʾ����ת��������sockaddr*��sockaddr_in�ṹ�岻ͬ�����뾭��void*��Ϊý��
	sockaddr* a = static_cast<sockaddr*>(p);
	iResult = bind(pchatroom->m_ListenSock, a, sizeof(service));
	if (iResult == SOCKET_ERROR)
	{
		AfxMessageBox(_T("�󶨶˿�ʧ�ܣ�"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//listen�˿�
	if (listen(pchatroom->m_ListenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("����ʧ�ܣ�"));
		closesocket(pchatroom->m_ListenSock);
		pchatroom->m_ListenSock = INVALID_SOCKET;
		return false;
	}

	//�ж�Ϊ�����
	pchatroom->bIsServer = 1;

	//ʹ����������ѡ����
	pchatroom->Enable(IDC_B_servicestartservice, false);

	//����selectģ��accept�ͻ��˶˿�,����socket�洢�ͻ��˶˿�����
	while (!pchatroom->bShutDown)
	{
		if (Socket_Select(pchatroom->m_ListenSock, 100, true))
		{
			sockaddr_in clientaddr;
			int iLen = sizeof(clientaddr);
			SOCKET AcceptSock = accept(pchatroom->m_ListenSock, static_cast<sockaddr*>(static_cast<void*>(&clientaddr)), &iLen);
			if (AcceptSock == INVALID_SOCKET)
			{
				AfxMessageBox(_T("���տͻ���ʧ�ܣ��������ԣ�"));
				closesocket(AcceptSock);
				continue;
			}

			//�½�socket������ʱ�ӿͻ��˽�����Ϣ
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
				pchatroom->ShowMsg(_T("�ͻ��ˣ�") + tItem.m_strIP + _T("�����������ң�"));
				pchatroom->SendClientMsg(_T("�ͻ��ˣ�") + tItem.m_strIP + _T("�����������ң�"), &tItem);
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
				strMsg = _T("�ͻ��ˣ�") + m_ClientItem.m_strIP + _T("<<") + szbuf;
				m_ClientItem.m_pMain_Wnd->ShowMsg(strMsg);
				m_ClientItem.m_pMain_Wnd->SendClientMsg(strMsg, &m_ClientItem);
			}
			else			
			{
				strMsg = _T("�ͻ��ˣ�") + m_ClientItem.m_strIP + _T("�뿪�������ң�");
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