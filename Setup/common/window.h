#pragma once
#include "UIlib.h"
#include <string>

using namespace DuiLib;

class Window :public WindowImplBase {

protected:
	 virtual CDuiString GetSkinType() { return _T("xml"); }
	 virtual CDuiString GetSkinFile() override;
	 virtual CDuiString GetSkinFolder();
	 virtual LPCTSTR GetWindowClassName(void) const override;
	 virtual void InitWindow();

public:
	 inline virtual void Show(const string& title)
	 {
		 HWND hwnd = Create(NULL, title.c_str(), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		 //禁止拖拽窗口到屏幕上边沿最大化
		 SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME);
		 CenterWindow();
		 ShowModal();
	 }

	inline bool IsClick(void* param) {
		TNotifyUI* msg = reinterpret_cast<TNotifyUI*>(param);
		return msg->sType == DUI_MSGTYPE_CLICK;
	}

	inline void ToPage(int index)
	{
		wizardTab->SelectItem(index);
	}

	inline void SetCloseBtnEnabled(bool bEnable) 
	{
		closeBtn->SetEnabled(bEnable);
	}

	inline bool CloseWindow(void* param)
	{
		if (!IsClick(param)) return false;
		Close();
		return true;
	}

	inline bool MinimizedWindow(void* param)
	{
		if (!IsClick(param)) return false;
		::SendMessage(GetHWND(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
		return true;
	}
	
private:

	CTabLayoutUI* wizardTab;
	CButtonUI* closeBtn;//关闭按钮
	CButtonUI* minBtn;//最小化按钮

	CButtonUI* doneBtn;
};