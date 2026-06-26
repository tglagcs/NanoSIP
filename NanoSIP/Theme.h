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

#pragma once

// Application-wide dark color palette. There is no light theme anymore,
// these colors are used unconditionally throughout the UI.
namespace Theme {
	const COLORREF Window       = RGB(32, 32, 32);   // dialog / main window background
	const COLORREF WindowAlt    = RGB(24, 24, 24);   // slightly darker panels
	const COLORREF Control      = RGB(45, 45, 48);   // edit fields, list background
	const COLORREF ControlHover = RGB(60, 60, 64);
	const COLORREF ControlPressed = RGB(70, 70, 74);
	const COLORREF Border       = RGB(63, 63, 70);
	const COLORREF Selection    = RGB(9, 71, 113);   // selected list row / tab
	const COLORREF Text         = RGB(225, 225, 225);
	const COLORREF TextDim      = RGB(150, 150, 150);
	const COLORREF TextDisabled = RGB(110, 110, 110);

	// Applies the dark title bar and asks the OS to render the window's
	// native chrome (scrollbars etc.) using the dark Explorer theme.
	void ApplyToWindow(HWND hWnd);

	// Drops visual-styles theming for a single control. Needed for
	// AUTORADIOBUTTON: under DarkMode_Explorer its caption is painted by the
	// theme engine itself and ignores the dark text color set in
	// HandleCtlColor, so it stays black. Without a theme the control falls
	// back to classic owner-painted text, which does honor that color.
	void DisableControlTheme(HWND hWnd);

	// Returns the shared dark dialog-background brush (CTLCOLOR_DLG / OnEraseBkgnd).
	HBRUSH BackgroundBrush();

	// Returns the shared dark control brush (CTLCOLOR_EDIT / CTLCOLOR_LISTBOX).
	HBRUSH ControlBrush();

	// Shared WM_CTLCOLOR handling for plain dialogs. Pass through the result
	// of the base class first; this overrides it for the standard cases.
	HBRUSH HandleCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	// Shared WM_ERASEBKGND handling - fills the client area with the dark
	// window background color.
	BOOL HandleEraseBkgnd(CWnd* pWnd, CDC* pDC);

	// Converts every item of hMenu (and its submenus) to owner-draw so it can
	// be painted with the dark palette. Call this right before
	// TrackPopupMenu, and call ReleaseDarkMenuData right after it returns to
	// free the item data this allocates.
	void ApplyDarkMenu(HMENU hMenu);
	void ReleaseDarkMenuData(HMENU hMenu);

	// Wire these up from the owning window's OnMeasureItem/OnDrawItem when
	// nIDCtl == 0 (menus pass 0 as the control id).
	void MeasureDarkMenuItem(MEASUREITEMSTRUCT* mis);
	void DrawDarkMenuItem(DRAWITEMSTRUCT* dis);

	// Call from OnNotify when pNMHDR->code == NM_CUSTOMDRAW. Paints
	// msctls_trackbar32 controls (volume sliders) with the dark palette;
	// returns -1 for any other control so the caller can fall through to
	// the base class implementation.
	LRESULT HandleTrackbarCustomDraw(NMHDR* pNMHDR);
}
