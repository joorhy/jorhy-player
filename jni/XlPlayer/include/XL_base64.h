#ifndef __XL_base64_h
#define __XL_base64_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

void base64_in (unsigned char *buf, char *obuf, int len);
void base64_out (char *buf, unsigned char *obuf, int len);

#ifdef __cplusplus
}
#endif

#endif //~__XL_base64_h
