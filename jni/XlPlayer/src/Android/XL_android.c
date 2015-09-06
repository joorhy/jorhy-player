#include "XL_android.h"

#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/

/* Library init */
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return JNI_VERSION_1_4;
}