#ifndef __DEBUGOUT_H__
#define __DEBUGOUT_H__

#ifdef __ANDROID__
#include <android/log.h>

static int my_fprintf(FILE *stream, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, "ONS_Out", format, ap);
    va_end(ap);
    return 0;
}  
static int my_printf(const char *format, ...){
    va_list ap;
    va_start(ap, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, "ONS_Out", format, ap);
    va_end(ap);
    return 0;
}   
#ifdef fprintf
#undef fprintf
#endif
#define fprintf(fp,...) my_fprintf(fp, __VA_ARGS__)
#ifdef printf
#undef printf
#endif
#define printf(...) my_printf( __VA_ARGS__)

#endif /*__ANDROID__*/
#endif
