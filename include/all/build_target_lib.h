#ifndef SRC_BUILD_TARGET_LIB_H
#define SRC_BUILD_TARGET_LIB_H

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef BYTE
#define BYTE char
#endif

typedef void (CALLBACK* fun_SetMediaVideoStreamSendCallback)(const BYTE* chBuff, int iBufLen);

#endif // SRC_BUILD_TARGET_LIB_H