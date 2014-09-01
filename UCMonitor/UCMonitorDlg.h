
// UCMonitorDlg.h : header file
//

#pragma once


// CUCMonitorDlg dialog
class CUCMonitorDlg : public CDialogEx
{
// Construction
public:
	CUCMonitorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_UCMONITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	NOTIFYICONDATA m_nid;

protected:
	afx_msg LRESULT OnShowtask(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL DestroyWindow();
};
