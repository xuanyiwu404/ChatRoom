
#include "pch.h"
#include "Inc.h"
#include "ChatRoomDlg.h"
#include "resource.h"


DWORD WINAPI ConnectThreadFunc(LPVOID pParam)
{
	CChatRoomDlg* pchatroom = static_cast<CChatRoomDlg*>(pParam);
	ASSERT(pchatroom != nullptr);

	//�½�socket
	pchatroom->m_ConnectSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pchatroom->m_ConnectSock == INVALID_SOCKET)
	{
		AfxMessageBox(_T("�½�socketʧ�ܣ������ԣ�"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//��socket����Ϊ������
	u_long mode = 1;
	if (ioctlsocket(pchatroom->m_ConnectSock, FIONBIO, &mode) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("socket����Ϊnon_blockingʧ�ܣ�"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//����tcp_nodelay
	char flag = 1;
	if (setsockopt(pchatroom->m_ConnectSock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("socket����TCP_NOCELAYʧ�ܣ�"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//ץȡ�û��˿ڲ��ж�
	UINT uport = pchatroom->GetDlgItemInt(IDC_port_service);
	if (uport <= 0 || uport > 65536)
	{
		AfxMessageBox(_T("�˿ںŴ���1-65535"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}

	//ץȡIP��ַ��ת��
	CString strSerIP;
	pchatroom->GetDlgItemTextW(IDC_IP_service, strSerIP);
	/*char temp[16] = { 0 };   //ATL3.0д��
	USES_CONVERSION;
	strcpy_s(temp, 16, T2A(strSerIP));*/

	//�½�һ��sockaddr_in�Ľṹ�壬���ڽ����ض˿ں�IP��ַ��socket��
	sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(CT2A(strSerIP));
	client.sin_port = htons(uport);

	//connect���ӷ�����
	connect(pchatroom->m_ConnectSock, static_cast<sockaddr*>(static_cast<void*>(&client)), sizeof(client));
	if (!isSocketAlive(pchatroom->m_ConnectSock))
	{
		AfxMessageBox(_T("���ӷ�����ʧ�ܣ������ԣ�"));
		closesocket(pchatroom->m_ConnectSock);
		pchatroom->m_ConnectSock = INVALID_SOCKET;
		return false;
	}
	pchatroom->ShowMsg(_T("ϵͳ��ʾ�����ӷ������ɹ���"));

	//�ж�Ϊ�ͻ���
	pchatroom->bIsServer = 0;

	//ʹ������ѡ����
	pchatroom->Enable(IDC_B_clientconnectservice, false);

	//����selectģ����������շ���Ϣ
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
				pchatroom->ShowMsg(_T("�����ѶϿ��������ԣ�"));
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