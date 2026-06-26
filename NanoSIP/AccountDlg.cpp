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
#include "define.h"

#include "AccountDlg.h"
#include "mainDlg.h"
#include "langpack.h"
#include "atlrx.h"
#include <ws2tcpip.h>
#include "json.h"
#include "Theme.h"

static CString transportItems[] = {
	_T("udp"),
	_T("tcp"),
	_T(""),
};
static CString transportValues[] = {
	_T("UDP"),
	_T("TCP"),
	_T("UDP+TCP"),
};

static const UINT advancedControlIds[] = {
	IDC_EDIT_PROXY, IDC_ACCOUNT_LBL_PROXY,
	IDC_ACCOUNT_DIALING_PREFIX, IDC_ACCOUNT_LBL_DIALING_PREFIX,
	IDC_ACCOUNT_DIAL_PLAN, IDC_ACCOUNT_LBL_DIAL_PLAN,
	IDC_ACCOUNT_HIDE_CID,
	IDC_TRANSPORT, IDC_ACCOUNT_LBL_TRANSPORT,
	IDC_PUBLIC_ADDR, IDC_ACCOUNT_LBL_PUBLIC_ADDR,
	IDC_ACCOUNT_REGISTER_REFRESH, IDC_ACCOUNT_LBL_REGISTER_REFRESH,
	IDC_ACCOUNT_KEEP_ALIVE, IDC_ACCOUNT_LBL_KEEP_ALIVE,
	IDC_REWRITE,
	IDC_ICE,
	IDC_SESSION_TIMER,
};

// Y offsets (dialog units, matching res/dialog.rc2 IDD_ACCOUNT_* macros)
#define ACCOUNT_DLU_WIDTH 236
#define ACCOUNT_DLU_HEIGHT_COLLAPSED 179
#define ACCOUNT_DLU_HEIGHT_EXPANDED 358
#define ACCOUNT_DLU_BUTTONS_Y_COLLAPSED 160
#define ACCOUNT_DLU_BUTTONS_Y_EXPANDED 339

AccountDlg::AccountDlg(CWnd* pParent /*=NULL*/)
: CDialog(AccountDlg::IDD, pParent)
{
	accountId = -1;
	advancedExpanded = false;
    if (!Create(IDD, pParent)) {
        AfxMessageBox(_T("Failed to create account window on your system"));
        exit(0);
    }
}

AccountDlg::~AccountDlg(void)
{
}

int AccountDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT);
	}
	return 0;
}

BOOL AccountDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Theme::ApplyToWindow(m_hWnd);

    CRect rect;
    GetWindowRect(&rect);
    width = rect.Width() * 96 / dpiY;
    height = rect.Height() * 96 / dpiY;

	TranslateDialog(this->m_hWnd);

	CString str;
	CEdit* edit;
	str.Format(_T("<a>%s</a>"),Translate(_T("Delete Account")));
	GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->SetWindowText(str);

	CComboBox *combobox;

	combobox = (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	int n = sizeof(transportItems) / sizeof(transportItems[0]);
	for (int i = 0; i < n; i++) {
		combobox->AddString(Translate(transportValues[i].GetBuffer()));
	}
	combobox->SetCurSel(0);

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	combobox->AddString(Translate(_T("Auto")));
	char buf[256]={0};
	if ( gethostname(buf, 256) == 0) {
		struct addrinfo* l_addrInfo = NULL;
		struct addrinfo l_addrInfoHints;
		ZeroMemory(&l_addrInfoHints, sizeof(addrinfo));
		l_addrInfoHints.ai_socktype = SOCK_STREAM;
		l_addrInfoHints.ai_family = PF_INET;
		if ( getaddrinfo(buf,NULL, &l_addrInfoHints,&l_addrInfo) == 0 ) {
			if (l_addrInfo) {
				struct addrinfo* l_addrInfoCurrent = l_addrInfo;
				for (l_addrInfoCurrent = l_addrInfo; l_addrInfoCurrent; l_addrInfoCurrent=l_addrInfoCurrent->ai_next) {
					struct sockaddr_in *ipv4 = (struct sockaddr_in *)l_addrInfoCurrent->ai_addr;
					char * ip = inet_ntoa(ipv4->sin_addr);
					combobox->AddString(CString(ip));
				}
			}
		}
	}
	combobox->SetCurSel(0);
	if (accountSettings.enableSTUN && !accountSettings.stun.IsEmpty()) {
		combobox->EnableWindow(FALSE);
	}

	str.Format(_T("<a>%s</a>"), Translate(_T("Advanced")));
	GetDlgItem(IDC_SYSLINK_ACCOUNT_ADVANCED)->SetWindowText(str);

	UpdateAdvancedLayout();

	return TRUE;
}

void AccountDlg::UpdateAdvancedLayout()
{
	for (int i = 0; i < sizeof(advancedControlIds) / sizeof(advancedControlIds[0]); i++) {
		GetDlgItem(advancedControlIds[i])->ShowWindow(advancedExpanded ? SW_SHOW : SW_HIDE);
	}

	CString str;
	str.Format(_T("<a>%s</a>"), Translate(advancedExpanded ? _T("Hide advanced") : _T("Advanced")));
	GetDlgItem(IDC_SYSLINK_ACCOUNT_ADVANCED)->SetWindowText(str);

	int buttonsY = advancedExpanded ? ACCOUNT_DLU_BUTTONS_Y_EXPANDED : ACCOUNT_DLU_BUTTONS_Y_COLLAPSED;

	CRect rc;
	rc.SetRect(83, buttonsY, 83 + 70, buttonsY + 14);
	MapDialogRect(&rc);
	GetDlgItem(IDOK)->MoveWindow(&rc);

	rc.SetRect(158, buttonsY, 158 + 70, buttonsY + 14);
	MapDialogRect(&rc);
	GetDlgItem(IDCANCEL)->MoveWindow(&rc);

	rc.SetRect(5, buttonsY + 2, 5 + 75, buttonsY + 2 + 8);
	MapDialogRect(&rc);
	GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->MoveWindow(&rc);

	CRect rcWindow;
	rcWindow.SetRect(0, 0, ACCOUNT_DLU_WIDTH, advancedExpanded ? ACCOUNT_DLU_HEIGHT_EXPANDED : ACCOUNT_DLU_HEIGHT_COLLAPSED);
	MapDialogRect(&rcWindow);

	CRect rcCurrent, rcClient;
	GetWindowRect(&rcCurrent);
	GetClientRect(&rcClient);
	int nonClientHeight = rcCurrent.Height() - rcClient.Height();
	SetWindowPos(NULL, 0, 0, rcCurrent.Width(), rcWindow.Height() + nonClientHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void AccountDlg::OnNMClickSyslinkAdvanced(NMHDR *pNMHDR, LRESULT *pResult)
{
	advancedExpanded = !advancedExpanded;
	UpdateAdvancedLayout();
	*pResult = 0;
}

void AccountDlg::OnDestroy()
{
	mainDlg->accountDlg = NULL;
	CDialog::OnDestroy();
}

void AccountDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(AccountDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDCANCEL, &AccountDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &AccountDlg::OnBnClickedOk)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ACCOUNT_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_ACCOUNT_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ACCOUNT_ADVANCED, &AccountDlg::OnNMClickSyslinkAdvanced)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_ACCOUNT_ADVANCED, &AccountDlg::OnNMClickSyslinkAdvanced)

END_MESSAGE_MAP()

HBRUSH AccountDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = Theme::HandleCtlColor(pDC, pWnd, nCtlColor);
	if (br) {
		return br;
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL AccountDlg::OnEraseBkgnd(CDC* pDC)
{
	return Theme::HandleEraseBkgnd(this, pDC);
}

void AccountDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	__super::OnSysCommand(nID, lParam);
}

void AccountDlg::OnClose() 
{
	DestroyWindow();
}

LRESULT AccountDlg::OnNcHitTest(CPoint point)
{
	ScreenToClient(&point);

	CRect rc;
	GetClientRect(&rc);

	//rc.bottom = rc.top + 100; 

	if (rc.PtInRect(point))
		return HTCAPTION;

	return CDialog::OnNcHitTest(point);
}

void AccountDlg::OnBnClickedCancel()
{
	OnClose();
}

void AccountDlg::Load(int id)
{
	CEdit* edit;
	CComboBox *combobox;
	CString str;
	int i;
	int n;
	bool found;
	
	accountId = id;
	if (accountSettings.AccountLoad(id, &m_Account)) {
		accountId = id;
		if (accountId  && accountSettings.accountId == accountId && !accountSettings.account.rememberPassword) {
			m_Account.username = accountSettings.account.username;
			m_Account.password = accountSettings.account.password;
			m_Account.rememberPassword = false;
		}
	}
	else {
		accountId = -1;
	}
	
	bool isEdit = (accountId > 0 && (!m_Account.username.IsEmpty() || accountId > 1));

	int show = id ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_ACCOUNT_REQUIRED_USERNAME)->ShowWindow(show);
	GetDlgItem(IDC_ACCOUNT_REQUIRED_DOMAIN)->ShowWindow(show);
	GetDlgItem(IDC_EDIT_SERVER)->EnableWindow(id);
	GetDlgItem(IDC_ACCOUNT_REGISTER_REFRESH)->EnableWindow(id);
	GetDlgItem(IDC_ACCOUNT_KEEP_ALIVE)->EnableWindow(id);
	GetDlgItem(IDC_REWRITE)->EnableWindow(id);

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_LABEL);
	edit->SetWindowText(m_Account.label);

	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->SetWindowText(m_Account.server);
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->SetWindowText(m_Account.proxy);
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	edit->SetWindowText(m_Account.domain);

	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->SetWindowText(m_Account.authID);

	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
	edit->SetWindowText(m_Account.username);

	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->SetPasswordChar(accountId == -1 ? 0 : '*');
	edit->SetWindowText(m_Account.password);

	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	edit->SetWindowText(m_Account.displayName);

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_DIALING_PREFIX);
	edit->SetWindowText(m_Account.dialingPrefix);

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_DIAL_PLAN);
	edit->SetWindowText(m_Account.dialPlan);

	((CButton*)GetDlgItem(IDC_ACCOUNT_HIDE_CID))->SetCheck(m_Account.hideCID);

	combobox = (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	n = sizeof(transportItems) / sizeof(transportItems[0]);
	found = false;
	for (int i = 0; i < n; i++) {
		if (m_Account.transport == transportItems[i]) {
			combobox->SetCurSel(i);
			found = true;
		}
	}
	if (!found) {
		combobox->SetCurSel(0);
	}

	combobox = (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	if (combobox->IsWindowEnabled()) {
		str = get_public_addr(&m_Account);
		if (!str.IsEmpty()) {
			combobox->SetWindowText(str);
		}
	}

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_REGISTER_REFRESH);
	str.Format(_T("%d"), m_Account.registerRefresh);
	edit->SetWindowText(str);

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_KEEP_ALIVE);
	str.Format(_T("%d"), m_Account.keepAlive);
	edit->SetWindowText(str);

	((CButton*)GetDlgItem(IDC_REWRITE))->SetCheck(m_Account.allowRewrite);

	((CButton*)GetDlgItem(IDC_ICE))->SetCheck(m_Account.ice);

	((CButton*)GetDlgItem(IDC_SESSION_TIMER))->SetCheck(m_Account.disableSessionTimer);
    if (isEdit) {
        GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->ShowWindow(SW_SHOW);
    }
    else {
        GetDlgItem(IDC_SYSLINK_ACCOUNT_DELETE)->ShowWindow(SW_HIDE);
    }
}

void AccountDlg::OnBnClickedOk()
{
	CEdit* edit;
	CString str;
	CComboBox *combobox;
	int i;

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_LABEL);
	edit->GetWindowText(str);
	m_Account.label=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->GetWindowText(str);
	m_Account.server=str.Trim();
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->GetWindowText(str);
	m_Account.proxy=str.Trim();
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	edit->GetWindowText(str);
	m_Account.domain=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->GetWindowText(str);
	m_Account.authID=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
	edit->GetWindowText(str);
	m_Account.username=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->GetWindowText(str);
	m_Account.password=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	edit->GetWindowText(str);
	m_Account.displayName=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_DIALING_PREFIX);
	edit->GetWindowText(str);
	m_Account.dialingPrefix=str.Trim();

	edit = (CEdit*)GetDlgItem(IDC_ACCOUNT_DIAL_PLAN);
	edit->GetWindowText(str);
	m_Account.dialPlan = str.Trim();

	m_Account.hideCID = ((CButton*)GetDlgItem(IDC_ACCOUNT_HIDE_CID))->GetCheck();

	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	m_Account.transport = transportItems[combobox->GetCurSel()];

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	if (combobox->IsWindowEnabled()) {
		i = combobox->GetCurSel();
		combobox->GetWindowText(m_Account.publicAddr);
		if (m_Account.publicAddr == Translate(_T("Auto")))
		{
			m_Account.publicAddr = _T("");
		}
	}

	m_Account.rememberPassword = 1;

	GetDlgItem(IDC_ACCOUNT_REGISTER_REFRESH)->GetWindowText(str);
	m_Account.registerRefresh = _wtoi(str);
	if (m_Account.registerRefresh <= 0) {
		m_Account.registerRefresh = PJSUA_REG_INTERVAL;
	}

	GetDlgItem(IDC_ACCOUNT_KEEP_ALIVE)->GetWindowText(str);
	m_Account.keepAlive = _wtoi(str);
	if (m_Account.keepAlive < 0) {
		m_Account.keepAlive = 15;
	}

	m_Account.allowRewrite = ((CButton*)GetDlgItem(IDC_REWRITE))->GetCheck();

	m_Account.ice = ((CButton*)GetDlgItem(IDC_ICE))->GetCheck();

	m_Account.disableSessionTimer = ((CButton*)GetDlgItem(IDC_SESSION_TIMER))->GetCheck();

	if (
		m_Account.domain.IsEmpty() ||
		m_Account.username.IsEmpty()) {
		if (accountId != 0) {
			CString str;
			str.Append(Translate(_T("Please fill out at least the required fields marked with *.")));
			str.AppendFormat(_T(" %s"), Translate(_T("Ask your SIP provider how to configure the account correctly.")));
			AfxMessageBox(str);
			return;
		}
	}

	this->ShowWindow(SW_HIDE);
	mainDlg->accountDlg = NULL;

	if (accountId == -1) { // find id for new account
		Account dummy;
		int i = 1;
		while (true) {
			if (!accountSettings.AccountLoad(i, &dummy)) {
				break;
			}
			i++;
		}
		accountId = i;
	}

	accountSettings.AccountSave(accountId, &m_Account);

	if (accountId) {
		mainDlg->PJAccountDelete(true);

		accountSettings.accountId = accountId;
		accountSettings.account = m_Account;
		accountSettings.AccountLoad(accountSettings.accountId, &accountSettings.account);
		if (!m_Account.rememberPassword) {
			accountSettings.account.username = m_Account.username;
			accountSettings.account.password = m_Account.password;
			accountSettings.account.rememberPassword = false;
		}
		mainDlg->OnAccountChanged();
		mainDlg->InitUI();
		accountSettings.SettingsSave();
		mainDlg->PJAccountAdd();
	} else {
		// local account
		mainDlg->PJAccountDeleteLocal();
		accountSettings.AccountLoad(0, &accountSettings.accountLocal);
		mainDlg->PJAccountAddLocal();
	}
	OnClose();
}

void AccountDlg::OnNMClickSyslinkDelete(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (accountId>0 && AfxMessageBox(Translate(_T("Are you sure you want to delete?")), MB_YESNO)==IDYES) {
		this->ShowWindow(SW_HIDE);
		mainDlg->accountDlg = NULL;

		Account account;
		int i = accountId;
		while (true) {
			if (!accountSettings.AccountLoad(i+1,&account)) {
				break;
			}
			accountSettings.AccountSave(i,&account);
			i++;
		}
		accountSettings.AccountDelete(i);
		if (accountId && accountId == accountSettings.accountId) {
			mainDlg->PJAccountDelete(true);
			if (i>1) {
				accountSettings.accountId = 1;
				accountSettings.AccountLoad(accountSettings.accountId,&accountSettings.account);
				mainDlg->OnAccountChanged();
				mainDlg->InitUI();
				mainDlg->PJAccountAdd();
			} else {
				accountSettings.accountId = 0;
				mainDlg->OnAccountChanged();
				mainDlg->InitUI();
			}
			accountSettings.SettingsSave();
		}
		OnClose();
	}
	*pResult = 0;
}



