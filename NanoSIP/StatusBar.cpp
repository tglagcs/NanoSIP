/*
 * Copyright (C) 2011-2024 MicroSIP (http://www.microsip.org)
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

#include "StdAfx.h"
#include "StatusBar.h"
#include "global.h"
#include "Theme.h"

 // StatusBar   
IMPLEMENT_DYNAMIC(StatusBar, CStatusBar)
StatusBar::StatusBar()
{
	CStatusBar::CStatusBar();
}

StatusBar::~StatusBar()
{
}

BEGIN_MESSAGE_MAP(StatusBar, CStatusBar)
	//{{AFX_MSG_MAP(StatusBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

LRESULT StatusBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible())
	{
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
		if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
	}
	return 0L;
}

void StatusBar::OnLButtonUp(UINT nFlags, CPoint point)
{
}

void StatusBar::OnMouseMove(UINT nFlags, CPoint point)
{
}

BOOL StatusBar::OnEraseBkgnd(CDC* pDC)
{
	// OnPaint always fills the whole client area itself.
	return TRUE;
}

void StatusBar::OnPaint()
{
	CPaintDC dc(this);
	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(rcClient, Theme::Window);

	// NM_CUSTOMDRAW for this control's panes turned out unreliable (the
	// default text/border drawing kept winning), so the whole bar is
	// painted by hand here instead of trying to override its own WM_PAINT.
	CStatusBarCtrl& ctrl = GetStatusBarCtrl();
	int count = ctrl.GetParts(0, NULL);
	for (int i = 0; i < count; i++) {
		CRect rc;
		ctrl.GetRect(i, &rc);
		if (rc.Width() <= 0 || rc.Height() <= 0) {
			continue;
		}

		CRect rcText(rc);
		rcText.left += 4;
		HICON hIcon = ctrl.GetIcon(i);
		if (hIcon) {
			int iconSize = rc.Height() - 4;
			if (iconSize > 0) {
				int iconY = rc.top + (rc.Height() - iconSize) / 2;
				::DrawIconEx(dc.GetSafeHdc(), rcText.left, iconY, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
				rcText.left += iconSize + 4;
			}
		}

		CString text = ctrl.GetText(i);
		dc.SetTextColor(RGB(255, 255, 255));
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(text, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
}
