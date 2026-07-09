#include "extra_cmd_tab.h"

#include "layout_utils.h"

namespace {

enum : UINT {
    IDC_EXTRA_LIST = 22000,
};

} // namespace

BEGIN_MESSAGE_MAP(CExtraCmdTab, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CExtraCmdTab::Create(CWnd* parent,
                          const RECT& rect,
                          UINT id,
                          const std::vector<mfc_tool::core::DashboardFieldDefinition>& fields,
                          const wchar_t* field_column) {
    fields_ = &fields;
    field_column_ = field_column != nullptr ? field_column : L"Command";
    CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                      reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);
    return CWnd::CreateEx(0, cls, L"", WS_CHILD | WS_VISIBLE, rect, parent, id);
}

int CExtraCmdTab::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 92);
    list_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, CRect(), this, IDC_EXTRA_LIST);
    list_.SetFont(&ui_font_);
    list_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
    list_.InsertColumn(0, field_column_.c_str(), LVCFMT_LEFT, 220);
    list_.InsertColumn(1, L"Value", LVCFMT_LEFT, 360);
    list_.InsertColumn(2, L"Status", LVCFMT_LEFT, 260);

    int row = 0;
    for (const auto& field : Fields()) {
        list_.InsertItem(row, field.label);
        list_.SetItemText(row, 1, L"-");
        list_.SetItemText(row, 2, L"Not read");
        ++row;
    }
    LayoutControls();
    return 0;
}

void CExtraCmdTab::RefreshDpiLayout() {
    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 92);
    if (::IsWindow(list_.GetSafeHwnd())) {
        list_.SetFont(&ui_font_);
    }
    LayoutControls();
}

void CExtraCmdTab::OnSize(UINT nType, int cx, int cy) {
    CWnd::OnSize(nType, cx, cy);
    (void)cx;
    (void)cy;
    LayoutControls();
}

void CExtraCmdTab::LayoutControls() {
    if (!::IsWindow(GetSafeHwnd())) {
        return;
    }
    CRect client;
    GetClientRect(&client);
    client.DeflateRect(8, 8, 8, 8);
    mfc_tool::ui::SafeMoveWindow(list_, client);
    if (::IsWindow(list_.GetSafeHwnd())) {
        const int w = client.Width();
        list_.SetColumnWidth(0, (std::max)(180, w / 4));
        list_.SetColumnWidth(1, (std::max)(260, w / 2));
        list_.SetColumnWidth(2, (std::max)(160, w - list_.GetColumnWidth(0) - list_.GetColumnWidth(1) - 24));
    }
}

void CExtraCmdTab::ClearValues() {
    if (!::IsWindow(list_.GetSafeHwnd())) {
        return;
    }
    for (int i = 0; i < list_.GetItemCount(); ++i) {
        list_.SetItemText(i, 1, L"-");
        list_.SetItemText(i, 2, L"Not read");
    }
}

void CExtraCmdTab::ApplySnapshot(const mfc_tool::core::DashboardSnapshot& snapshot) {
    if (!::IsWindow(list_.GetSafeHwnd())) {
        return;
    }
    for (const auto& field : Fields()) {
        const int row = RowForField(field.id);
        if (row < 0) {
            continue;
        }
        const auto it = snapshot.values.find(field.id);
        if (it == snapshot.values.end()) {
            continue;
        }
        list_.SetItemText(row, 1, it->second.value.c_str());
        list_.SetItemText(row, 2, it->second.status.c_str());
    }
}

int CExtraCmdTab::RowForField(const std::wstring& id) const {
    int row = 0;
    for (const auto& field : Fields()) {
        if (id == field.id) {
            return row;
        }
        ++row;
    }
    return -1;
}

const std::vector<mfc_tool::core::DashboardFieldDefinition>& CExtraCmdTab::Fields() const {
    if (fields_ != nullptr) {
        return *fields_;
    }
    return mfc_tool::core::DashboardExtraFieldDefinitions();
}
