#pragma once

#include <afxwin.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../core/dashboard_poller.h"

class CDashboardTab : public CWnd {
public:
    BOOL Create(CWnd* parent, const RECT& rect, UINT id);
    void RefreshDpiLayout();
    void ClearValues();
    void ApplySnapshot(const mfc_tool::core::DashboardSnapshot& snapshot);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    struct FieldControl {
        std::wstring id;
        std::unique_ptr<CStatic> label;
        std::unique_ptr<CEdit> edit;
        int x_group = 0;
        int y_index = 0;
        int width_group = 1;
    };

    void CreateField(const wchar_t* id, const wchar_t* label, int x_group, int y_index, int width_group = 1);
    void LayoutControls();
    void SetFieldValue(const std::wstring& id, const std::wstring& value);
    static std::wstring SnapshotText(const mfc_tool::core::DashboardSnapshot& snapshot, const std::wstring& id);

private:
    CFont ui_font_;
    CButton rail_group_;
    CButton system_group_;
    CButton identity_group_;
    CButton status_group_;
    CButton temp_group_;
    std::vector<FieldControl> fields_;
    std::map<std::wstring, CEdit*> edit_by_id_;
};
