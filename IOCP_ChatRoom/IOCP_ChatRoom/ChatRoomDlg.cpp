
// ChatRoomDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ChatRoom.h"
#include "ChatRoomDlg.h"
#include "afxdialogex.h"
#include "IOCP_SERVER.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChatRoomDlg 对话框



CChatRoomDlg::CChatRoomDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hListenThread = nullptr;
	m_hConnectThread = nullptr;
	bIsServer = -1;
	bShutDown = false;
	m_ListenSock = INVALID_SOCKET;
	m_ConnectSock = INVALID_SOCKET;
}

void CChatRoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_acc_msg, m_MsgShow);
}

BEGIN_MESSAGE_MAP(CChatRoomDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_B_clientconnectservice, &CChatRoomDlg::OnBnClickedBclientconnectservice)
	ON_BN_CLICKED(IDC_R_service, &CChatRoomDlg::OnBnClickedRservice)
	ON_BN_CLICKED(IDC_B_servicestartservice, &CChatRoomDlg::OnBnClickedBservicestartservice)
	ON_BN_CLICKED(IDC_R_client, &CChatRoomDlg::OnBnClickedRclient)
	ON_BN_CLICKED(IDC_B_clientcease, &CChatRoomDlg::OnBnClickedBclientcease)
	ON_BN_CLICKED(IDC_B_servicecease, &CChatRoomDlg::OnBnClickedBservicecease)
	ON_BN_CLICKED(IDC_B_netset, &CChatRoomDlg::OnBnClickedBnetset)
	ON_BN_CLICKED(IDC_B_sendmsg, &CChatRoomDlg::OnBnClickedBsendmsg)
	ON_EN_CHANGE(IDC_send_msg, &CChatRoomDlg::OnEnChangesendmsg)
	ON_BN_CLICKED(IDCANCEL, &CChatRoomDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CChatRoomDlg 消息处理程序

BOOL CChatRoomDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标



	CEdit* p_Edit;
	p_Edit = (CEdit*)GetDlgItem(IDC_acc_msg);
	p_Edit->SetLimitText(UINT_MAX);//设置显示最大字符数


	// TODO: 在此添加额外的初始化代码

	AllDlgInit();
	SetDlgAll(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CChatRoomDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChatRoomDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChatRoomDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CChatRoomDlg::AllDlgInit()
{
	CheckRadioButton(IDC_R_client, IDC_R_service, IDC_R_client);  //从单选框范围内选中某个单选框
	SetDlgItemText(IDC_IP_service, _T("127.0.0.1"));  //设置窗口文本
	SetDlgItemText(IDC_port_service, _T("65530"));
	SetDlgItemText(IDC_port_client, _T("65530"));
	Enable(IDC_B_clientcease, false);
	Enable(IDC_STATIC3, false);
	Enable(IDC_port_client, false);
	Enable(IDC_B_servicestartservice, false);
	Enable(IDC_B_servicecease, false);
	ShowMsg(_T("系统提示：程序启动成功！"));
	Enable(IDC_B_sendmsg, false);
	// 绑定主界面指针(为了方便在界面中显示信息 )
	m_IOCP.SetMainDlg(this);
}

bool CChatRoomDlg::Enable(int nID, bool enable)
{
	return GetDlgItem(nID)->EnableWindow(enable);
}


void CChatRoomDlg::OnBnClickedRclient()
{
	// TODO: 在此添加控件通知处理程序代码
	int iRet = -1;
	if (bIsServer == 1)
	{
		iRet = AfxMessageBox(_T("您是聊天室的服务器端，如退出，所有客户端都将掉线，是否退出？"), MB_OKCANCEL, 0);
		if (iRet == IDOK)
		{
			StopServer();
		}
	}
	if (iRet == IDOK || bIsServer == -1)
	{
		Enable(IDC_STATIC1);
		Enable(IDC_IP_service);
		Enable(IDC_STATIC2);
		Enable(IDC_port_service);
		Enable(IDC_B_clientconnectservice);
		Enable(IDC_B_clientcease, false);
		Enable(IDC_STATIC3, false);
		Enable(IDC_port_client, false);
		Enable(IDC_B_servicestartservice, false);
		Enable(IDC_B_servicecease, false);
		SetDlgItemText(IDC_IP_service, _T("127.0.0.1"));  //设置窗口文本
		SetDlgItemText(IDC_port_service, _T("65530"));
	}
}



void CChatRoomDlg::OnBnClickedBclientconnectservice()
{
	// TODO: 在此添加控件通知处理程序代码

	//新建thread连接服务器
	m_hConnectThread = CreateThread(nullptr, 0, ConnectThreadFunc, this, 0, nullptr);
	if (!m_hConnectThread)
	{
		AfxMessageBox(_T("客户端启用失败，请重试！"));
	}

	//客户端选项中点击连接服务器选项后停止选项可用
	if (GetCheckedRadioButton(IDC_R_client, IDC_R_service) == 1007 && m_hConnectThread)
		Enable(IDC_B_clientcease);
}


void CChatRoomDlg::OnBnClickedBclientcease()
{
	// TODO: 在此添加控件通知处理程序代码
	if (GetCheckedRadioButton(IDC_R_client, IDC_R_service) == 1007)
	{
		int iRet = AfxMessageBox(_T("是否断开与服务器的连接？"), MB_OKCANCEL, 0);
		if (iRet == IDOK)
		{
			StopClient();
			ShowMsg(_T("已断开与服务器连接!"));
		}
	}
	OnBnClickedRclient();
}


void CChatRoomDlg::OnBnClickedRservice()
{
	// TODO: 在此添加控件通知处理程序代码
	//点击服务器选项后
	int iRet = -1;
	if (bIsServer == 0)
	{
		iRet = AfxMessageBox(_T("您正在聊天室中，是否退出？"), MB_OKCANCEL, 0);
		if (iRet == IDOK)
		{
			StopClient();
		}
	}
	if (iRet == IDOK || bIsServer == -1)
	{
		Enable(IDC_STATIC1, false);
		Enable(IDC_IP_service, false);
		Enable(IDC_STATIC2, false);
		Enable(IDC_port_service, false);
		Enable(IDC_B_clientconnectservice, false);
		Enable(IDC_B_clientcease, false);
		Enable(IDC_STATIC3);
		Enable(IDC_port_client);
		Enable(IDC_B_servicestartservice);
		Enable(IDC_B_servicecease, false);
		SetDlgItemText(IDC_port_client, _T("65530"));

	}
}


void CChatRoomDlg::OnBnClickedBservicestartservice()
{
	// TODO: 在此添加控件通知处理程序代码
	//新建线程用于监听客户端请求
	if (false == m_IOCP.Start())
	{
		AfxMessageBox(_T("服务器启动失败！"));
		return;
	}
	//
	
	//点击服务器选项中的开启服务器后
	if (GetCheckedRadioButton(IDC_R_client, IDC_R_service) == 1008 && m_hListenThread)
	{
		ShowMsg(_T("系统提示：服务器启用成功！"));
		Enable(IDC_B_servicecease);
	}
}



void CChatRoomDlg::OnBnClickedBservicecease()
{
	// TODO: 在此添加控件通知处理程序代码
	if (GetCheckedRadioButton(IDC_R_client, IDC_R_service) == 1008)
	{
		int iRet = AfxMessageBox(_T("是否断开与所有客户端的连接？"), MB_OKCANCEL, 0);
		if (iRet == IDOK)
		{
			m_IOCP.Stop();
			ShowMsg(_T("已断开与所有客户端的连接!"));
		}
		OnBnClickedRservice();
	}
}


void CChatRoomDlg::SetDlgAll(bool bdlgall)
{
	static CRect S_WindowLarge(0, 0, 0, 0);
	static CRect S_WindowSmall(0, 0, 0, 0);
	static CRect S_GroupLarge(0, 0, 0, 0);
	static CRect S_GroupSmall(0, 0, 0, 0);
	if (S_WindowLarge.IsRectNull())
	{
		GetWindowRect(S_WindowLarge);
		::GetWindowRect(GetDlgItem(IDC_STATIC8)->GetSafeHwnd(), S_GroupLarge);
	}
	S_WindowSmall = S_WindowLarge;
	S_WindowSmall.right -= 350;
	S_GroupSmall = S_GroupLarge;
	S_GroupSmall.right -= 350;
	if (bdlgall)
	{
		bdall = true;
		SetWindowPos(&wndTop, S_WindowLarge.left, S_WindowLarge.top, S_WindowLarge.Width(), S_WindowLarge.Height(), SWP_NOMOVE);
		::SetWindowPos(GetDlgItem(IDC_STATIC8)->GetSafeHwnd(), nullptr, S_GroupLarge.left, S_GroupLarge.top, S_GroupLarge.Width(), S_GroupLarge.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
	else
	{
		bdall = false;
		SetWindowPos(&wndTop, S_WindowSmall.left, S_WindowSmall.top, S_WindowSmall.Width(), S_WindowSmall.Height(), SWP_NOMOVE);
		::SetWindowPos(GetDlgItem(IDC_STATIC8)->GetSafeHwnd(), nullptr, S_GroupSmall.left, S_GroupSmall.top, S_GroupSmall.Width(), S_GroupSmall.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CChatRoomDlg::OnBnClickedBnetset()
{
	// TODO: 在此添加控件通知处理程序代码
	//点击网络设置
	if (bdall)
		SetDlgAll(false);
	else
		SetDlgAll(true);
}


//用于显示聊天记录
void CChatRoomDlg::ShowMsg(CString strMsg)
{
	m_MsgShow.SetSel(-1, -1);
	m_MsgShow.ReplaceSel(strMsg + _T("\r\n"));
}

//删除消息队列中的已断开客户端
void CChatRoomDlg::RemoveClientFromArray(CClientItem in_Item)
{
	for (INT_PTR i = 0; i < m_ClientArray.GetCount(); ++i)
	{
		CClientItem temp = m_ClientArray.GetAt(i);
		if (temp == in_Item)
			m_ClientArray.RemoveAt(i);
	}
}

//发送消息
void CChatRoomDlg::SendClientMsg(CString strMsg, CClientItem* pNotSend)
{
	TCHAR szbuf[MAX_BUF_SIZE] = { 0 };
	_tcscpy_s(szbuf, MAX_BUF_SIZE, strMsg);
	INT_PTR temp = m_ClientArray.GetCount();
	for (INT_PTR i = 0; i < temp; ++i)
	{
		if (!pNotSend || m_ClientArray.GetAt(i).hThread != pNotSend->hThread ||
			m_ClientArray.GetAt(i).m_Socket != pNotSend->m_Socket || m_ClientArray.GetAt(i).m_strIP != pNotSend->m_strIP)
		{
			if (send(m_ClientArray.GetAt(i).m_Socket, static_cast<char*>(static_cast<void*>(szbuf)), _tcslen(szbuf) * sizeof(TCHAR), 0) == SOCKET_ERROR)
			{
				continue;
			}
		}
	}
}

void CChatRoomDlg::OnBnClickedBsendmsg()
{
	// TODO: 在此添加控件通知处理程序代码
	//点击发送信息
	CString strMsg;
	GetDlgItemText(IDC_send_msg, strMsg);
	if (bIsServer == 1)
	{
		strMsg = _T("服务器：<<") + strMsg;
		ShowMsg(strMsg);
		SendClientMsg(strMsg);
	}
	else if (bIsServer == 0)
	{
		ShowMsg(_T("本地客户端：<<") + strMsg);
		send(m_ConnectSock, (char*)strMsg.GetBuffer(), strMsg.GetLength() * sizeof(TCHAR), 0);
		strMsg.ReleaseBuffer();
	}
	else
	{
		AfxMessageBox(_T("请先进入聊天室"));
	}
	SetDlgItemText(IDC_send_msg, _T(""));
;}


void CChatRoomDlg::OnEnChangesendmsg()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString strMsg;
	GetDlgItemText(IDC_send_msg, strMsg);
	if (strMsg == _T(""))
	{
		Enable(IDC_B_sendmsg, false);
	}
	else
		Enable(IDC_B_sendmsg);
}

void CChatRoomDlg::StopClient()
{
	bShutDown = true;
	DWORD iRet = WaitForSingleObject(m_hConnectThread, 1500);
	if (iRet != WAIT_OBJECT_0)
	{
		TerminateThread(m_hConnectThread, -1);
		closesocket(m_ConnectSock);
	}
	m_ConnectSock = INVALID_SOCKET;
	bIsServer = -1;
	m_hConnectThread = nullptr;
	bShutDown = false;
}

void CChatRoomDlg::StopServer()
{
	UINT icount= m_ClientArray.GetCount();
	HANDLE* m_closethread = new HANDLE[icount + 1];
	m_closethread[0] = m_hListenThread;
	for (unsigned int i = 0; i < icount; ++i)
	{
		m_closethread[i + 1] = m_ClientArray.GetAt(i).hThread;
	}
	bShutDown = true;
	DWORD iRet = WaitForMultipleObjects(icount + 1, m_closethread, true, 6000);
	if (iRet == WAIT_OBJECT_0)
	{
		for (INT_PTR i = 0; i < m_ClientArray.GetCount(); ++i)
		{
			TerminateThread(m_ClientArray.GetAt(i).hThread, -1);
			closesocket(m_ClientArray.GetAt(i).m_Socket);
		}
		TerminateThread(m_hListenThread, -1);
		closesocket(m_ListenSock);
	}
	delete[] m_closethread;
	m_hListenThread = nullptr;
	m_ListenSock = INVALID_SOCKET;
	bIsServer = -1;
	bShutDown = false;
}


void CChatRoomDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (bIsServer == 1)
		StopServer();
	else if (bIsServer == 0)
		StopClient();
	CDialogEx::OnCancel();
}
