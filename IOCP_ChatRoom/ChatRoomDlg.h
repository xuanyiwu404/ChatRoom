
// ChatRoomDlg.h: 头文件
//

#pragma once
#include "Inc.h"
#include "IOCP_SERVER.h"

class CClientItem;

// CChatRoomDlg 对话框
class CChatRoomDlg : public CDialogEx
{
// 构造
public:
	CChatRoomDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
private:
	bool bdall = false;

protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void AllDlgInit();
	bool Enable(int nID, bool enable = true);
	afx_msg void OnBnClickedBclientconnectservice();
	afx_msg void OnBnClickedRservice();
	afx_msg void OnBnClickedBservicestartservice();
	afx_msg void OnBnClickedRclient();
	afx_msg void OnBnClickedBclientcease();
	afx_msg void OnBnClickedBservicecease();
	void SetDlgAll(bool bdlgall);
	afx_msg void OnBnClickedBnetset();
	HANDLE m_hListenThread;
	HANDLE m_hConnectThread;
	CArray<CClientItem, CClientItem>m_ClientArray;
	void ShowMsg(CString strMsg);
	CEdit m_MsgShow;
	void RemoveClientFromArray(CClientItem in_Item);
	void SendClientMsg(CString strMsg, CClientItem* pNotSend = nullptr);
	int bIsServer;
	afx_msg void OnBnClickedBsendmsg();
	SOCKET m_ConnectSock;
	SOCKET m_ListenSock;
	afx_msg void OnEnChangesendmsg();
	bool bShutDown;
	void StopClient();
	void StopServer();
	afx_msg void OnBnClickedCancel();
	CIOCPModel m_IOCP;
};