#include "base64.h"

#include <memory.h>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

/// 参考: https://www.openssl.org/docs/man1.0.2/man3/BIO_f_base64.html 以及
/// https://gist.github.com/barrysteyn/7308212

void b64_encode_s(const unsigned char *in_data, size_t in_len, char **out_str) {
  BIO_s_mem();
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *bio = BIO_new(BIO_s_mem());
  BIO_set_close(bio, BIO_NOCLOSE);
  BIO_push(b64, bio);
  BIO_write(b64, in_data, in_len);
  BIO_flush(b64);
  BUF_MEM *pbuf;
  BIO_get_mem_ptr(bio, &(pbuf));
  *out_str = (char *)calloc(pbuf->length + 1, sizeof(char));
  memcpy(*out_str, pbuf->data, pbuf->length);
  BIO_free_all(bio);
}

void b64_encode_b(const unsigned char *in_data, size_t in_len,
                  unsigned char **out_data, size_t *out_len) {
  BIO_s_mem();
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *bio = BIO_new(BIO_s_mem());
  BIO_push(b64, bio);
  BIO_write(b64, in_data, in_len);
  BIO_flush(b64);
  BIO_set_close(bio, BIO_NOCLOSE);
  BUF_MEM *pbuf;
  BIO_get_mem_ptr(bio, &(pbuf));
  *out_data = (unsigned char *)calloc(pbuf->length, sizeof(unsigned char));
  memcpy(*out_data, pbuf->data, pbuf->length);
  if (out_len)
    *out_len = pbuf->length;
  BIO_free_all(bio);
}