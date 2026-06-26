# Third-party licenses

NanoSIP (a fork of MicroSIP) is GPL-2.0-or-later — see [`LICENSE`](LICENSE). It bundles the
following third-party components, each under its own original license. None of these are
modified by this fork beyond standard build configuration (see `CLAUDE.md` for details).

| Component | Location | License | License text |
|---|---|---|---|
| PJSIP / pjproject | `NanoSIP/pjproject/` | GPL-2.0-or-later | same as this repo's `LICENSE` |
| Speex | `NanoSIP/pjproject/third_party/speex/` | BSD-style | `pjproject/third_party/speex/COPYING` |
| libresample | `NanoSIP/pjproject/third_party/resample/` | LGPL-2.1 | `pjproject/third_party/resample/COPYING` |
| libsrtp | `NanoSIP/pjproject/third_party/srtp/` | BSD-style | `pjproject/third_party/srtp/LICENSE` |
| WebRTC (subset: AEC/AGC/NS modules) | `NanoSIP/pjproject/third_party/webrtc/` | BSD-style | `pjproject/third_party/webrtc/LICENSE` |
| JsonCpp | `NanoSIP/lib/jsoncpp/` | MIT | see https://github.com/open-source-parsers/jsoncpp |

Notes:
- This is a voice-only build (`_GLOBAL_VIDEO` disabled, see `CLAUDE.md`), so video-related
  third-party codecs that ship with stock pjproject (e.g. VPX, FFmpeg) are not linked into
  the produced `NanoSIP.exe`, even though their source trees may still be present under
  `pjproject/third_party/` as part of the upstream pjproject tree.
- Opus is intentionally not linked (`opus.lib` removed from the project's
  `AdditionalDependencies`), so its license does not apply to the built binary.
- For exact license text of each component, open the file referenced in the "License text"
  column inside this repository — they are not duplicated here to avoid drift from upstream.
