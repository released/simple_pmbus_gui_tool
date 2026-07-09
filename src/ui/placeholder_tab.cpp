#include "placeholder_tab.h"

#include "layout_utils.h"

namespace {

enum : UINT {
    IDC_PLACEHOLDER_LABEL = 23000,
};

} // namespace

BEGIN_MESSAGE_MAP(CPlaceholderTab, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CPlaceholderTab::Create(CWnd* parent, const RECT& rect, UINT id, const std::wstring& text) {
    text_ = text;
    CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                      reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);
    return CWnd::CreateEx(0, cls, L"", WS_CHILD | WS_VISIBLE, rect, parent, id);
}

int CPlaceholderTab::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 95);
    label_.Create(text_.c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, CRect(), this, IDC_PLACEHOLDER_LABEL);
    label_.SetFont(&ui_font_);
    LayoutControls();
    return 0;
}

void CPlaceholderTab::RefreshDpiLayout() {
    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 95);
    if (::IsWindow(label_.GetSafeHwnd())) {
        label_.SetFont(&ui_font_);
    }
    LayoutControls();
}

void CPlaceholderTab::OnSize(UINT nType, int cx, int cy) {
    CWnd::OnSize(nType, cx, cy);
    (void)cx;
    (void)cy;
    LayoutControls();
}

void CPlaceholderTab::LayoutControls() {
    if (!::IsWindow(GetSafeHwnd())) {
        return;
    }
    CRect client;
    GetClientRect(&client);
    mfc_tool::ui::SafeMoveWindow(label_, client);
}
