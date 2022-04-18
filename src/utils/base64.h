#ifndef __utils_base64_h__
#define __utils_base64_h__

#include <stddef.h>

#ifdef __cplusplus
extern {
#endif

  void b64_encode_s(const unsigned char *in_data, size_t in_len,
                    char **out_str);

  void b64_encode_b(const unsigned char *in_data, size_t in_len,
                    unsigned char **out_data, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
