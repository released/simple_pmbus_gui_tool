#include "dashboard_tab.h"

#include <algorithm>
#include <array>

#include "layout_utils.h"

namespace {

enum : UINT {
    IDC_DASHBOARD_RAIL_GROUP = 21000,
    IDC_DASHBOARD_SYSTEM_GROUP,
    IDC_DASHBOARD_IDENTITY_GROUP,
    IDC_DASHBOARD_STATUS_GROUP,
    IDC_DASHBOARD_TEMP_GROUP,
    IDC_DASHBOARD_FIELD_BASE,
};

} // namespace

BEGIN_MESSAGE_MAP(CDashboardTab, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDashboardTab::Create(CWnd* parent, const RECT& rect, UINT id) {
    CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                      reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);
    return CWnd::CreateEx(0, cls, L"", WS_CHILD | WS_VISIBLE, rect, parent, id);
}

int CDashboardTab::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 92);
    rail_group_.Create(L"M-CRPS Output", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(), this, IDC_DASHBOARD_RAIL_GROUP);
    system_group_.Create(L"System", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(), this, IDC_DASHBOARD_SYSTEM_GROUP);
    identity_group_.Create(L"Identity", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(), this, IDC_DASHBOARD_IDENTITY_GROUP);
    status_group_.Create(L"Status", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(), this, IDC_DASHBOARD_STATUS_GROUP);
    temp_group_.Create(L"Temperature", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(), this, IDC_DASHBOARD_TEMP_GROUP);

    const std::array<CWnd*, 5> groups = {&rail_group_, &system_group_, &identity_group_, &status_group_, &temp_group_};
    for (CWnd* group : groups) {
        group->SetFont(&ui_font_);
    }

    CreateField(L"voltage_12", L"Output Voltage (12V)", 0, 0);
    CreateField(L"current_12", L"Output Current (12V total)", 1, 0);
    CreateField(L"watt_12", L"Output Power", 2, 0);
    CreateField(L"ovp_12", L"VOUT OV Fault Limit", 0, 1);
    CreateField(L"uvp_12", L"VOUT UV Fault Limit", 1, 1);
    CreateField(L"ocp_12", L"IOUT OC Fault Limit", 2, 1);

    CreateField(L"rpm", L"RPM", 3, 0);
    CreateField(L"pout", L"POUT", 4, 0);
    CreateField(L"pin", L"PIN", 5, 0);
    CreateField(L"vin", L"VIN", 6, 0);
    CreateField(L"pmbus_revision", L"PMBus Rev.", 3, 1);
    CreateField(L"fw_revision", L"Firmware Rev.", 4, 1, 2);
    CreateField(L"mfr_id", L"MFR ID", 3, 2);
    CreateField(L"mfr_revision", L"MFR Rev.", 4, 2);
    CreateField(L"mfr_model", L"MFR MODEL", 3, 3, 3);
    CreateField(L"mfr_serial", L"MFR Number", 6, 3);

    CreateField(L"status_word", L"STATUS_WORD", 3, 5);
    CreateField(L"status_vout", L"STATUS_VOUT", 4, 5);
    CreateField(L"status_iout", L"STATUS_IOUT", 5, 5);
    CreateField(L"status_input", L"STATUS_INPUT", 6, 5);
    CreateField(L"status_temp", L"STATUS_TEMP", 3, 6);
    CreateField(L"status_cml", L"STATUS_CML", 4, 6);
    CreateField(L"vout_mode", L"VOUT_MODE", 5, 6);

    CreateField(L"temp_sr_mos", L"Temp. (SR MOS)", 0, 8);
    CreateField(L"temp_ambient", L"Temp. (Ambient)", 1, 8);
    CreateField(L"temp_primary", L"Temp. (Primary)", 2, 8);

    ClearValues();
    LayoutControls();
    return 0;
}

void CDashboardTab::CreateField(const wchar_t* id, const wchar_t* label, int x_group, int y_index, int width_group) {
    FieldControl field;
    field.id = id;
    field.x_group = x_group;
    field.y_index = y_index;
    field.width_group = width_group;
    field.label = std::make_unique<CStatic>();
    field.edit = std::make_unique<CEdit>();
    const UINT label_id = IDC_DASHBOARD_FIELD_BASE + static_cast<UINT>(fields_.size() * 2u);
    const UINT edit_id = label_id + 1u;
    field.label->Create(label, WS_CHILD | WS_VISIBLE, CRect(), this, label_id);
    field.edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, CRect(), this, edit_id);
    field.label->SetFont(&ui_font_);
    field.edit->SetFont(&ui_font_);
    edit_by_id_[field.id] = field.edit.get();
    fields_.push_back(std::move(field));
}

void CDashboardTab::RefreshDpiLayout() {
    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 92);
    mfc_tool::ui::ApplyFontToChildWindows(*this, ui_font_, FALSE);
    LayoutControls();
}

void CDashboardTab::OnSize(UINT nType, int cx, int cy) {
    CWnd::OnSize(nType, cx, cy);
    (void)cx;
    (void)cy;
    LayoutControls();
}

void CDashboardTab::LayoutControls() {
    if (!::IsWindow(GetSafeHwnd())) {
        return;
    }
    CRect client;
    GetClientRect(&client);
    const mfc_tool::ui::LayoutMetrics m = mfc_tool::ui::MetricsForWindow(*this);
    const mfc_tool::ui::DpiScaler dpi = mfc_tool::ui::DpiScaler::FromWindow(*this);
    const int margin = m.margin8;
    const int gap = m.gap;
    const int inner = m.margin8;
    const int label_h = m.label18;
    const int value_h = m.row26;
    const int field_gap = dpi.Scale(4);
    const int field_step = label_h + value_h + field_gap;
    const int rail_section_gap = dpi.Scale(18);
    const int group_bottom_pad = m.margin8;
    const int content_w = (std::max)(dpi.Scale(760), client.Width() - margin * 2);
    const int min_left_w = dpi.Scale(500);
    const int min_right_w = dpi.Scale(560);
    int left_w = ((content_w - gap) * 45) / 100;
    if (content_w >= min_left_w + gap + min_right_w) {
        left_w = (std::max)(min_left_w, (std::min)(left_w, content_w - gap - min_right_w));
    } else {
        left_w = (content_w - gap) / 2;
    }
    const int right_x = margin + left_w + gap;
    const int right_w = content_w - left_w - gap;
    const int left_cell_w = (std::max)(dpi.Scale(118), (left_w - inner * 2 - gap * 2) / 3);
    const int right_cell_w = (std::max)(dpi.Scale(108), (right_w - inner * 2 - gap * 3) / 4);
    const int temp_cell_w = (std::max)(dpi.Scale(118), (left_w - inner * 2 - gap * 2) / 3);
    const int rail_h = m.groupTopPad + field_step * 2 + group_bottom_pad;
    const int system_h = m.groupTopPad + field_step * 2 + group_bottom_pad;
    const int identity_h = m.groupTopPad + field_step * 2 + group_bottom_pad;
    const int status_h = m.groupTopPad + field_step * 2 + group_bottom_pad;
    const int temp_h = m.groupTopPad + field_step + group_bottom_pad;
    const int system_y = margin;
    const int identity_y = system_y + system_h + gap;
    const int status_y = identity_y + identity_h + gap;
    const int temp_y = margin + rail_h + gap;

    mfc_tool::ui::SafeMoveWindow(rail_group_, margin, margin, left_w, rail_h);
    mfc_tool::ui::SafeMoveWindow(system_group_, right_x, system_y, right_w, system_h);
    mfc_tool::ui::SafeMoveWindow(identity_group_, right_x, identity_y, right_w, identity_h);
    mfc_tool::ui::SafeMoveWindow(status_group_, right_x, status_y, right_w, status_h);
    mfc_tool::ui::SafeMoveWindow(temp_group_, margin, temp_y, left_w, temp_h);

    for (FieldControl& field : fields_) {
        int x = margin + inner + field.x_group * (left_cell_w + gap);
        int y = margin + m.groupTopPad + field.y_index * field_step;
        int w = left_cell_w * field.width_group + gap * (field.width_group - 1);

        if (field.x_group < 3 && field.y_index >= 4 && field.y_index < 8) {
            y = margin + m.groupTopPad + (field.y_index - 1) * field_step + rail_section_gap;
        }

        if (field.x_group >= 3) {
            x = right_x + inner + (field.x_group - 3) * (right_cell_w + gap);
            w = right_cell_w * field.width_group + gap * (field.width_group - 1);
            if (field.y_index >= 5) {
                y = status_y + m.groupTopPad + (field.y_index - 5) * field_step;
            } else if (field.y_index >= 2) {
                y = identity_y + m.groupTopPad + (field.y_index - 2) * field_step;
            } else {
                y = system_y + m.groupTopPad + field.y_index * field_step;
            }
        }
        if (field.y_index >= 8) {
            x = margin + inner + field.x_group * (temp_cell_w + gap);
            y = temp_y + m.groupTopPad + (field.y_index - 8) * field_step;
            w = temp_cell_w * field.width_group + gap * (field.width_group - 1);
        }

        mfc_tool::ui::SafeMoveWindow(*field.label, x, y, w, label_h);
        mfc_tool::ui::SafeMoveWindow(*field.edit, x, y + label_h, w, value_h);
    }
}

void CDashboardTab::ClearValues() {
    for (auto& pair : edit_by_id_) {
        if (::IsWindow(pair.second->GetSafeHwnd())) {
            pair.second->SetWindowTextW(L"-");
        }
    }
}

void CDashboardTab::ApplySnapshot(const mfc_tool::core::DashboardSnapshot& snapshot) {
    for (const auto& pair : snapshot.values) {
        SetFieldValue(pair.first, pair.second.value);
    }
}

void CDashboardTab::SetFieldValue(const std::wstring& id, const std::wstring& value) {
    const auto it = edit_by_id_.find(id);
    if (it != edit_by_id_.end() && ::IsWindow(it->second->GetSafeHwnd())) {
        it->second->SetWindowTextW(value.c_str());
    }
}

std::wstring CDashboardTab::SnapshotText(const mfc_tool::core::DashboardSnapshot& snapshot, const std::wstring& id) {
    const auto it = snapshot.values.find(id);
    return it == snapshot.values.end() ? L"-" : it->second.value;
}
