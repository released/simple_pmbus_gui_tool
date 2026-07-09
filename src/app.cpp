#include "app.h"

#include "main_frame.h"

BEGIN_MESSAGE_MAP(CPmbusGuiToolApp, CWinApp)
END_MESSAGE_MAP()

BOOL CPmbusGuiToolApp::InitInstance() {
    CWinApp::InitInstance();

    auto* frame = new CMainFrame();
    if (!frame->Create(nullptr, L"PMBus GUI tool", WS_OVERLAPPEDWINDOW, CRect(0, 0, 1280, 820))) {
        delete frame;
        return FALSE;
    }

    m_pMainWnd = frame;
    frame->ShowWindow(SW_SHOWMAXIMIZED);
    frame->UpdateWindow();
    return TRUE;
}
