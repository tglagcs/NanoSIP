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

#include "StdAfx.h"
#include "SettingsDlg.h"
#include "mainDlg.h"
#include "settings.h"
#include "global.h"
#include "Preview.h"
#include "langpack.h"
#include "Theme.h"
#include <afxshellmanager.h>

SettingsDlg::SettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SettingsDlg::IDD, pParent)
{
    if (!Create(IDD, pParent)) {
        AfxMessageBox(_T("Failed to create settings window on your system"));
        exit(0);
    }
}

SettingsDlg::~SettingsDlg(void)
{
}

int SettingsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL SettingsDlg::OnInitDialog()
{
	CComboBox *combobox;
	CComboBox *combobox2;
	unsigned count;
	int i;
	CString str;

	CDialog::OnInitDialog();
	Theme::ApplyToWindow(m_hWnd);

	featureCodesDlg = NULL;

	TranslateDialog(this->m_hWnd);

	str.Format(_T("<a>%s</a>"),Translate(_T("Feature Codes")));
	GetDlgItem(IDC_SETTINGS_FEATURE_CODES)->SetWindowText(str);

	((CSliderCtrl*)GetDlgItem(IDC_SETTINGS_VOLUME_RING))->SetRange(0, 100);
	((CSliderCtrl*)GetDlgItem(IDC_SETTINGS_VOLUME_RING))->SetPos(accountSettings.volumeRing);
	GetDlgItem(IDC_SETTINGS_RECORDING)->SetWindowText(accountSettings.recordingPath);
	if (accountSettings.recordingFormat == _T("wav")) {
		CheckRadioButton(IDC_SETTINGS_RECORDING_MP3, IDC_SETTINGS_RECORDING_WAV, IDC_SETTINGS_RECORDING_WAV);
	}
	else {
		CheckRadioButton(IDC_SETTINGS_RECORDING_MP3, IDC_SETTINGS_RECORDING_WAV, IDC_SETTINGS_RECORDING_MP3);
	}
	((CButton*)GetDlgItem(IDC_SETTINGS_RECORDING_CHECKBOX))->SetCheck(accountSettings.autoRecording);
	((CButton*)GetDlgItem(IDC_SETTINGS_RECORDING_BUTTON))->SetCheck(accountSettings.recordingButton);
	Theme::DisableControlTheme(GetDlgItem(IDC_SETTINGS_RECORDING_MP3)->m_hWnd);
	Theme::DisableControlTheme(GetDlgItem(IDC_SETTINGS_RECORDING_WAV)->m_hWnd);
	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_MICROPHONE);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	pjmedia_aud_dev_info aud_dev_info[PJMEDIA_AUD_MAX_DEVS];
	if (is_pjsua_running()) {
		count = PJMEDIA_AUD_MAX_DEVS;
		pjsua_enum_aud_devs(aud_dev_info, &count);
	}
	else {
		count = 0;
	}
	for (unsigned i = 0; i < count; i++)
	{
		if (aud_dev_info[i].input_count) {
			CString audDevName = MSIP::Utf8DecodeUni(aud_dev_info[i].name);
			combobox->AddString(audDevName);
			if (!accountSettings.audioInputDevice.Compare(audDevName)) {
				combobox->SetCurSel(combobox->GetCount() - 1);
			}
		}
	}
	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_SPEAKERS);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	CComboBox* comboboxRing = (CComboBox*)GetDlgItem(IDC_SETTINGS_RING);
	comboboxRing->AddString(Translate(_T("Default")));
	comboboxRing->SetCurSel(0);
	for (unsigned i = 0; i < count; i++)
	{
		if (aud_dev_info[i].output_count) {
			CString audDevName = MSIP::Utf8DecodeUni(aud_dev_info[i].name);
			combobox->AddString(audDevName);
			comboboxRing->AddString(audDevName);
			if (!accountSettings.audioOutputDevice.Compare(audDevName)) {
				combobox->SetCurSel(combobox->GetCount() - 1);
			}
			if (!accountSettings.audioRingDevice.Compare(audDevName)) {
				comboboxRing->SetCurSel(comboboxRing->GetCount() - 1);
			}
		}
	}
	((CButton*)GetDlgItem(IDC_SETTINGS_MIC_AMPLIF))->SetCheck(accountSettings.micAmplification);
	((CButton*)GetDlgItem(IDC_SETTINGS_SW_ADJUST))->SetCheck(accountSettings.swLevelAdjustment);

	((CButton*)GetDlgItem(IDC_SETTINGS_VAD))->SetCheck(accountSettings.vad);
	((CButton*)GetDlgItem(IDC_SETTINGS_EC))->SetCheck(accountSettings.ec);

#ifdef _GLOBAL_VIDEO
	((CButton*)GetDlgItem(IDC_SETTINGS_DISABLE_VIDEO))->SetCheck(accountSettings.disableVideo);

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_VID_CAP_DEV);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);

	pjmedia_vid_dev_info vid_dev_info[PJMEDIA_VID_DEV_MAX_DEVS];
	if (is_pjsua_running()) {
		count = PJMEDIA_VID_DEV_MAX_DEVS;
		pjsua_vid_enum_devs(vid_dev_info, &count);
	}
	else {
		count = 0;
	}
	for (unsigned i = 0; i < count; i++)
	{
		if (vid_dev_info[i].fmt_cnt && (vid_dev_info[i].dir == PJMEDIA_DIR_ENCODING || vid_dev_info[i].dir == PJMEDIA_DIR_ENCODING_DECODING))
		{
			CString vidDevName = MSIP::Utf8DecodeUni(vid_dev_info[i].name);
			combobox->AddString(vidDevName);
			if (!accountSettings.videoCaptureDevice.Compare(vidDevName))
			{
				combobox->SetCurSel(combobox->GetCount() - 1);
			}
		}
	}

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_VIDEO_CODEC);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	if (is_pjsua_running()) {
		count = PJMEDIA_CODEC_MGR_MAX_CODECS;
		pjsua_vid_enum_codecs(codec_info, &count);
	}
	else {
		count = 0;
	}
	for (unsigned i = 0; i < count; i++)
	{
		combobox->AddString(MSIP::PjToStr(&codec_info[i].codec_id));
		if (!accountSettings.videoCodec.Compare(MSIP::PjToStr(&codec_info[i].codec_id)))
		{
			combobox->SetCurSel(combobox->GetCount() - 1);
		}
	}

	((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_H264))->SetCheck(accountSettings.videoH264);
	((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_H263))->SetCheck(accountSettings.videoH263);
	((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_VP8))->SetCheck(accountSettings.videoVP8);
	((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_VP9))->SetCheck(accountSettings.videoVP9);
	if (!accountSettings.videoBitrate) {
		if (is_pjsua_running()) {
			const pj_str_t codec_id = { "H264", 4 };
			pjmedia_vid_codec_param param;
			pjsua_vid_codec_get_param(&codec_id, &param);
			accountSettings.videoBitrate = param.enc_fmt.det.vid.max_bps / 1000;
		}
	}
	str.Format(_T("%d"), accountSettings.videoBitrate);
	GetDlgItem(IDC_SETTINGS_VIDEO_BITRATE)->SetWindowText(str);
#endif

	((CButton*)GetDlgItem(IDC_SETTINGS_RPORT))->SetCheck(accountSettings.rport);
	str.Format(_T("%d"), accountSettings.sourcePort);
	GetDlgItem(IDC_SETTINGS_SOURCE_PORT)->SetWindowText(str);
	str.Format(_T("%d"), accountSettings.rtpPortMin);
	GetDlgItem(IDC_SETTINGS_RTP_PORT_MIN)->SetWindowText(str);
	str.Format(_T("%d"), accountSettings.rtpPortMax);
	GetDlgItem(IDC_SETTINGS_RTP_PORT_MAX)->SetWindowText(str);

	GetDlgItem(IDC_SETTINGS_DNS_SRV_NS)->SetWindowText(accountSettings.dnsSrvNs);
	((CButton*)GetDlgItem(IDC_SETTINGS_DNS_SRV_CHECKBOX))->SetCheck(accountSettings.dnsSrv);

	GetDlgItem(IDC_SETTINGS_STUN)->SetWindowText(accountSettings.stun);
	((CButton*)GetDlgItem(IDC_SETTINGS_STUN_CHECKBOX))->SetCheck(accountSettings.enableSTUN);

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_DTMF_METHOD);
	combobox->AddString(Translate(_T("Auto")));
	combobox->AddString(Translate(_T("In-band")));
	combobox->AddString(Translate(_T("RFC2833")));
	combobox->AddString(Translate(_T("SIP-INFO")));
	combobox->SetCurSel(accountSettings.DTMFMethod);

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_FWD);
	combobox->AddString(Translate(_T("No")));
	forwardingValues.Add(_T(""));
	combobox->SetCurSel(0);
	combobox->AddString(Translate(_T("Control Button")));
	forwardingValues.Add(_T("button"));
	combobox->AddString(Translate(_T("All Calls")));
	forwardingValues.Add(_T("all"));
	for (i = 0; i < forwardingValues.GetCount(); i++) {
		if (accountSettings.forwarding == forwardingValues.GetAt(i)) {
			combobox->SetCurSel(i);
			break;
		}
	}
	GetDlgItem(IDC_SETTINGS_FWD_NUMBER)->SetWindowText(accountSettings.forwardingNumber);
	str.Format(_T("%d"), accountSettings.forwardingDelay);
	GetDlgItem(IDC_SETTINGS_FWD_DELAY)->SetWindowText(str);

	((CButton*)GetDlgItem(IDC_SETTINGS_MEDIA_BUTTONS))->SetCheck(accountSettings.enableMediaButtons);
	((CButton*)GetDlgItem(IDC_SETTINGS_HID))->SetCheck(accountSettings.headsetSupport);
	((CButton*)GetDlgItem(IDC_SETTINGS_LOCAL_DTMF))->SetCheck(accountSettings.localDTMF);
	((CButton*)GetDlgItem(IDC_SETTINGS_BRING_TO_FRONT))->SetCheck(accountSettings.bringToFrontOnIncoming);
	((CButton*)GetDlgItem(IDC_SETTINGS_MULTI_MONITOR))->SetCheck(accountSettings.multiMonitor);
	((CButton*)GetDlgItem(IDC_SETTINGS_NETWORK_CHANGES))->SetCheck(accountSettings.networkChanges);
	((CButton*)GetDlgItem(IDC_SETTINGS_ENABLE_LOCAL))->SetCheck(accountSettings.enableLocalAccount);

	CRegKey regKey;
	CString rab;
	LPTSTR ptr;
	ULONG pnChars;
	rab=_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	if (regKey.Open(HKEY_CURRENT_USER, rab, KEY_READ) == ERROR_SUCCESS) {
		ptr = str.GetBuffer(255);
		pnChars = 256;
		regKey.QueryStringValue(_T(_GLOBAL_NAME_NICE), ptr, &pnChars);
		str.ReleaseBuffer();
		regKey.Close();
		str.MakeLower();
		CString tmp = accountSettings.exeFile;
		tmp.MakeLower();
		if (str.Find(tmp) != -1) {
			((CButton*)GetDlgItem(IDC_SETTINGS_STARTUP))->SetCheck(1);
		}
	}

	return TRUE;
}

void SettingsDlg::OnDestroy()
{
	mainDlg->settingsDlg = NULL;
	CDialog::OnDestroy();
}

void SettingsDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(SettingsDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDCANCEL, &SettingsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &SettingsDlg::OnBnClickedOk)
	ON_MESSAGE(UM_UPDATE_SETTINGS, &SettingsDlg::OnUpdateSettings)
	ON_NOTIFY(NM_CLICK, IDC_SETTINGS_FEATURE_CODES, &SettingsDlg::OnNMClickSyslinkFeatureCodes)
	ON_NOTIFY(NM_RETURN, IDC_SETTINGS_FEATURE_CODES, &SettingsDlg::OnNMClickSyslinkFeatureCodes)
#ifdef _GLOBAL_VIDEO
	ON_BN_CLICKED(IDC_SETTINGS_PREVIEW, &SettingsDlg::OnBnClickedPreview)
#endif
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SETTINGS_RECORDING_BROWSE, &SettingsDlg::OnBnClickedRecordingBrowse)
	ON_EN_CHANGE(IDC_SETTINGS_RECORDING, &SettingsDlg::OnEnChangeRecording)
	ON_BN_CLICKED(IDC_SETTINGS_RECORDING_DEFAULT, &SettingsDlg::OnBnClickedRecordingDefault)
	ON_BN_CLICKED(IDC_SETTINGS_DNS_SRV_CHECKBOX, &SettingsDlg::OnBnClickedDnsSrv)
	ON_BN_CLICKED(IDC_SETTINGS_STUN_CHECKBOX, &SettingsDlg::OnBnClickedStun)
END_MESSAGE_MAP()

HBRUSH SettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = Theme::HandleCtlColor(pDC, pWnd, nCtlColor);
	if (br) {
		return br;
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL SettingsDlg::OnEraseBkgnd(CDC* pDC)
{
	return Theme::HandleEraseBkgnd(this, pDC);
}

BOOL SettingsDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	if (pNMHDR->code == NM_CUSTOMDRAW) {
		LRESULT r = Theme::HandleTrackbarCustomDraw(pNMHDR);
		if (r != -1) {
			*pResult = r;
			return TRUE;
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void SettingsDlg::OnClose()
{
	DestroyWindow();
}

void SettingsDlg::OnBnClickedCancel()
{
	mainDlg->PlayerStop();
	OnClose();
}

void SettingsDlg::OnBnClickedOk()
{
	this->ShowWindow(SW_HIDE);
	mainDlg->PJDestroy();
	PostMessage(UM_UPDATE_SETTINGS, 0, 0);
}

LRESULT SettingsDlg::OnUpdateSettings(WPARAM wParam, LPARAM lParam)
{
	CString str; 

	CComboBox *combobox;
	int i;
	GetDlgItem(IDC_SETTINGS_MICROPHONE)->GetWindowText(accountSettings.audioInputDevice);
	if (accountSettings.audioInputDevice == Translate(_T("Default")))
	{
		accountSettings.audioInputDevice = _T("");
	}

	GetDlgItem(IDC_SETTINGS_SPEAKERS)->GetWindowText(accountSettings.audioOutputDevice);
	if (accountSettings.audioOutputDevice == Translate(_T("Default")))
	{
		accountSettings.audioOutputDevice = _T("");
	}
	GetDlgItem(IDC_SETTINGS_RING)->GetWindowText(accountSettings.audioRingDevice);
	if (accountSettings.audioRingDevice == Translate(_T("Default")))
	{
		accountSettings.audioRingDevice = _T("");
	}
	accountSettings.micAmplification = ((CButton*)GetDlgItem(IDC_SETTINGS_MIC_AMPLIF))->GetCheck();
	accountSettings.swLevelAdjustment = ((CButton*)GetDlgItem(IDC_SETTINGS_SW_ADJUST))->GetCheck();

	accountSettings.vad = ((CButton*)GetDlgItem(IDC_SETTINGS_VAD))->GetCheck();
	accountSettings.ec = ((CButton*)GetDlgItem(IDC_SETTINGS_EC))->GetCheck();

#ifdef _GLOBAL_VIDEO
	accountSettings.disableVideo = ((CButton*)GetDlgItem(IDC_SETTINGS_DISABLE_VIDEO))->GetCheck();
	GetDlgItem(IDC_SETTINGS_VID_CAP_DEV)->GetWindowText(accountSettings.videoCaptureDevice);
	if (accountSettings.videoCaptureDevice == Translate(_T("Default")))
	{
		accountSettings.videoCaptureDevice = _T("");
	}
	GetDlgItem(IDC_SETTINGS_VIDEO_CODEC)->GetWindowText(accountSettings.videoCodec);
	if (accountSettings.videoCodec == Translate(_T("Default")))
	{
		accountSettings.videoCodec = _T("");
	}
	accountSettings.videoH264 = ((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_H264))->GetCheck();
	accountSettings.videoH263 = ((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_H263))->GetCheck();
	accountSettings.videoVP8 = ((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_VP8))->GetCheck();
	accountSettings.videoVP9 = ((CButton*)GetDlgItem(IDC_SETTINGS_VIDEO_VP9))->GetCheck();
	GetDlgItem(IDC_SETTINGS_VIDEO_BITRATE)->GetWindowText(str);
	accountSettings.videoBitrate = _wtoi(str);
#endif

	accountSettings.rport = ((CButton*)GetDlgItem(IDC_SETTINGS_RPORT))->GetCheck();
	GetDlgItem(IDC_SETTINGS_SOURCE_PORT)->GetWindowText(str);
	accountSettings.sourcePort = _wtoi(str);
	GetDlgItem(IDC_SETTINGS_RTP_PORT_MIN)->GetWindowText(str);
	accountSettings.rtpPortMin = _wtoi(str);
	GetDlgItem(IDC_SETTINGS_RTP_PORT_MAX)->GetWindowText(str);
	accountSettings.rtpPortMax = _wtoi(str);

	GetDlgItem(IDC_SETTINGS_DNS_SRV_NS)->GetWindowText(accountSettings.dnsSrvNs);
	accountSettings.dnsSrvNs.Trim();
	if (!accountSettings.dnsSrvNs.IsEmpty()) {
		accountSettings.dnsSrv = ((CButton*)GetDlgItem(IDC_SETTINGS_DNS_SRV_CHECKBOX))->GetCheck();
	}
	else {
		accountSettings.dnsSrv = false;
	}

	GetDlgItem(IDC_SETTINGS_STUN)->GetWindowText(accountSettings.stun);
	accountSettings.stun.Trim();
	if (!accountSettings.stun.IsEmpty()) {
		accountSettings.enableSTUN = ((CButton*)GetDlgItem(IDC_SETTINGS_STUN_CHECKBOX))->GetCheck();
	}
	else {
		accountSettings.enableSTUN = false;
	}

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_DTMF_METHOD);
	accountSettings.DTMFMethod = combobox->GetCurSel();

	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_FWD);
	accountSettings.forwarding = forwardingValues.GetAt(combobox->GetCurSel());
	GetDlgItem(IDC_SETTINGS_FWD_NUMBER)->GetWindowText(accountSettings.forwardingNumber);
	GetDlgItem(IDC_SETTINGS_FWD_DELAY)->GetWindowText(str);
	accountSettings.forwardingDelay = _wtoi(str);

	accountSettings.enableMediaButtons = ((CButton*)GetDlgItem(IDC_SETTINGS_MEDIA_BUTTONS))->GetCheck();
	accountSettings.headsetSupport = ((CButton*)GetDlgItem(IDC_SETTINGS_HID))->GetCheck();
	accountSettings.localDTMF = ((CButton*)GetDlgItem(IDC_SETTINGS_LOCAL_DTMF))->GetCheck();
	accountSettings.bringToFrontOnIncoming = ((CButton*)GetDlgItem(IDC_SETTINGS_BRING_TO_FRONT))->GetCheck();
	accountSettings.multiMonitor = ((CButton*)GetDlgItem(IDC_SETTINGS_MULTI_MONITOR))->GetCheck();
	accountSettings.networkChanges = ((CButton*)GetDlgItem(IDC_SETTINGS_NETWORK_CHANGES))->GetCheck();
	accountSettings.volumeRing = ((CSliderCtrl*)GetDlgItem(IDC_SETTINGS_VOLUME_RING))->GetPos();
	GetDlgItem(IDC_SETTINGS_RECORDING)->GetWindowText(accountSettings.recordingPath);
	accountSettings.recordingPath.Trim();
	accountSettings.recordingFormat = IsDlgButtonChecked(IDC_SETTINGS_RECORDING_MP3) ? _T("mp3") : _T("wav");
	accountSettings.autoRecording = ((CButton*)GetDlgItem(IDC_SETTINGS_RECORDING_CHECKBOX))->GetCheck();
	accountSettings.recordingButton = ((CButton*)GetDlgItem(IDC_SETTINGS_RECORDING_BUTTON))->GetCheck();
	accountSettings.enableLocalAccount = ((CButton*)GetDlgItem(IDC_SETTINGS_ENABLE_LOCAL))->GetCheck();

	msip_startup_set(((CButton*)GetDlgItem(IDC_SETTINGS_STARTUP))->GetCheck());

	accountSettings.SettingsSave();

	if (accountSettings.singleMode) {
		mainDlg->messagesDlg->OnClose();
	}
#ifdef _GLOBAL_VIDEO
	mainDlg->messagesDlg->UpdateCallButton();
#endif
	mainDlg->pageDialer->RebuildButtons();
	mainDlg->pageDialer->UpdateCallButton();
	mainDlg->PJCreate();
	mainDlg->OnAccountChanged();
	mainDlg->PJAccountAdd();
	OnClose();
	return 0;
}

void SettingsDlg::OnHScroll(UINT nSBCode, UINT, CScrollBar* sender)
{
	if (sender == GetDlgItem(IDC_SETTINGS_VOLUME_RING)) {
		if (nSBCode == SB_ENDSCROLL) {
			int volumeRingOld = accountSettings.volumeRing;
			accountSettings.volumeRing = ((CSliderCtrl*)GetDlgItem(IDC_SETTINGS_VOLUME_RING))->GetPos();
			CString ringtone = accountSettings.ringtone.IsEmpty() ? CString(_T("ringtone.wav")) : accountSettings.ringtone;
			mainDlg->onPlayerPlay(MSIP_SOUND_CUSTOM_NOLOOP, (LPARAM)&ringtone);
			accountSettings.volumeRing = volumeRingOld;
		}
	}
}

void SettingsDlg::OnBnClickedRecordingBrowse()
{
	CString strOutFolder;
	CString str;
	CShellManager* pShellManager = ((CWinAppEx*)AfxGetApp())->GetShellManager();
	GetDlgItem(IDC_SETTINGS_RECORDING)->GetWindowText(str);
	if (str.IsEmpty() || PathIsRelative(str)) {
		TCHAR currentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, currentDir);
		strOutFolder = currentDir;
		if (!str.IsEmpty()) {
			strOutFolder.AppendFormat(_T("\\%s"), str);
		}
	}
	else {
		strOutFolder = str;
	}
	if (pShellManager->BrowseForFolder(strOutFolder,this, strOutFolder))
	{
		GetDlgItem(IDC_SETTINGS_RECORDING)->SetWindowText(strOutFolder);
	}
}

void SettingsDlg::OnEnChangeRecording()
{
	CString str;
	GetDlgItem(IDC_SETTINGS_RECORDING)->GetWindowText(str);
	GetDlgItem(IDC_SETTINGS_RECORDING_DEFAULT)->EnableWindow(str.GetLength() > 0);
}

void SettingsDlg::OnBnClickedRecordingDefault()
{
	GetDlgItem(IDC_SETTINGS_RECORDING)->SetWindowText(_T(""));
}

void SettingsDlg::OnNMClickSyslinkFeatureCodes(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!featureCodesDlg) {
		featureCodesDlg = new FeatureCodesDlg(this);
	}
	else {
		featureCodesDlg->SetForegroundWindow();
	}
}

#ifdef _GLOBAL_VIDEO
void SettingsDlg::OnBnClickedPreview()
{
	CComboBox *combobox;
	combobox = (CComboBox*)GetDlgItem(IDC_SETTINGS_VID_CAP_DEV);
	CString name;
	combobox->GetWindowText(name);
	if (!mainDlg->previewWin) {
		mainDlg->previewWin = new Preview(mainDlg);
	}
	mainDlg->previewWin->Start(mainDlg->VideoCaptureDeviceId(name));
}
#endif

void SettingsDlg::OnBnClickedDnsSrv()
{
	if (((CButton*)GetDlgItem(IDC_SETTINGS_DNS_SRV_CHECKBOX))->GetCheck()) {
		CString str;
		GetDlgItem(IDC_SETTINGS_DNS_SRV_NS)->GetWindowText(str);
		if (str.IsEmpty()) {
			GetDlgItem(IDC_SETTINGS_DNS_SRV_NS)->SetWindowText(_T("8.8.8.8; 8.8.4.4"));
		}
	}
}

void SettingsDlg::OnBnClickedStun()
{
	if (((CButton*)GetDlgItem(IDC_SETTINGS_STUN_CHECKBOX))->GetCheck()) {
		CString str;
		GetDlgItem(IDC_SETTINGS_STUN)->GetWindowText(str);
		if (str.IsEmpty()) {
			GetDlgItem(IDC_SETTINGS_STUN)->SetWindowText(_T("stun.l.google.com:19302"));
		}
	}
}

