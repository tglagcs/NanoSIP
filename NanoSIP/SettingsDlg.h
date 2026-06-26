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

#pragma once

#include "resource.h"
#include "global.h"
#include "FeatureCodesDlg.h"

class SettingsDlg :
	public CDialog
{
public:
	SettingsDlg(CWnd* pParent = NULL);	// standard constructor
	~SettingsDlg();
	enum { IDD = IDD_SETTINGS };

	FeatureCodesDlg* featureCodesDlg;

protected:
	CStringArray forwardingValues;

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void PostNcDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnUpdateSettings(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNMClickSyslinkFeatureCodes(NMHDR* pNMHDR, LRESULT* pResult);
#ifdef _GLOBAL_VIDEO
	afx_msg void OnBnClickedPreview();
#endif
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* );
	afx_msg void OnBnClickedRecordingBrowse();
	afx_msg void OnEnChangeRecording();
	afx_msg void OnBnClickedRecordingDefault();
	afx_msg void OnBnClickedDnsSrv();
	afx_msg void OnBnClickedStun();
};

