#pragma once

#include <afxwin.h>

#include <string>

class CPlaceholderTab : public CWnd {
public:
    BOOL Create(CWnd* parent, const RECT& rect, UINT id, const std::wstring& text);
    void RefreshDpiLayout();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutControls();

private:
    std::wstring text_;
    CFont ui_font_;
    CStatic label_;
};
