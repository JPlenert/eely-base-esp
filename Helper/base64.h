#ifndef Base64_h
#define Base64_h

#ifdef __cplusplus
extern "C" {
#endif

unsigned char * base64_decode(const char *src, size_t len, size_t *out_len);
unsigned char * base64_url_decode(const char *src, size_t len, size_t *out_len);

#ifdef __cplusplus
}
#endif


#endif