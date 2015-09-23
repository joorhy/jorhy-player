#ifndef _XL_log_h
#define _XL_log_h

#ifdef __ANDROID__
#include <android/log.h>
const static char *LOG_TAG = "XL_LOG";
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__) 
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define LOGI(...)  printf(__VA_ARGS__) 
#define LOGE(...)  printf(__VA_ARGS__)
#endif

#endif