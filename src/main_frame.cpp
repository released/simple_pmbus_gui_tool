#include "main_frame.h"

#include <afxdlgs.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <sstream>

#include "build_info.generated.h"
#include "core/fw_version.generated.h"
#include "core/text_utils.h"
#include "resource.h"
#include "ui/layout_utils.h"

namespace {

enum : UINT {
    IDC_HEC_SCAN_BTN = 3000,
    IDC_HEC_DEVICE_COMBO,
    IDC_HEC_CONNECT_BTN,
    IDC_HEC_DISCONNECT_BTN,
    IDC_HEC_START_BTN,
    IDC_HEC_STOP_BTN,
    IDC_HEC_STATUS_CHIP,
    IDC_HEC_VID_LABEL,
    IDC_HEC_VID_EDIT,
    IDC_HEC_PID_LABEL,
    IDC_HEC_PID_EDIT,
    IDC_HEC_TIMEOUT_LABEL,
    IDC_HEC_TIMEOUT_EDIT,
    IDC_HEC_ADDR_LABEL,
    IDC_HEC_ADDR_EDIT,
    IDC_HEC_PIN_LABEL,
    IDC_HEC_PIN_COMBO,
    IDC_HEC_SPEED_LABEL,
    IDC_HEC_SPEED_COMBO,
    IDC_HEC_INTERVAL_LABEL,
    IDC_HEC_INTERVAL_EDIT,
    IDC_HEC_RAIL_LABEL,
    IDC_HEC_RAIL_COMBO,
    IDC_HEC_PEC_CHECK,
    IDC_HEC_MANUFACTURER_LABEL,
    IDC_HEC_MANUFACTURER_EDIT,
    IDC_HEC_PRODUCT_LABEL,
    IDC_HEC_PRODUCT_EDIT,
    IDC_HEC_UNIQUE_LABEL,
    IDC_HEC_UNIQUE_EDIT,
    IDC_HEC_BUILD_LABEL,
    IDC_HEC_BUILD_VALUE,
    IDC_HEC_FW_LABEL,
    IDC_HEC_FW_VALUE,
    IDC_HEC_TAB_CTRL,
    IDC_HEC_LOG_EDIT,
    IDC_HEC_CLEAR_LOG,
};

constexpr int kUiLogEditLimitChars = 512 * 1024;
constexpr int kUiLogTrimThresholdChars = 160 * 1024;
constexpr int kUiLogTrimTargetChars = 120 * 1024;

std::wstring NowTimeText() {
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    wchar_t buf[32] = {};
    swprintf_s(buf, L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
    return std::wstring(buf);
}

std::wstring SectionValue(const mfc_tool::config::IniData& data,
                          const std::wstring& section,
                          const std::wstring& key,
                          const std::wstring& fallback) {
    const auto sec_it = data.find(section);
    if (sec_it == data.end()) {
        return fallback;
    }
    const auto key_it = sec_it->second.find(key);
    return key_it == sec_it->second.end() ? fallback : key_it->second;
}

} // namespace

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_DPICHANGED, &CMainFrame::OnDpiChanged)
    ON_WM_GETMINMAXINFO()
    ON_NOTIFY(TCN_SELCHANGE, IDC_HEC_TAB_CTRL, &CMainFrame::OnTabSelChange)
    ON_BN_CLICKED(IDC_HEC_SCAN_BTN, &CMainFrame::OnRefreshDevices)
    ON_BN_CLICKED(IDC_HEC_CONNECT_BTN, &CMainFrame::OnConnect)
    ON_BN_CLICKED(IDC_HEC_DISCONNECT_BTN, &CMainFrame::OnDisconnect)
    ON_BN_CLICKED(IDC_HEC_START_BTN, &CMainFrame::OnStart)
    ON_BN_CLICKED(IDC_HEC_STOP_BTN, &CMainFrame::OnStop)
    ON_BN_CLICKED(IDC_HEC_CLEAR_LOG, &CMainFrame::OnClearLog)
    ON_MESSAGE(mfc_tool::core::WM_DASHBOARD_POLLER_EVENT, &CMainFrame::OnPollerEvent)
    ON_WM_CTLCOLOR()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
    : ini_(mfc_tool::config::IniManager::DefaultIniPath(L"pmbus_gui_tool.ini")) {
    expected_fw_version_ = M032_EXPECTED_FW_VERSION;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
    if (!CFrameWnd::PreCreateWindow(cs)) {
        return FALSE;
    }
    cs.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    HICON app_icon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                       reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), app_icon);
    return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    current_dpi_ = mfc_tool::ui::GetDpiForWnd(*this);
    RecreateUiFont();
    status_chip_brush_.CreateSolidBrush(status_chip_color_);
    HICON app_icon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
    SetIcon(app_icon, TRUE);
    SetIcon(app_icon, FALSE);

    auto create_or_fail = [this](BOOL ok, const wchar_t* name) -> bool {
        if (!ok) {
            AppendLog(std::wstring(L"Create control failed: ") + name);
            return false;
        }
        return true;
    };

    if (!create_or_fail(scan_btn_.Create(L"Scan USB list", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_SCAN_BTN), L"scan")) return -1;
    if (!create_or_fail(device_combo_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, CRect(), this, IDC_HEC_DEVICE_COMBO), L"device combo")) return -1;
    if (!create_or_fail(connect_btn_.Create(L"Connect Device", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_CONNECT_BTN), L"connect")) return -1;
    if (!create_or_fail(disconnect_btn_.Create(L"Disconnect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_DISCONNECT_BTN), L"disconnect")) return -1;
    if (!create_or_fail(start_btn_.Create(L"Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_START_BTN), L"start")) return -1;
    if (!create_or_fail(stop_btn_.Create(L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_STOP_BTN), L"stop")) return -1;
    if (!create_or_fail(status_chip_.Create(L"Disconnected", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, CRect(), this, IDC_HEC_STATUS_CHIP), L"status")) return -1;

    if (!create_or_fail(vid_label_.Create(L"VID", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_VID_LABEL), L"VID label")) return -1;
    if (!create_or_fail(vid_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_HEC_VID_EDIT), L"VID edit")) return -1;
    if (!create_or_fail(pid_label_.Create(L"PID", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_PID_LABEL), L"PID label")) return -1;
    if (!create_or_fail(pid_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_HEC_PID_EDIT), L"PID edit")) return -1;
    if (!create_or_fail(timeout_label_.Create(L"Timeout(ms)", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_TIMEOUT_LABEL), L"timeout label")) return -1;
    if (!create_or_fail(timeout_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_HEC_TIMEOUT_EDIT), L"timeout edit")) return -1;
    if (!create_or_fail(addr_label_.Create(L"Addr", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_ADDR_LABEL), L"addr label")) return -1;
    if (!create_or_fail(addr_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_HEC_ADDR_EDIT), L"addr edit")) return -1;
    if (!create_or_fail(pin_label_.Create(L"I2C Pins", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_PIN_LABEL), L"pin label")) return -1;
    if (!create_or_fail(pin_combo_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, CRect(), this, IDC_HEC_PIN_COMBO), L"pin combo")) return -1;
    if (!create_or_fail(speed_label_.Create(L"Speed", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_SPEED_LABEL), L"speed label")) return -1;
    if (!create_or_fail(speed_combo_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, CRect(), this, IDC_HEC_SPEED_COMBO), L"speed combo")) return -1;
    if (!create_or_fail(interval_label_.Create(L"Interval(ms)", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_INTERVAL_LABEL), L"interval label")) return -1;
    if (!create_or_fail(interval_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_HEC_INTERVAL_EDIT), L"interval edit")) return -1;
    if (!create_or_fail(rail_label_.Create(L"Output Mode", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_RAIL_LABEL), L"output mode label")) return -1;
    if (!create_or_fail(rail_combo_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, CRect(), this, IDC_HEC_RAIL_COMBO), L"rail combo")) return -1;
    if (!create_or_fail(pec_check_.Create(L"PEC", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, CRect(), this, IDC_HEC_PEC_CHECK), L"PEC")) return -1;

    if (!create_or_fail(manufacturer_label_.Create(L"Manufacturer", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_MANUFACTURER_LABEL), L"manufacturer label")) return -1;
    if (!create_or_fail(manufacturer_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, CRect(), this, IDC_HEC_MANUFACTURER_EDIT), L"manufacturer edit")) return -1;
    if (!create_or_fail(product_label_.Create(L"Product", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_PRODUCT_LABEL), L"product label")) return -1;
    if (!create_or_fail(product_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, CRect(), this, IDC_HEC_PRODUCT_EDIT), L"product edit")) return -1;
    if (!create_or_fail(unique_label_.Create(L"Unique ID", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_UNIQUE_LABEL), L"unique label")) return -1;
    if (!create_or_fail(unique_edit_.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, CRect(), this, IDC_HEC_UNIQUE_EDIT), L"unique edit")) return -1;
    if (!create_or_fail(build_label_.Create(L"Build version", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_BUILD_LABEL), L"build label")) return -1;
    if (!create_or_fail(build_value_.Create(L"-", WS_CHILD | WS_VISIBLE | SS_PATHELLIPSIS, CRect(), this, IDC_HEC_BUILD_VALUE), L"build value")) return -1;
    if (!create_or_fail(fw_label_.Create(L"Bridge FW", WS_CHILD | WS_VISIBLE, CRect(), this, IDC_HEC_FW_LABEL), L"fw label")) return -1;
    if (!create_or_fail(fw_value_.Create(L"-", WS_CHILD | WS_VISIBLE | SS_PATHELLIPSIS, CRect(), this, IDC_HEC_FW_VALUE), L"fw value")) return -1;

    if (!create_or_fail(tab_ctrl_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_FIXEDWIDTH, CRect(), this, IDC_HEC_TAB_CTRL), L"tab")) return -1;
    if (!create_or_fail(clear_log_btn_.Create(L"Clear Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(), this, IDC_HEC_CLEAR_LOG), L"clear log")) return -1;
    if (!create_or_fail(log_edit_.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER,
                                         CRect(), this, IDC_HEC_LOG_EDIT), L"log")) return -1;
    log_edit_.LimitText(kUiLogEditLimitChars);

    CRect dummy(0, 0, 100, 100);
    if (!dashboard_tab_.Create(&tab_ctrl_, dummy, ID_TAB_PMBUS)) return -1;
    if (!config_tab_.Create(&tab_ctrl_, dummy, ID_TAB_SMBUS, mfc_tool::core::DashboardConfigFieldDefinitions(), L"Config / Readback")) return -1;
    if (!fault_tab_.Create(&tab_ctrl_, dummy, ID_TAB_I2C, mfc_tool::core::DashboardFaultFieldDefinitions(), L"Fault / Warning")) return -1;
    if (!monitor_tab_.Create(&tab_ctrl_, dummy, ID_TAB_SCRIPT, mfc_tool::core::DashboardMonitorFieldDefinitions(), L"Monitor")) return -1;
    if (!mfr_tab_.Create(&tab_ctrl_, dummy, 1201, mfc_tool::core::DashboardMfrFieldDefinitions(), L"MFR Field")) return -1;
    if (!crps_tab_.Create(&tab_ctrl_, dummy, 1202, mfc_tool::core::DashboardCrpsFieldDefinitions(), L"M-CRPS Field")) return -1;

    ApplyTopControlFont();
    PopulatePinCombo();
    speed_combo_.AddString(L"100000");
    speed_combo_.AddString(L"400000");
    speed_combo_.SetCurSel(1);
    rail_combo_.AddString(L"M-CRPS 12V output");
    rail_combo_.SetCurSel(0);
    InitializeTabs();
    LoadIni();
    RefreshDevices();
    SetConnectionUi(false);
    SetRunningUi(false);
    SetControlText(build_value_, std::wstring(L"v") + PMBUS_TOOL_BUILD_VERSION + L" | " + PMBUS_TOOL_BUILD_TIME);
    UpdateFirmwareInfoUi();
    AppendLog(L"PMBus GUI tool initialized.");

    CRect rc;
    GetClientRect(&rc);
    LayoutControls(rc.Width(), rc.Height());
    return 0;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) {
    CFrameWnd::OnSize(nType, cx, cy);
    LayoutControls(cx, cy);
}

LRESULT CMainFrame::OnDpiChanged(WPARAM wParam, LPARAM lParam) {
    current_dpi_ = HIWORD(wParam);
    RecreateUiFont();
    ApplyTopControlFont();
    dashboard_tab_.RefreshDpiLayout();
    config_tab_.RefreshDpiLayout();
    fault_tab_.RefreshDpiLayout();
    monitor_tab_.RefreshDpiLayout();
    mfr_tab_.RefreshDpiLayout();
    crps_tab_.RefreshDpiLayout();
    const RECT* suggested_rect = reinterpret_cast<const RECT*>(lParam);
    if (suggested_rect != nullptr) {
        SetWindowPos(nullptr,
                     suggested_rect->left,
                     suggested_rect->top,
                     suggested_rect->right - suggested_rect->left,
                     suggested_rect->bottom - suggested_rect->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    CRect rc;
    GetClientRect(&rc);
    LayoutControls(rc.Width(), rc.Height());
    return 0;
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI) {
    CFrameWnd::OnGetMinMaxInfo(lpMMI);
    if (lpMMI == nullptr) {
        return;
    }
    const mfc_tool::ui::DpiScaler dpi(current_dpi_ != 0u ? current_dpi_ : mfc_tool::ui::GetDpiForWnd(*this));
    lpMMI->ptMinTrackSize.x = (std::max)(lpMMI->ptMinTrackSize.x, static_cast<LONG>(dpi.Scale(1120)));
    lpMMI->ptMinTrackSize.y = (std::max)(lpMMI->ptMinTrackSize.y, static_cast<LONG>(dpi.Scale(760)));
}

void CMainFrame::RecreateUiFont() {
    (void)mfc_tool::ui::CreatePointFontForWindow(ui_font_, *this, 90);
}

void CMainFrame::ApplyTopControlFont() {
    const std::array<CWnd*, 38> controls = {
        &scan_btn_, &device_combo_, &connect_btn_, &disconnect_btn_, &start_btn_, &stop_btn_, &status_chip_,
        &vid_label_, &vid_edit_, &pid_label_, &pid_edit_, &timeout_label_, &timeout_edit_, &addr_label_,
        &addr_edit_, &pin_label_, &pin_combo_, &speed_label_, &speed_combo_, &interval_label_, &interval_edit_,
        &rail_label_, &rail_combo_, &pec_check_, &manufacturer_label_, &manufacturer_edit_, &product_label_,
        &product_edit_, &unique_label_, &unique_edit_, &build_label_, &build_value_, &fw_label_, &fw_value_,
        &tab_ctrl_, &clear_log_btn_, &log_edit_, nullptr
    };
    for (CWnd* w : controls) {
        if (w != nullptr && ::IsWindow(w->GetSafeHwnd())) {
            w->SetFont(&ui_font_, FALSE);
        }
    }
}

void CMainFrame::LayoutControls(int cx, int cy) {
    if (cx <= 0 || cy <= 0) {
        return;
    }

    const mfc_tool::ui::LayoutMetrics m = mfc_tool::ui::MetricsForWindow(*this);
    const mfc_tool::ui::DpiScaler dpi(current_dpi_ != 0u ? current_dpi_ : mfc_tool::ui::GetDpiForWnd(*this));
    const int margin = m.margin8;
    const int gap = m.gap;
    const int row_h = m.row26;
    const int label_y_pad = dpi.Scale(4);
    const int btn_w = dpi.Scale(116);
    const int small_w = dpi.Scale(76);
    int x = margin;
    int y = margin;

    mfc_tool::ui::SafeMoveWindow(scan_btn_, x, y, btn_w + dpi.Scale(16), row_h);
    x += btn_w + dpi.Scale(16) + gap;
    const int device_combo_w = (std::max)(dpi.Scale(240), cx - x - margin - (btn_w * 4 + gap * 4 + dpi.Scale(120)));
    mfc_tool::ui::SafeMoveWindow(device_combo_, x, y, device_combo_w, row_h + m.comboDrop120);
    x += device_combo_w + gap;
    mfc_tool::ui::SafeMoveWindow(connect_btn_, x, y, btn_w + dpi.Scale(12), row_h);
    x += btn_w + dpi.Scale(12) + gap;
    mfc_tool::ui::SafeMoveWindow(disconnect_btn_, x, y, btn_w, row_h);
    x += btn_w + gap;
    mfc_tool::ui::SafeMoveWindow(start_btn_, x, y, dpi.Scale(82), row_h);
    x += dpi.Scale(82) + gap;
    mfc_tool::ui::SafeMoveWindow(stop_btn_, x, y, dpi.Scale(82), row_h);
    x += dpi.Scale(82) + gap;
    mfc_tool::ui::SafeMoveWindow(status_chip_, x, y, cx - x - margin, row_h);

    y += row_h + gap;
    x = margin;
    x = mfc_tool::ui::PlaceLabelAndControl(vid_label_, vid_edit_, x, y + label_y_pad, y, small_w, row_h, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(pid_label_, pid_edit_, x, y + label_y_pad, y, small_w, row_h, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(timeout_label_, timeout_edit_, x, y + label_y_pad, y, dpi.Scale(70), row_h, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(addr_label_, addr_edit_, x, y + label_y_pad, y, dpi.Scale(70), row_h, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(speed_label_, speed_combo_, x, y + label_y_pad, y, dpi.Scale(96), row_h + m.comboDrop120, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(interval_label_, interval_edit_, x, y + label_y_pad, y, dpi.Scale(80), row_h, gap, dpi.Scale(10)) + gap;
    mfc_tool::ui::SafeMoveWindow(pec_check_, x, y + dpi.Scale(3), dpi.Scale(70), row_h);

    y += row_h + gap;
    x = margin;
    x = mfc_tool::ui::PlaceLabelAndControl(pin_label_, pin_combo_, x, y + label_y_pad, y, dpi.Scale(300), row_h + m.comboDrop300, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(rail_label_, rail_combo_, x, y + label_y_pad, y, dpi.Scale(150), row_h + m.comboDrop120, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(manufacturer_label_, manufacturer_edit_, x, y + label_y_pad, y, dpi.Scale(190), row_h, gap, dpi.Scale(10)) + gap;
    x = mfc_tool::ui::PlaceLabelAndControl(product_label_, product_edit_, x, y + label_y_pad, y, (std::max)(dpi.Scale(180), cx - x - margin), row_h, gap, dpi.Scale(10));

    y += row_h + gap;
    x = margin;
    x = mfc_tool::ui::PlaceLabelAndControl(unique_label_, unique_edit_, x, y + label_y_pad, y, dpi.Scale(250), row_h, gap, dpi.Scale(10)) + gap;
    x += mfc_tool::ui::PlaceLabel(build_label_, x, y + label_y_pad, dpi.Scale(10)) + gap;
    mfc_tool::ui::SafeMoveWindow(build_value_, x, y + label_y_pad, dpi.Scale(260), m.label18);
    x += dpi.Scale(260) + gap;
    x += mfc_tool::ui::PlaceLabel(fw_label_, x, y + label_y_pad, dpi.Scale(10)) + gap;
    mfc_tool::ui::SafeMoveWindow(fw_value_, x, y + label_y_pad, cx - x - margin, m.label18);

    const int top_h = y + row_h + gap;
    const int log_h = (std::max)(dpi.Scale(90), cy / 7);
    const int log_btn_y = cy - log_h - row_h - gap - margin;
    const int log_y = cy - log_h - margin;
    mfc_tool::ui::SafeMoveWindow(clear_log_btn_, margin, log_btn_y, btn_w, row_h);
    mfc_tool::ui::SafeMoveWindow(log_edit_, margin, log_y, cx - margin * 2, cy - log_y - margin);
    mfc_tool::ui::SafeMoveWindow(tab_ctrl_, margin, top_h, cx - margin * 2, (std::max)(dpi.Scale(120), log_btn_y - top_h - gap));
    ShowActiveTabPage();
}

void CMainFrame::InitializeTabs() {
    tab_ctrl_.DeleteAllItems();
    tab_ctrl_.InsertItem(0, L"Main");
    tab_ctrl_.InsertItem(1, L"Config");
    tab_ctrl_.InsertItem(2, L"Faults");
    tab_ctrl_.InsertItem(3, L"Monitor");
    tab_ctrl_.InsertItem(4, L"MFR");
    tab_ctrl_.InsertItem(5, L"M-CRPS");
    const mfc_tool::ui::DpiScaler dpi(current_dpi_ != 0u ? current_dpi_ : mfc_tool::ui::GetDpiForWnd(*this));
    tab_ctrl_.SetItemSize(CSize(dpi.Scale(104), dpi.Scale(24)));
    tab_ctrl_.SetCurSel(0);
    ShowActiveTabPage();
}

void CMainFrame::ShowActiveTabPage() {
    if (!::IsWindow(tab_ctrl_.GetSafeHwnd())) {
        return;
    }
    CRect rc;
    tab_ctrl_.GetClientRect(&rc);
    tab_ctrl_.AdjustRect(FALSE, &rc);
    rc.DeflateRect(4, 4, 4, 4);
    const int sel = tab_ctrl_.GetCurSel();
    std::array<CWnd*, 6> pages = {&dashboard_tab_, &config_tab_, &fault_tab_, &monitor_tab_, &mfr_tab_, &crps_tab_};
    for (int i = 0; i < static_cast<int>(pages.size()); ++i) {
        mfc_tool::ui::SafeMoveWindow(*pages[i], rc);
        pages[i]->ShowWindow(sel == i ? SW_SHOW : SW_HIDE);
    }
}

void CMainFrame::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult) {
    (void)pNMHDR;
    ShowActiveTabPage();
    if (pResult != nullptr) {
        *pResult = 0;
    }
}

void CMainFrame::PopulatePinCombo() {
    pin_combo_.ResetContent();
    const auto& pairs = mfc_tool::core::board_i2c::AllPinPairs();
    for (size_t i = 0; i < pairs.size(); ++i) {
        const int index = pin_combo_.AddString(pairs[i].label);
        pin_combo_.SetItemData(index, static_cast<DWORD_PTR>(i));
    }
    pin_combo_.SetCurSel(0);
}

void CMainFrame::RefreshDevices() {
    try {
        const int vid = mfc_tool::core::ParseInt(GetControlText(vid_edit_));
        const int pid = mfc_tool::core::ParseInt(GetControlText(pid_edit_));
        scanned_devices_ = service_.ScanDevices(static_cast<std::uint16_t>(vid), static_cast<std::uint16_t>(pid));
        std::sort(scanned_devices_.begin(), scanned_devices_.end(), [](const auto& a, const auto& b) {
            return DeviceLabel(a, 0) < DeviceLabel(b, 0);
        });
        device_combo_.ResetContent();
        device_combo_.AddString(L"Auto Select");
        for (size_t i = 0; i < scanned_devices_.size(); ++i) {
            device_combo_.AddString(DeviceLabel(scanned_devices_[i], static_cast<int>(i)).c_str());
        }
        device_combo_.SetCurSel(scanned_devices_.empty() ? 0 : 1);
        AppendLog(L"USB scan complete: " + std::to_wstring(scanned_devices_.size()) + L" device(s) found.");
    } catch (const std::exception& e) {
        AppendLog(L"USB scan failed: " + AnsiToWide(e.what()));
        ShowErrorBox(L"Scan USB list", AnsiToWide(e.what()));
    }
}

void CMainFrame::OnRefreshDevices() {
    RefreshDevices();
}

void CMainFrame::OnConnect() {
    try {
        const int vid = mfc_tool::core::ParseInt(GetControlText(vid_edit_));
        const int pid = mfc_tool::core::ParseInt(GetControlText(pid_edit_));
        const int timeout = mfc_tool::core::ParseInt(GetControlText(timeout_edit_));
        service_.Disconnect();
        service_.Connect(static_cast<std::uint16_t>(vid), static_cast<std::uint16_t>(pid), SelectedDevicePath(), timeout);
        SetConnectionUi(true);
        UpdateDeviceMetaUi();
        AppendLog(L"HID bridge connected.");
        try {
            LogBridgeInfoPayload(service_.GetInfo(), true);
        } catch (const std::exception& e) {
            AppendLog(L"Auto Get Info failed: " + AnsiToWide(e.what()));
        }
        SaveIni();
    } catch (const std::exception& e) {
        SetConnectionUi(false);
        ShowErrorBox(L"Connect Device", AnsiToWide(e.what()));
        AppendLog(L"Connect failed: " + AnsiToWide(e.what()));
    }
}

void CMainFrame::OnDisconnect() {
    OnStop();
    service_.Disconnect();
    SetConnectionUi(false);
    UpdateDeviceMetaUi();
    AppendLog(L"HID bridge disconnected.");
}

void CMainFrame::OnStart() {
    if (!service_.IsConnected()) {
        ShowErrorBox(L"Start", L"Connect HID bridge before Start.");
        return;
    }
    if (poller_.IsRunning()) {
        return;
    }
    try {
        mfc_tool::core::DashboardPollerSettings settings = BuildPollerSettings();
        dashboard_tab_.ClearValues();
        config_tab_.ClearValues();
        fault_tab_.ClearValues();
        monitor_tab_.ClearValues();
        mfr_tab_.ClearValues();
        crps_tab_.ClearValues();
        if (!poller_.Start(&service_, GetSafeHwnd(), settings)) {
            ShowErrorBox(L"Start", L"Unable to start polling worker.");
            return;
        }
        SetRunningUi(true);
        UpdateStatusChip(L"Running", RGB(35, 132, 67));
        AppendLog(L"Dashboard polling started.");
        SaveIni();
    } catch (const std::exception& e) {
        ShowErrorBox(L"Start", AnsiToWide(e.what()));
        AppendLog(L"Start failed: " + AnsiToWide(e.what()));
    }
}

void CMainFrame::OnStop() {
    if (poller_.IsRunning()) {
        AppendLog(L"Stopping dashboard polling...");
        poller_.Stop();
    }
    SetRunningUi(false);
    if (service_.IsConnected()) {
        UpdateStatusChip(L"Connected", RGB(46, 139, 87));
    }
}

void CMainFrame::SetConnectionUi(bool connected) {
    mfc_tool::ui::SafeEnableWindow(connect_btn_, !connected);
    mfc_tool::ui::SafeEnableWindow(disconnect_btn_, connected);
    mfc_tool::ui::SafeEnableWindow(start_btn_, connected && !poller_.IsRunning());
    mfc_tool::ui::SafeEnableWindow(scan_btn_, !connected);
    mfc_tool::ui::SafeEnableWindow(device_combo_, !connected);
    UpdateStatusChip(connected ? L"Connected" : L"Disconnected", connected ? RGB(46, 139, 87) : RGB(95, 104, 117));
}

void CMainFrame::SetRunningUi(bool running) {
    mfc_tool::ui::SafeEnableWindow(start_btn_, service_.IsConnected() && !running);
    mfc_tool::ui::SafeEnableWindow(stop_btn_, running);
    mfc_tool::ui::SafeEnableWindow(disconnect_btn_, service_.IsConnected());
    mfc_tool::ui::SafeEnableWindow(addr_edit_, !running);
    mfc_tool::ui::SafeEnableWindow(pin_combo_, !running);
    mfc_tool::ui::SafeEnableWindow(speed_combo_, !running);
    mfc_tool::ui::SafeEnableWindow(interval_edit_, !running);
    mfc_tool::ui::SafeEnableWindow(rail_combo_, !running);
    mfc_tool::ui::SafeEnableWindow(pec_check_, !running);
}

LRESULT CMainFrame::OnPollerEvent(WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    std::unique_ptr<mfc_tool::core::DashboardPollerEvent> event(
        reinterpret_cast<mfc_tool::core::DashboardPollerEvent*>(lParam));
    if (!event) {
        return 0;
    }
    switch (event->kind) {
    case mfc_tool::core::DashboardPollerEvent::Kind::Snapshot:
        dashboard_tab_.ApplySnapshot(event->snapshot);
        config_tab_.ApplySnapshot(event->snapshot);
        fault_tab_.ApplySnapshot(event->snapshot);
        monitor_tab_.ApplySnapshot(event->snapshot);
        mfr_tab_.ApplySnapshot(event->snapshot);
        crps_tab_.ApplySnapshot(event->snapshot);
        {
            wchar_t buf[80] = {};
            swprintf_s(buf, L"Running  cycle=%d  err=%d  %02u:%02u:%02u",
                       event->snapshot.cycle,
                       event->snapshot.consecutive_errors,
                       event->snapshot.timestamp.wHour,
                       event->snapshot.timestamp.wMinute,
                       event->snapshot.timestamp.wSecond);
            UpdateStatusChip(buf, event->snapshot.consecutive_errors == 0 ? RGB(35, 132, 67) : RGB(178, 105, 0));
        }
        break;
    case mfc_tool::core::DashboardPollerEvent::Kind::Info:
        AppendLog(event->message);
        break;
    case mfc_tool::core::DashboardPollerEvent::Kind::Error:
        AppendLog(L"Polling error: " + event->message);
        UpdateStatusChip(L"Polling error", RGB(178, 34, 34));
        break;
    case mfc_tool::core::DashboardPollerEvent::Kind::Stopped:
        AppendLog(event->message);
        SetRunningUi(false);
        if (service_.IsConnected()) {
            UpdateStatusChip(L"Connected", RGB(46, 139, 87));
        }
        break;
    default:
        break;
    }
    return 0;
}

void CMainFrame::UpdateStatusChip(const std::wstring& text, COLORREF color) {
    SetControlText(status_chip_, text);
    status_chip_color_ = color;
    if (status_chip_brush_.GetSafeHandle()) {
        status_chip_brush_.DeleteObject();
    }
    status_chip_brush_.CreateSolidBrush(status_chip_color_);
    status_chip_.Invalidate(FALSE);
}

void CMainFrame::AppendLog(const std::wstring& text) {
    const std::wstring line = L"[" + NowTimeText() + L"] " + text;
    logger_.AddLine(line);
    if (!::IsWindow(log_edit_.GetSafeHwnd())) {
        return;
    }
    int len = log_edit_.GetWindowTextLengthW();
    log_edit_.SetSel(len, len);
    log_edit_.ReplaceSel((line + L"\r\n").c_str());
    TrimVisibleLogIfNeeded();
}

void CMainFrame::TrimVisibleLogIfNeeded() {
    const int len = log_edit_.GetWindowTextLengthW();
    if (len <= kUiLogTrimThresholdChars) {
        return;
    }
    CString visible_log;
    log_edit_.GetWindowTextW(visible_log);
    std::wstring text = visible_log.GetString();
    int trim_chars = len - kUiLogTrimTargetChars;
    std::wstring::size_type trim_end = text.find(L'\n', static_cast<std::wstring::size_type>(trim_chars));
    if (trim_end == std::wstring::npos) {
        trim_end = static_cast<std::wstring::size_type>(trim_chars);
    } else {
        ++trim_end;
    }
    if (trim_end == 0 || trim_end >= text.size()) {
        return;
    }
    log_edit_.SetSel(0, static_cast<int>(trim_end));
    log_edit_.ReplaceSel(L"");
    log_edit_.SetSel(log_edit_.GetWindowTextLengthW(), log_edit_.GetWindowTextLengthW());
}

void CMainFrame::OnClearLog() {
    logger_.Clear();
    log_edit_.SetWindowTextW(L"");
}

void CMainFrame::LoadIni() {
    vid_edit_.SetWindowTextW(L"0x0416");
    pid_edit_.SetWindowTextW(L"0x5020");
    timeout_edit_.SetWindowTextW(L"2000");
    addr_edit_.SetWindowTextW(L"0x5A");
    interval_edit_.SetWindowTextW(L"1000");
    speed_combo_.SetCurSel(1);
    pec_check_.SetCheck(BST_CHECKED);
    std::wstring error;
    if (!ini_.Exists()) {
        SaveIni();
        return;
    }
    mfc_tool::config::IniData data;
    if (!ini_.Load(&data, &error)) {
        AppendLog(L"INI load failed: " + error);
        return;
    }
    vid_edit_.SetWindowTextW(SectionValue(data, L"ui", L"vid", L"0x0416").c_str());
    pid_edit_.SetWindowTextW(SectionValue(data, L"ui", L"pid", L"0x5020").c_str());
    timeout_edit_.SetWindowTextW(SectionValue(data, L"ui", L"timeout_ms", L"2000").c_str());
    addr_edit_.SetWindowTextW(SectionValue(data, L"target", L"addr", L"0x5A").c_str());
    interval_edit_.SetWindowTextW(SectionValue(data, L"target", L"interval_ms", L"1000").c_str());
    const std::wstring speed = SectionValue(data, L"target", L"speed_hz", L"400000");
    speed_combo_.SetCurSel(speed == L"400000" ? 1 : 0);
    rail_combo_.SetCurSel(0);
    pec_check_.SetCheck(SectionValue(data, L"target", L"pec", L"1") == L"1" ? BST_CHECKED : BST_UNCHECKED);
    const std::wstring pin_name = SectionValue(data, L"target", L"pin_pair", L"I2C0_PB4_PB5");
    const auto& pairs = mfc_tool::core::board_i2c::AllPinPairs();
    for (int i = 0; i < static_cast<int>(pairs.size()); ++i) {
        if (pin_name == pairs[static_cast<size_t>(i)].ini_name) {
            pin_combo_.SetCurSel(i);
            break;
        }
    }
}

void CMainFrame::SaveIni() {
    mfc_tool::config::IniData data;
    data[L"ui"][L"vid"] = GetControlText(vid_edit_);
    data[L"ui"][L"pid"] = GetControlText(pid_edit_);
    data[L"ui"][L"timeout_ms"] = GetControlText(timeout_edit_);
    data[L"target"][L"addr"] = GetControlText(addr_edit_);
    data[L"target"][L"interval_ms"] = GetControlText(interval_edit_);
    data[L"target"][L"speed_hz"] = GetControlText(speed_combo_);
    data[L"target"][L"rail_mode"] = L"single";
    data[L"target"][L"pec"] = pec_check_.GetCheck() == BST_CHECKED ? L"1" : L"0";
    data[L"target"][L"pin_pair"] = CurrentPinPair().ini_name;
    data[L"bridge"][L"expected_fw_version"] = expected_fw_version_;
    data[L"bridge"][L"last_seen_fw_version"] = current_fw_version_;
    std::wstring error;
    if (!ini_.Save(data, &error)) {
        AppendLog(L"INI save failed: " + error);
    }
}

void CMainFrame::UpdateDeviceMetaUi() {
    if (!service_.IsConnected()) {
        SetControlText(manufacturer_edit_, L"-");
        SetControlText(product_edit_, L"-");
        SetControlText(unique_edit_, L"-");
        return;
    }
    const auto& info = service_.CurrentDevice();
    SetControlText(manufacturer_edit_, info.manufacturer.empty() ? L"-" : info.manufacturer);
    SetControlText(product_edit_, info.product.empty() ? L"-" : info.product);
    SetControlText(unique_edit_, info.serial_number.empty() ? L"-" : info.serial_number);
}

void CMainFrame::UpdateFirmwareInfoUi() {
    std::wstring text = L"expected=" + expected_fw_version_ + L" actual=" + current_fw_version_;
    if (!fw_version_match_) {
        text += L"  MISMATCH";
    }
    SetControlText(fw_value_, text);
}

void CMainFrame::LogBridgeInfoPayload(const std::vector<std::uint8_t>& rx, bool from_connect) {
    std::wstring bridge_name;
    if (!rx.empty()) {
        bridge_name.assign(rx.begin(), rx.end());
        const auto end_pos = std::find(bridge_name.begin(), bridge_name.end(), L'\0');
        bridge_name.erase(end_pos, bridge_name.end());
    }
    current_fw_version_ = ExtractFirmwareVersion(bridge_name);
    if (current_fw_version_.empty()) {
        current_fw_version_ = L"-";
    }
    fw_version_match_ = FirmwareVersionMatch(expected_fw_version_, current_fw_version_) || current_fw_version_ == L"-";
    UpdateFirmwareInfoUi();
    AppendLog(std::wstring(from_connect ? L"Connected bridge: " : L"Bridge info: ") + bridge_name);
    if (!fw_version_match_) {
        const std::wstring warn = L"Firmware version mismatch: expected " + expected_fw_version_ +
                                  L", device " + current_fw_version_;
        AppendLog(warn);
        if (from_connect) {
            ::MessageBoxW(m_hWnd, warn.c_str(), L"Firmware Version Mismatch", MB_ICONWARNING | MB_OK);
        }
    }
}

std::wstring CMainFrame::SelectedDevicePath() const {
    const int sel = device_combo_.GetCurSel();
    if (sel == CB_ERR || sel <= 0) {
        return scanned_devices_.empty() ? L"" : scanned_devices_.front().path;
    }
    const size_t idx = static_cast<size_t>(sel - 1);
    return idx < scanned_devices_.size() ? scanned_devices_[idx].path : L"";
}

const mfc_tool::core::board_i2c::PinPair& CMainFrame::CurrentPinPair() const {
    const auto& pairs = mfc_tool::core::board_i2c::AllPinPairs();
    const int sel = pin_combo_.GetCurSel();
    if (sel != CB_ERR) {
        const DWORD_PTR data = pin_combo_.GetItemData(sel);
        if (data != CB_ERR && data < pairs.size()) {
            return pairs[static_cast<size_t>(data)];
        }
    }
    return pairs.front();
}

mfc_tool::core::DashboardPollerSettings CMainFrame::BuildPollerSettings() const {
    const auto& pins = CurrentPinPair();
    mfc_tool::core::DashboardPollerSettings settings;
    settings.port = pins.i2c_port;
    settings.sda_pin = pins.sda_pin;
    settings.scl_pin = pins.scl_pin;
    settings.addr7 = mfc_tool::core::ParseInt(GetControlText(addr_edit_)) & 0x7F;
    settings.interval_ms = (std::max)(100, mfc_tool::core::ParseInt(GetControlText(interval_edit_)));
    settings.baud = mfc_tool::core::ParseInt(GetControlText(speed_combo_));
    settings.pec = pec_check_.GetCheck() == BST_CHECKED;
    settings.multi_rail_pages = rail_combo_.GetCurSel() == 1;
    return settings;
}

std::wstring CMainFrame::GetControlText(const CWnd& w) const {
    CString s;
    const_cast<CWnd&>(w).GetWindowTextW(s);
    return s.GetString();
}

void CMainFrame::SetControlText(CWnd& w, const std::wstring& text) {
    if (::IsWindow(w.GetSafeHwnd())) {
        w.SetWindowTextW(text.c_str());
    }
}

std::wstring CMainFrame::DeviceLabel(const mfc_tool::hid::DeviceInfo& d, int index) {
    std::wstringstream ss;
    ss << L"#" << index + 1 << L" VID:PID ";
    ss << std::uppercase << std::hex;
    ss.width(4);
    ss.fill(L'0');
    ss << d.vendor_id << L":";
    ss.width(4);
    ss << d.product_id;
    ss << std::dec << L" IN:" << d.input_report_len << L" OUT:" << d.output_report_len;
    if (!d.manufacturer.empty() || !d.product.empty()) {
        ss << L" [" << d.manufacturer;
        if (!d.manufacturer.empty() && !d.product.empty()) {
            ss << L" - ";
        }
        ss << d.product << L"]";
    }
    return ss.str();
}

std::wstring CMainFrame::ExtractFirmwareVersion(const std::wstring& bridge_name) {
    const auto pos = bridge_name.find(L'/');
    if (pos == std::wstring::npos || pos + 1 >= bridge_name.size()) {
        return L"";
    }
    return bridge_name.substr(pos + 1);
}

bool CMainFrame::FirmwareVersionMatch(const std::wstring& expected, const std::wstring& actual) {
    return !expected.empty() && expected == actual;
}

std::wstring CMainFrame::AnsiToWide(const char* text) {
    if (text == nullptr) {
        return L"";
    }
    const int n = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (n <= 0) {
        return L"";
    }
    std::wstring out(static_cast<std::size_t>(n - 1), L'\0');
    MultiByteToWideChar(CP_ACP, 0, text, -1, out.data(), n);
    return out;
}

void CMainFrame::ShowErrorBox(const std::wstring& title, const std::wstring& message) {
    ::MessageBoxW(nullptr, message.c_str(), title.c_str(), MB_ICONERROR | MB_OK);
}

HBRUSH CMainFrame::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hbr = CFrameWnd::OnCtlColor(pDC, pWnd, nCtlColor);
    if (pWnd != nullptr && pWnd->GetSafeHwnd() == status_chip_.GetSafeHwnd()) {
        pDC->SetBkColor(status_chip_color_);
        pDC->SetTextColor(status_chip_text_color_);
        return status_chip_brush_;
    }
    return hbr;
}

void CMainFrame::OnClose() {
    SaveIni();
    poller_.Stop();
    service_.Disconnect();
    CFrameWnd::OnClose();
}
