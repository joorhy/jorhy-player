#ifndef _XL_android_h
#define _XL_android_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#include <jni.h>

extern void Java_com_jorhy_player_PlayerActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj);
/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /*_XL_android_h*/