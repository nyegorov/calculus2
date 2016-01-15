// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "expr_render.h"
#include "../parser.h"

typedef std::unique_ptr<std::remove_pointer<HTHEME>::type, std::function<void(HTHEME)>> theme_ptr;

class CHistory : public aero::CListBox
{
};

class CHistoryEditBox : public aero::CEdit
{
	CHistory&	_history;
public:
	CHistoryEditBox(CHistory& history_ctrl) : _history(history_ctrl) {}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if(nChar == VK_UP || nChar == VK_DOWN)	_history.PostMessageW(WM_KEYDOWN, nChar, nRepCnt | (nFlags << 16));
		else DefWindowProc();
		Invalidate(FALSE);
	}

	BEGIN_MSG_MAP(CHistoryEditBox)
		MSG_WM_KEYDOWN(OnKeyDown)
		CHAIN_MSG_MAP(aero::CEdit)
	END_MSG_MAP()
};

class CMainDlg : public aero::CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>, public CWinDataExchange<CMainDlg>,
		public CMessageFilter, public CIdleHandler, public CDialogResize<CMainDlg>
{
	CHistory		_listCtrl;
	CHistoryEditBox	_inputCtrl{_listCtrl};
	CFont			_font;
	theme_ptr		_theme;
	std::vector<expr_info>	_history;
	expr_renderer	_renderer{1.5};
	cas::NScript	_parser;

public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_DLGRESIZE_MAP(CMainDlg)
		DLGRESIZE_CONTROL(IDC_INPUT, DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_DDX_MAP(CMainDlg)
		DDX_CONTROL(IDC_INPUT, _inputCtrl)
		DDX_CONTROL(IDC_LIST, _listCtrl)
	END_DDX_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		CHAIN_MSG_MAP(aero::CDialogImpl<CMainDlg>)
		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MSG_WM_MEASUREITEM(OnMeasureItem)
		MSG_WM_DRAWITEM(OnDrawItem)
		COMMAND_HANDLER_EX(IDC_LIST, LBN_SELCHANGE, OnSelChange)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		ExecuteDlgInit(IDD_MAINDLG);
		DlgResize_Init(false);
		MARGINS m = {-1};
		SetMargins(m);

		DoDataExchange();
		_theme = theme_ptr(OpenThemeData(L"Explorer::ListView"), ::CloseThemeData);
		_font.CreatePointFont(120, L"Consolas");
		_inputCtrl.SetFont(_font);
		//SetOpaqueUnder(_listCtrl);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		auto& ei = _history[lpMeasureItemStruct->itemData];
		lpMeasureItemStruct->itemWidth = ei.width() + 10;
		lpMeasureItemStruct->itemHeight = ei.height() + 5;
	}

	void DrawItem(const expr_info& ei, HDC hdc, RECT rcItem, unsigned itemState)
	{
		CRect rc(rcItem);
		CDCHandle dc(hdc);
		dc.FillSolidRect(rc, GetSysColor(COLOR_WINDOW));
		if(itemState & ODS_SELECTED)	::DrawThemeBackground(_theme.get(), dc, LVP_LISTITEM, LISS_SELECTED, rc, rc);
		_renderer.render(ei, dc, rc.left + 10, rc.top + 5);
	}

	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		auto& ei = _history[lpDrawItemStruct->itemData];
		HDC hDCPaint;

		if(lpDrawItemStruct->itemAction == ODA_FOCUS)	return;
		ATLTRACE2(atlTraceUI, 0, _T("OnDrawItem(%d): %d/%2x\n"), lpDrawItemStruct->itemID, lpDrawItemStruct->itemAction, lpDrawItemStruct->itemState);
		if(_listCtrl.IsBufferedPaintSupported() && _listCtrl.m_BufferedPaint.IsNull() &&
		   _listCtrl.m_BufferedPaint.Begin(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, m_dwFormat, &m_PaintParams, &hDCPaint)) {
			DrawItem(ei, hDCPaint, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState);
			_listCtrl.m_BufferedPaint.MakeOpaque(&lpDrawItemStruct->rcItem);
			_listCtrl.m_BufferedPaint.End();
		} else {
			DrawItem(ei, lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState);
		}
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ATL::CString wtext;
		_inputCtrl.GetWindowTextW(wtext);
		NScript builder(false);
		string text = CW2A(wtext);
		_history.push_back(_renderer.create(text, builder.eval(text.c_str()), _parser.eval(text.c_str())));
		_listCtrl.InsertString(-1, (LPCTSTR)(_history.size() - 1));
		_listCtrl.SetCurSel(_listCtrl.GetCount() - 1);
		return 0;
	}

	LRESULT OnSelChange(UINT uNotifyCode, int nID, CWindow wndCtl) {
		ATL::CString s;
		unsigned idx = _listCtrl.GetItemData(_listCtrl.GetCurSel());
		auto& ei = _history[idx];
		_inputCtrl.SetWindowTextW(CA2W(ei.text.c_str()));
		_inputCtrl.SetSel(ei.text.size(), ei.text.size(), FALSE);
		return TRUE;
	}		
};
