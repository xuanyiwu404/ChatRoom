
#pragma once
#include "pch.h"
#include "ChatRoomDlg.h"


class CChatRoomDlg;

class CClientItem {
public:
	CString m_strIP;
	SOCKET m_Socket;
	HANDLE hThread;
	CChatRoomDlg* m_pMain_Wnd;
	CClientItem() {
		m_Socket = INVALID_SOCKET;
		hThread = nullptr;
		m_pMain_Wnd = nullptr;
	}
	bool operator ==(const CClientItem& a);
};

inline bool CClientItem::operator==(const CClientItem& a)
{
	if (m_strIP == a.m_strIP && m_Socket == a.m_Socket && hThread == a.hThread)
		return true;
	else
		return false;
}


DWORD WINAPI ListenThreadFunc(LPVOID pParam);
DWORD WINAPI ClientThreadProc(LPVOID pParam);
DWORD WINAPI ConnectThreadFunc(LPVOID pParam);
const  int MAX_BUF_SIZE = 1024;
bool Socket_Select(SOCKET hsocket, int nTimeout = 100, bool bread = true);
bool isSocketAlive(SOCKET s);