#pragma once

#include <afxcmn.h>
#include <afxwin.h>

#include <string>
#include <vector>

#include "config/ini_manager.h"
#include "core/board_i2c_catalog.h"
#include "core/bridge_service.h"
#include "core/dashboard_poller.h"
#include "core/fw_version.generated.h"
#include "log/logger.h"
#include "ui/dashboard_tab.h"
#include "ui/extra_cmd_tab.h"

class CMainFrame : public CFrameWnd {
public:
    CMainFrame();

protected:
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRefreshDevices();
    afx_msg void OnConnect();
    afx_msg void OnDisconnect();
    afx_msg void OnReturnBootloader();
    afx_msg void OnStart();
    afx_msg void OnStop();
    afx_msg void OnClearLog();
    afx_msg LRESULT OnPollerEvent(WPARAM wParam, LPARAM lParam);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnClose();
    DECLARE_MESSAGE_MAP()

private:
    void RecreateUiFont();
    void ApplyTopControlFont();
    void LayoutControls(int cx, int cy);
    void InitializeTabs();
    void ShowActiveTabPage();
    void PopulatePinCombo();
    void RefreshDevices();
    void SetConnectionUi(bool connected);
    void SetRunningUi(bool running);
    void UpdateStatusChip(const std::wstring& text, COLORREF color);
    void AppendLog(const std::wstring& text);
    void TrimVisibleLogIfNeeded();
    void LoadIni();
    void SaveIni();
    void UpdateDeviceMetaUi();
    void UpdateFirmwareInfoUi();
    void LogBridgeInfoPayload(const std::vector<std::uint8_t>& rx, bool from_connect);
    std::wstring SelectedDevicePath() const;
    const mfc_tool::core::board_i2c::PinPair& CurrentPinPair() const;
    mfc_tool::core::DashboardPollerSettings BuildPollerSettings() const;
    std::wstring GetControlText(const CWnd& w) const;
    void SetControlText(CWnd& w, const std::wstring& text);
    static std::wstring DeviceLabel(const mfc_tool::hid::DeviceInfo& d, int index);
    static std::wstring ExtractFirmwareVersion(const std::wstring& bridge_name);
    static bool FirmwareVersionMatch(const std::wstring& expected, const std::wstring& actual);
    static std::wstring AnsiToWide(const char* text);
    static void ShowErrorBox(const std::wstring& title, const std::wstring& message);

private:
    CFont ui_font_;
    UINT current_dpi_ = 96u;
    CBrush status_chip_brush_;
    COLORREF status_chip_color_ = RGB(95, 104, 117);
    COLORREF status_chip_text_color_ = RGB(255, 255, 255);

    CButton scan_btn_;
    CComboBox device_combo_;
    CButton connect_btn_;
    CButton disconnect_btn_;
    CButton start_btn_;
    CButton stop_btn_;
    CButton return_bootloader_btn_;
    CStatic status_chip_;

    CStatic vid_label_;
    CEdit vid_edit_;
    CStatic pid_label_;
    CEdit pid_edit_;
    CStatic timeout_label_;
    CEdit timeout_edit_;
    CStatic addr_label_;
    CEdit addr_edit_;
    CStatic pin_label_;
    CComboBox pin_combo_;
    CStatic speed_label_;
    CComboBox speed_combo_;
    CStatic interval_label_;
    CEdit interval_edit_;
    CStatic rail_label_;
    CComboBox rail_combo_;
    CButton pec_check_;

    CStatic manufacturer_label_;
    CEdit manufacturer_edit_;
    CStatic product_label_;
    CEdit product_edit_;
    CStatic unique_label_;
    CEdit unique_edit_;
    CStatic build_label_;
    CStatic build_value_;
    CStatic fw_label_;
    CStatic fw_value_;

    CTabCtrl tab_ctrl_;
    CDashboardTab dashboard_tab_;
    CExtraCmdTab config_tab_;
    CExtraCmdTab fault_tab_;
    CExtraCmdTab monitor_tab_;
    CExtraCmdTab mfr_tab_;
    CExtraCmdTab crps_tab_;
    CEdit log_edit_;
    CButton clear_log_btn_;

    std::vector<mfc_tool::hid::DeviceInfo> scanned_devices_;
    mfc_tool::config::IniManager ini_;
    mfc_tool::core::BridgeService service_;
    mfc_tool::core::DashboardPoller poller_;
    mfc_tool::log::Logger logger_;
    std::wstring expected_fw_version_ = M032_EXPECTED_FW_VERSION;
    std::wstring current_fw_version_ = L"-";
    bool fw_version_match_ = true;
};
