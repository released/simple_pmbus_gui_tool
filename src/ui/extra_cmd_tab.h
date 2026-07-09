#pragma once

#include <afxcmn.h>
#include <afxwin.h>

#include <string>
#include <vector>

#include "../core/dashboard_field_profile.h"
#include "../core/dashboard_poller.h"

class CExtraCmdTab : public CWnd {
public:
    BOOL Create(CWnd* parent,
                const RECT& rect,
                UINT id,
                const std::vector<mfc_tool::core::DashboardFieldDefinition>& fields,
                const wchar_t* field_column);
    void RefreshDpiLayout();
    void ClearValues();
    void ApplySnapshot(const mfc_tool::core::DashboardSnapshot& snapshot);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutControls();
    int RowForField(const std::wstring& id) const;
    const std::vector<mfc_tool::core::DashboardFieldDefinition>& Fields() const;

private:
    const std::vector<mfc_tool::core::DashboardFieldDefinition>* fields_ = nullptr;
    std::wstring field_column_;
    CFont ui_font_;
    CListCtrl list_;
};
