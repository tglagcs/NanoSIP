/*
 * Copyright (C) 2011-2025 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include "Theme.h"

#include <uxtheme.h>
#include <dwmapi.h>
#include <commctrl.h>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#define DWMWA_USE_IMMERSIVE_DARK_MODE_OLD 19

namespace {
	BOOL CALLBACK ThemeChildProc(HWND hWnd, LPARAM)
	{
		TCHAR className[64] = { 0 };
		GetClassName(hWnd, className, _countof(className));
		if (_tcsicmp(className, _T("Edit")) == 0 || _tcsicmp(className, _T("ComboBox")) == 0) {
			// Plain Edit/ComboBox controls ignore WM_CTLCOLOR brushes for their
			// themed background fill; DarkMode_CFD is the theme class Windows
			// itself uses for dark-mode-aware HWND edit/combo controls.
			SetWindowTheme(hWnd, L"DarkMode_CFD", NULL);
		}
		else {
			SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
		}
		return TRUE;
	}
}

void Theme::ApplyToWindow(HWND hWnd)
{
	if (!hWnd) {
		return;
	}
	BOOL dark = TRUE;
	if (FAILED(DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark)))) {
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE_OLD, &dark, sizeof(dark));
	}
	SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
	EnumChildWindows(hWnd, ThemeChildProc, 0);
}

void Theme::DisableControlTheme(HWND hWnd)
{
	if (hWnd) {
		SetWindowTheme(hWnd, L"", L"");
	}
}

HBRUSH Theme::BackgroundBrush()
{
	static CBrush brush(Window);
	return (HBRUSH)brush.GetSafeHandle();
}

HBRUSH Theme::ControlBrush()
{
	static CBrush brush(Control);
	return (HBRUSH)brush.GetSafeHandle();
}

HBRUSH Theme::HandleCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor) {
	case CTLCOLOR_EDIT:
	case CTLCOLOR_LISTBOX:
		pDC->SetTextColor(Text);
		pDC->SetBkColor(Control);
		return ControlBrush();
	case CTLCOLOR_STATIC:
	case CTLCOLOR_BTN:
		pDC->SetTextColor(Text);
		pDC->SetBkMode(TRANSPARENT);
		return BackgroundBrush();
	case CTLCOLOR_DLG:
	case CTLCOLOR_SCROLLBAR:
		return BackgroundBrush();
	default:
		return NULL;
	}
}

BOOL Theme::HandleEraseBkgnd(CWnd* pWnd, CDC* pDC)
{
	CRect rc;
	pWnd->GetClientRect(&rc);
	pDC->FillSolidRect(&rc, Window);
	return TRUE;
}

namespace {
	struct DarkMenuItemData {
		CString text;
		bool hasSubmenu;
		int index;       // position of this item in its menu
		int lastIndex;   // index of the last item in its menu
	};

	// Shared dark brush handed to the menu via MENUINFO::hbrBack. The menu
	// fills the gaps the owner-drawn item rects do NOT cover (its outer
	// margins, the gutter, the right/bottom padding) with this brush instead
	// of the system's light COLOR_MENU - that padding was the white border.
	HBRUSH DarkMenuBrush()
	{
		static HBRUSH brush = CreateSolidBrush(Theme::Window);
		return brush;
	}

	int MenuCornerRadius(HWND hMenuWnd)
	{
		HDC hdc = ::GetDC(hMenuWnd);
		int radius = MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 96);
		::ReleaseDC(hMenuWnd, hdc);
		return radius;
	}

	void ApplyRoundedRegion(HWND hMenuWnd, int width, int height)
	{
		if (width <= 0 || height <= 0) {
			return;
		}
		int radius = MenuCornerRadius(hMenuWnd);
		HRGN hRgn = CreateRoundRectRgn(0, 0, width + 1, height + 1, radius, radius);
		SetWindowRgn(hMenuWnd, hRgn, TRUE);
	}

	// Subclass proc for the live "#32768" popup window: just keeps the
	// rounded region matching the final menu size (the size at
	// HCBT_CREATEWND is only preliminary; WM_WINDOWPOSCHANGED reports the
	// real one). The visible border itself is drawn per-item in
	// DrawDarkMenuItem - see the comment there for why.
	LRESULT CALLBACK MenuSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		if (msg == WM_WINDOWPOSCHANGED) {
			WINDOWPOS* wp = (WINDOWPOS*)lParam;
			if (wp && wp->cx > 0 && wp->cy > 0) {
				ApplyRoundedRegion(hWnd, wp->cx, wp->cy);
			}
		}
		else if (msg == WM_NCDESTROY) {
			RemoveWindowSubclass(hWnd, MenuSubclassProc, uIdSubclass);
		}
		return DefSubclassProc(hWnd, msg, wParam, lParam);
	}

	void StyleDarkMenuWindow(HWND hMenuWnd)
	{
		// Drop visual-styles theming (the themed light frame/edge) and the
		// DWM drop shadow (the light halo) for this popup, leaving a flat
		// dark window we then clip to rounded corners.
		SetWindowTheme(hMenuWnd, L"", L"");
		DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
		DwmSetWindowAttribute(hMenuWnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
		SetWindowSubclass(hMenuWnd, MenuSubclassProc, 1, 0);
	}

	HHOOK g_hMenuCbtHook = NULL;

	LRESULT CALLBACK MenuCbtHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		// HCBT_CREATEWND fires for every window created on this thread the
		// moment it is created - popup menus never become the "active"
		// window (their owner stays active), so we cannot wait for
		// activation here.
		if (nCode == HCBT_CREATEWND) {
			HWND hWnd = (HWND)wParam;
			TCHAR className[16] = { 0 };
			GetClassName(hWnd, className, _countof(className));
			if (_tcscmp(className, _T("#32768")) == 0) {
				StyleDarkMenuWindow(hWnd);
			}
		}
		return CallNextHookEx(g_hMenuCbtHook, nCode, wParam, lParam);
	}
}

void Theme::ApplyDarkMenu(HMENU hMenu)
{
	if (!hMenu) {
		return;
	}
	if (!g_hMenuCbtHook) {
		g_hMenuCbtHook = SetWindowsHookEx(WH_CBT, MenuCbtHookProc, NULL, GetCurrentThreadId());
	}

	// Dark menu background brush (covers the padding around the owner-drawn
	// items). MIM_APPLYTOSUBMENUS makes it recurse, so this only needs to be
	// set on the top-level menu.
	MENUINFO mi = { sizeof(mi) };
	mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
	mi.hbrBack = DarkMenuBrush();
	SetMenuInfo(hMenu, &mi);

	int count = GetMenuItemCount(hMenu);
	for (int i = 0; i < count; i++) {
		TCHAR buf[256] = { 0 };
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_STRING;
		mii.dwTypeData = buf;
		mii.cch = _countof(buf);
		if (!GetMenuItemInfo(hMenu, i, TRUE, &mii)) {
			continue;
		}
		DarkMenuItemData* data = new DarkMenuItemData();
		if (!(mii.fType & MFT_SEPARATOR)) {
			data->text = buf;
		}
		data->hasSubmenu = (mii.hSubMenu != NULL);
		data->index = i;
		data->lastIndex = count - 1;

		MENUITEMINFO set = { sizeof(set) };
		set.fMask = MIIM_FTYPE | MIIM_DATA;
		set.fType = (mii.fType & MFT_SEPARATOR) | MFT_OWNERDRAW;
		set.dwItemData = (ULONG_PTR)data;
		SetMenuItemInfo(hMenu, i, TRUE, &set);

		if (mii.hSubMenu) {
			ApplyDarkMenu(mii.hSubMenu);
		}
	}
}

void Theme::ReleaseDarkMenuData(HMENU hMenu)
{
	if (!hMenu) {
		return;
	}
	int count = GetMenuItemCount(hMenu);
	for (int i = 0; i < count; i++) {
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_SUBMENU | MIIM_DATA;
		if (!GetMenuItemInfo(hMenu, i, TRUE, &mii)) {
			continue;
		}
		if (mii.hSubMenu) {
			ReleaseDarkMenuData(mii.hSubMenu);
		}
		delete (DarkMenuItemData*)mii.dwItemData;
	}
	if (g_hMenuCbtHook) {
		UnhookWindowsHookEx(g_hMenuCbtHook);
		g_hMenuCbtHook = NULL;
	}
}

void Theme::MeasureDarkMenuItem(MEASUREITEMSTRUCT* mis)
{
	if (!mis || mis->CtlType != ODT_MENU) {
		return;
	}
	DarkMenuItemData* data = (DarkMenuItemData*)mis->itemData;
	if (!data || data->text.IsEmpty()) {
		mis->itemWidth = 0;
		mis->itemHeight = 8;
		return;
	}
	HDC hdc = ::GetDC(NULL);
	CDC dc;
	dc.Attach(hdc);
	NONCLIENTMETRICS ncm = { sizeof(ncm) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	CFont font;
	font.CreateFontIndirect(&ncm.lfMenuFont);
	CFont* oldFont = dc.SelectObject(&font);
	CString text = data->text;
	int tabPos = text.Find(_T('\t'));
	CSize size = dc.GetTextExtent(tabPos >= 0 ? text.Left(tabPos) : text);
	int extra = 40;
	if (tabPos >= 0) {
		CSize accelSize = dc.GetTextExtent(text.Mid(tabPos + 1));
		extra += accelSize.cx + 20;
	}
	dc.SelectObject(oldFont);
	dc.Detach();
	::ReleaseDC(NULL, hdc);
	mis->itemWidth = size.cx + extra;
	mis->itemHeight = size.cy + 8;
}

void Theme::DrawDarkMenuItem(DRAWITEMSTRUCT* dis)
{
	if (!dis || dis->CtlType != ODT_MENU) {
		return;
	}
	CDC* pDC = CDC::FromHandle(dis->hDC);
	CRect rc(dis->rcItem);
	DarkMenuItemData* data = (DarkMenuItemData*)dis->itemData;
	bool isSeparator = !data || data->text.IsEmpty();
	bool selected = (dis->itemState & ODS_SELECTED) != 0;
	pDC->FillSolidRect(rc, selected && !isSeparator ? Selection : Window);

	// Each item draws its horizontal slice of the menu's full ROUNDED border
	// outline: we set a clip to this item's rect, then stroke the complete
	// rounded rectangle of the whole menu. The first item then renders the
	// top corner arcs, the last item the bottom arcs, and the middle items
	// just the straight side edges - together a single rounded frame that
	// matches the rounded window region. Drawing it as part of every item's
	// paint means the per-item repaints the menu does on hover redraw it too,
	// so it is never left behind and "erased" (the menu lays items out from
	// the window's top-left, so window-level borders kept getting clobbered).
	HWND hMenuWnd = ::WindowFromDC(dis->hDC);
	if (hMenuWnd && data) {
		CRect clientRc;
		::GetClientRect(hMenuWnd, &clientRc);
		int radius = MenuCornerRadius(hMenuWnd);
		// Outline the whole menu as a rounded rect sized to the CLIENT rect,
		// so the border sits on the menu's true right/bottom edge (covering
		// the couple-pixel margin the menu leaves between the items and the
		// window edge - the faint dark band). The item clips are stretched
		// out to the client right (every item) and the client bottom (the
		// last item) so those edges/arcs land inside a clip and actually get
		// drawn instead of being clipped away.
		int W = clientRc.Width();
		int H = clientRc.Height();
		bool isLast = (data->index == data->lastIndex);
		if (W > 0 && H > 0) {
			int clipBottom = isLast ? max(rc.bottom, (LONG)H) : rc.bottom;
			HRGN clip = CreateRectRgn(rc.left, rc.top, W, clipBottom);
			SelectClipRgn(pDC->m_hDC, clip);
			CPen borderPen(PS_SOLID, 1, Border);
			CPen* oldBorderPen = pDC->SelectObject(&borderPen);
			HGDIOBJ oldBrush = ::SelectObject(pDC->m_hDC, GetStockObject(NULL_BRUSH));
			pDC->RoundRect(0, 0, W, H, radius, radius);
			::SelectObject(pDC->m_hDC, oldBrush);
			pDC->SelectObject(oldBorderPen);
			SelectClipRgn(pDC->m_hDC, NULL);
			DeleteObject(clip);
		}
	}

	if (isSeparator) {
		CPen pen(PS_SOLID, 1, Border);
		CPen* oldPen = pDC->SelectObject(&pen);
		int y = rc.top + rc.Height() / 2;
		pDC->MoveTo(rc.left + 4, y);
		pDC->LineTo(rc.right - 4, y);
		pDC->SelectObject(oldPen);
		return;
	}

	NONCLIENTMETRICS ncm = { sizeof(ncm) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	CFont font;
	font.CreateFontIndirect(&ncm.lfMenuFont);
	CFont* oldFont = pDC->SelectObject(&font);

	pDC->SetBkMode(TRANSPARENT);
	bool disabled = (dis->itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;
	pDC->SetTextColor(disabled ? TextDisabled : Text);

	CString label = data->text;
	int tabPos = label.Find(_T('\t'));
	CString main = tabPos >= 0 ? label.Left(tabPos) : label;
	CString accel = tabPos >= 0 ? label.Mid(tabPos + 1) : CString();

	CRect rcText(rc);
	rcText.left += 24;
	rcText.right -= 8;
	pDC->DrawText(main, rcText, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
	if (!accel.IsEmpty()) {
		pDC->DrawText(accel, rcText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}
	if (dis->itemState & ODS_CHECKED) {
		CRect rcCheck(rc.left + 2, rc.top, rc.left + 22, rc.bottom);
		pDC->SetTextColor(Text);
		pDC->DrawText(_T("\x2713"), rcCheck, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	if (data->hasSubmenu) {
		CRect rcArrow(rc.right - 18, rc.top, rc.right - 4, rc.bottom);
		pDC->DrawText(_T("\x25B8"), rcArrow, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}

	pDC->SelectObject(oldFont);
}

LRESULT Theme::HandleTrackbarCustomDraw(NMHDR* pNMHDR)
{
	TCHAR className[64] = { 0 };
	GetClassName(pNMHDR->hwndFrom, className, _countof(className));

	// msctls_statusbar32 (the main status bar) is no longer handled here:
	// its NM_CUSTOMDRAW never reliably overrode the default pane/grip
	// painting, so StatusBar now paints itself entirely in StatusBar::OnPaint.

	if (_tcsicmp(className, _T("SysHeader32")) == 0) {
		LPNMCUSTOMDRAW pCD = (LPNMCUSTOMDRAW)pNMHDR;
		if (pCD->dwDrawStage == CDDS_PREPAINT) {
			return CDRF_NOTIFYITEMDRAW;
		}
		if (pCD->dwDrawStage == CDDS_ITEMPREPAINT) {
			CDC* pDC = CDC::FromHandle(pCD->hdc);
			CRect rc(pCD->rc);
			bool pressed = (pCD->uItemState & CDIS_SELECTED) != 0;
			pDC->FillSolidRect(rc, pressed ? ControlPressed : Control);
			CPen pen(PS_SOLID, 1, Border);
			CPen* oldPen = pDC->SelectObject(&pen);
			pDC->MoveTo(rc.right - 1, rc.top);
			pDC->LineTo(rc.right - 1, rc.bottom);
			pDC->SelectObject(oldPen);

			CHeaderCtrl* pHeader = (CHeaderCtrl*)CWnd::FromHandle(pNMHDR->hwndFrom);
			TCHAR text[256] = { 0 };
			HDITEM hdi = { 0 };
			hdi.mask = HDI_TEXT | HDI_FORMAT;
			hdi.pszText = text;
			hdi.cchTextMax = _countof(text) - 1;
			pHeader->GetItem((int)pCD->dwItemSpec, &hdi);

			CRect rcText(rc);
			rcText.DeflateRect(6, 0);
			UINT align = (hdi.fmt & HDF_RIGHT) ? DT_RIGHT : (hdi.fmt & HDF_CENTER) ? DT_CENTER : DT_LEFT;
			pDC->SetTextColor(Text);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(text, rcText, align | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			if (hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) {
				CRect rcArrow(rc);
				rcArrow.right -= 4;
				pDC->DrawText((hdi.fmt & HDF_SORTUP) ? _T("\x25B2") : _T("\x25BC"), rcArrow, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			}
			return CDRF_SKIPDEFAULT;
		}
		return CDRF_DODEFAULT;
	}

	if (_tcsicmp(className, _T("msctls_trackbar32")) != 0) {
		return -1;
	}

	LPNMCUSTOMDRAW pCD = (LPNMCUSTOMDRAW)pNMHDR;
	switch (pCD->dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
	{
		CDC* pDC = CDC::FromHandle(pCD->hdc);
		CRect rc(pCD->rc);
		switch (pCD->dwItemSpec) {
		case TBCD_CHANNEL:
			pDC->FillSolidRect(rc, Control);
			pDC->Draw3dRect(rc, Border, Border);
			return CDRF_SKIPDEFAULT;
		case TBCD_THUMB:
			pDC->FillSolidRect(rc, Selection);
			return CDRF_SKIPDEFAULT;
		default:
			return CDRF_DODEFAULT;
		}
	}
	}
	return CDRF_DODEFAULT;
}

