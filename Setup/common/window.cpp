#include "Window.h"

CDuiString Window::GetSkinFile()
{
	return "106";
}

CDuiString Window::GetSkinFolder()
{
	return "";
}

LPCTSTR Window::GetWindowClassName(void) const
{
	return "Window";
}


void Window::InitWindow()
{
	wizardTab = dynamic_cast<CTabLayoutUI*>(m_pm.FindControl("wizardTab"));
	closeBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnClose"));
	closeBtn->OnNotify += MakeDelegate(this, &Window::CloseWindow);

	minBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnMin"));
	minBtn->OnNotify += MakeDelegate(this, &Window::MinimizedWindow);

	doneBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnDone"));
	doneBtn->OnNotify += MakeDelegate(this, &Window::CloseWindow);
}

