
// AlarmMachineConfigToolDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "SerialPort.h"
#include "GenericBuffer.h"
#include <list>

enum ResponceData
{
	RD_NULL = 0x00,		// 无按键动作
	RD_1,				// 1~9
	RD_2,
	RD_3,
	RD_4,
	RD_5,
	RD_6,
	RD_7,
	RD_8,
	RD_9,
	RD_0,				// 0
	RD_ASTERISK,		// *
	RD_SIGN,			// #
	RD_SECURE_ALL,		// 全布防
	RD_SECURE_HALF,		// 半布防
	RD_SECURE_CANCEL,	// 撤防
	RD_ALARM,			// 报警
	RD_RECORD,			// 录音
	RD_PLAY,			// 放音
	RD_STOP,			// 停止
	RD_PLACEHOLDER,		// 占位符，永远不用
	//RD_RETRIEVE,		// 探头信息索要
	RD_MUTE_ONCE = 0x16,		// 影音一次
	RD_MAX
};

inline static ResponceData DecToRd(int intVar)
{
	if (intVar > RD_0)
		return RD_MAX;
	if (intVar == 0)
		return RD_0;
	return static_cast<ResponceData>(intVar);
}

typedef struct Task
{
	char* buff;
	size_t len;
	Task() : buff(NULL), len(0) {}
	Task(const char* cmd, size_t l) { buff = new char[l]; memcpy(buff, cmd, l); len = l; }
	~Task() { if (buff) { delete[] buff; buff = NULL; } }
	Task& operator=(const Task& rhs)
	{
		if (rhs.buff && rhs.len > 0) {
			buff = new char[rhs.len];
			memcpy(buff, rhs.buff, rhs.len);
			len = rhs.len;
		} else {
			buff = NULL; len = 0;
		}
		return *this;
	}
}Task;

typedef std::list<Task*> TaskList;

typedef enum Status{
	UNKNOWN,
	CONFIG,
	DUIMA,
}Status;


class CContainerDlg;
// CAlarmMachineConfigToolDlg 对话框
class CAlarmMachineConfigToolDlg : public CDialogEx, public CSerialPort
{
// 构造
public:
	CAlarmMachineConfigToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ALARMMACHINECONFIGTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private:
	CLock m_lock;
	TaskList m_taskList;
	Status m_status;
// 实现
protected:
	HICON m_hIcon;
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);
	virtual		void OnConnectionEstablished();
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	HANDLE m_hEventExit;
	HANDLE m_hThreadDeal;
	void AddTask(Task* task) { m_lock.Lock(); m_taskList.push_back(task); m_lock.UnLock(); }
	Task* GetTask() { if (m_lock.TryLock()) { if (m_taskList.size() > 0) { Task* task = m_taskList.front(); m_taskList.pop_front(); m_lock.UnLock(); return task; }m_lock.UnLock(); }return NULL; }

	CGenericBuffer m_buff;
	CGenericBuffer m_sendbuff;
	//CContainerDlg* m_container;
	static DWORD WINAPI ThreadDeal(LPVOID lp);
	void UpdateStatus(Status status);
	void Input(unsigned char cmd);
	void Input(const char* cmd, size_t len);
	void SendQuitConfig();
public:
	afx_msg void OnBnClickedOk();
	CComboBox m_cmbCom;
	CButton m_btnOpenClose;
	afx_msg void OnBnClickedButtonOpenClose();
	afx_msg void OnDestroy();
	CStatic m_staticGroup;
	afx_msg void OnBnClickedButtonConfigOrQuit();
	CButton m_btnConfigOrQuitConfig;
	afx_msg void OnBnClickedCheck1();
	CButton m_chkPhoneAlarm;
};
