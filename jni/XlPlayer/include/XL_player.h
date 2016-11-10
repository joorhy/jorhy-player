#ifndef _XL_player_h
#define _XL_player_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef __ANDROID__
#include <jni.h>
extern void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jstring parm);
extern void Java_org_libsdl_app_SDLActivity_nativeStartPlay(JNIEnv* env, jclass cls);
extern void Java_org_libsdl_app_SDLActivity_nativeStopPlay(JNIEnv* env, jclass cls);
extern void Java_org_libsdl_app_SDLActivity_nativeDoubleClick(JNIEnv* env, jclass cls, jfloat x, jfloat y);
#endif

extern void start_play();
extern void stop_play();
extern void changeScreen(float x, float y);
extern void snapshot(const char *file_name);
extern void record(const char *file_name);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /*_XL_player_h*/