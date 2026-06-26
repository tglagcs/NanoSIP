
#define PJSUA_HAS_VIDEO 0
#define PJSIP_HAS_TLS_TRANSPORT 0
#define PJ_HAS_SSL_SOCK 0

// Only G.711 (PCMA/PCMU, always built into pjmedia core) is used -
// strip every other audio codec out of the build.
#define PJMEDIA_HAS_GSM_CODEC 0
#define PJMEDIA_HAS_SPEEX_CODEC 0
#define PJMEDIA_HAS_ILBC_CODEC 0
#define PJMEDIA_HAS_G722_CODEC 0
#define PJMEDIA_HAS_L16_CODEC 0

