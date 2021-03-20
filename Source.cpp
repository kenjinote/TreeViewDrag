#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32")

#include <windows.h>
#include <commctrl.h>

#define DEFAULT_DPI 96
#define SCALEX(X) MulDiv(X, uDpiX, DEFAULT_DPI)
#define SCALEY(Y) MulDiv(Y, uDpiY, DEFAULT_DPI)
#define POINT2PIXEL(PT) MulDiv(PT, uDpiY, 72)

TCHAR szClassName[] = TEXT("Window");

BOOL GetScaling(HWND hWnd, UINT* pnX, UINT* pnY)
{
	BOOL bSetScaling = FALSE;
	const HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		HMODULE hShcore = LoadLibrary(TEXT("SHCORE"));
		if (hShcore)
		{
			typedef HRESULT __stdcall GetDpiForMonitor(HMONITOR, int, UINT*, UINT*);
			GetDpiForMonitor* fnGetDpiForMonitor = reinterpret_cast<GetDpiForMonitor*>(GetProcAddress(hShcore, "GetDpiForMonitor"));
			if (fnGetDpiForMonitor)
			{
				UINT uDpiX, uDpiY;
				if (SUCCEEDED(fnGetDpiForMonitor(hMonitor, 0, &uDpiX, &uDpiY)) && uDpiX > 0 && uDpiY > 0)
				{
					*pnX = uDpiX;
					*pnY = uDpiY;
					bSetScaling = TRUE;
				}
			}
			FreeLibrary(hShcore);
		}
	}
	if (!bSetScaling)
	{
		HDC hdc = GetDC(NULL);
		if (hdc)
		{
			*pnX = GetDeviceCaps(hdc, LOGPIXELSX);
			*pnY = GetDeviceCaps(hdc, LOGPIXELSY);
			ReleaseDC(NULL, hdc);
			bSetScaling = TRUE;
		}
	}
	if (!bSetScaling)
	{
		*pnX = DEFAULT_DPI;
		*pnY = DEFAULT_DPI;
		bSetScaling = TRUE;
	}
	return bSetScaling;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int flag;
	static HWND hTree;
	static HFONT hFont;
	static HTREEITEM start;
	static UINT uDpiX = DEFAULT_DPI, uDpiY = DEFAULT_DPI;
	switch (msg)
	{
	case WM_CREATE:
		InitCommonControls();
		hTree = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_TREEVIEW,
			0,
			WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT,
			0, 0, 0, 0,
			hWnd,
			(HMENU)100,
			((LPCREATESTRUCT)lParam)->hInstance,
			0);
		{
			// 親ノード
			HTREEITEM hRootItem = 0;
			{
				SetFocus(hTree);
				TV_INSERTSTRUCT tv = { 0 };
				tv.hInsertAfter = TVI_LAST;
				tv.item.mask = TVIF_TEXT;
				tv.hParent = TVI_ROOT;
				tv.item.pszText = TEXT("Root");
				hRootItem = TreeView_InsertItem(hTree, &tv);
			}
			// 子ノード1
			if (hRootItem) {
				TV_INSERTSTRUCT tv = { 0 };
				tv.hInsertAfter = TVI_LAST;
				tv.item.mask = TVIF_TEXT;
				tv.hParent = hRootItem;
				tv.item.pszText = TEXT("Child1");
				HTREEITEM hItem = TreeView_InsertItem(hTree, &tv);
				TreeView_Select(hTree, hItem, TVGN_CARET);
			}
			// 子ノード2
			if (hRootItem) {
				TV_INSERTSTRUCT tv = { 0 };
				tv.hInsertAfter = TVI_LAST;
				tv.item.mask = TVIF_TEXT;
				tv.hParent = hRootItem;
				tv.item.pszText = TEXT("Child2");
				HTREEITEM hItem = TreeView_InsertItem(hTree, &tv);
				TreeView_Select(hTree, hItem, TVGN_CARET);
			}
		}
		SendMessage(hWnd, WM_APP, 0, 0);
		break;
	case WM_NOTIFY:
		if (wParam == 100) {
			LPNMHDR lpnmhdr;
			lpnmhdr = (LPNMHDR)lParam;
			switch (lpnmhdr->code) {
			case TVN_BEGINDRAG:             //      ドラッグ開始
				NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)lParam;
				start = pnmtv->itemNew.hItem;
				TreeView_SelectItem(hTree, start);
				flag = 1;
				SetCapture(hWnd);
				break;
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (flag == 1)
		{
			InvalidateRect(hWnd, 0, 1);
		}
		break;
	case WM_LBUTTONUP:
		if (flag == 1) {
			ReleaseCapture();
			flag = 0;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			if (flag == 1) {
				POINT point;
				GetCursorPos(&point);
				ScreenToClient(hWnd, &point);
				Ellipse(hdc, point.x - POINT2PIXEL(10), point.y - POINT2PIXEL(10), point.x + POINT2PIXEL(10), point.y + POINT2PIXEL(10));
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_SETFOCUS:
		SetFocus(hTree);
		break;
	case WM_SIZE:
		MoveWindow(hTree, POINT2PIXEL(10), POINT2PIXEL(10), POINT2PIXEL(256), POINT2PIXEL(256), TRUE);
		break;
	case WM_NCCREATE:
		{
			const HMODULE hModUser32 = GetModuleHandle(TEXT("user32.dll"));
			if (hModUser32)
			{
				typedef BOOL(WINAPI*fnTypeEnableNCScaling)(HWND);
				const fnTypeEnableNCScaling fnEnableNCScaling = (fnTypeEnableNCScaling)GetProcAddress(hModUser32, "EnableNonClientDpiScaling");
				if (fnEnableNCScaling)
				{
					fnEnableNCScaling(hWnd);
				}
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	case WM_DPICHANGED:
		SendMessage(hWnd, WM_APP, 0, 0);
		break;
	case WM_APP:
		GetScaling(hWnd, &uDpiX, &uDpiY);
		DeleteObject(hFont);
		hFont = CreateFontW(-POINT2PIXEL(18), 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, 0, 0, 0, 0, L"Yu Gothic UI");
		SendMessage(hTree, WM_SETFONT, (WPARAM)hFont, 0);
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
