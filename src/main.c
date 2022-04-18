#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/hmac.h>

#include "utils/base64.h"

#define _MY_TMP_BUF_SZ (4096)

#define TRTC_SECRET_KEY                                                        \
  "7697bcea581c170101ef07752e4cdb95b457da860d57acba8094b84880a9f71b"

#define TRTC_APP_ID (1400662786)

/// 参考腾讯云 TRTC UserSig 相关：
/// https://cloud.tencent.com/document/product/647/17275#.E6.AD.A3.E5.BC.8F.E8.BF.90.E8.A1.8C.E9.98.B6.E6.AE.B5.E5.A6.82.E4.BD.95.E8.AE.A1.E7.AE.97-usersig.EF.BC.9F

/// UserSig 计算公式，其中 secretkey 为计算 usersig 用的加密密钥
///
/// usersig = hmacsha256(secretkey, (userid + sdkappid + currtime + expire +
/// base64(userid + sdkappid + currtime + expire)))

int main(int argc, char *argv[]) {
  char user_id[] = "user01";
  long currtime = time(NULL);
  long expire = currtime + 3600; // 一个小时过期

  // base64(userid + sdkappid + currtime + expire) 部分:
  char b64str[_MY_TMP_BUF_SZ];
  memset(b64str, 0, _MY_TMP_BUF_SZ);
  {
    char in_data[_MY_TMP_BUF_SZ];
    memset(in_data, 0, _MY_TMP_BUF_SZ);
    char *out_str;
    snprintf(in_data, _MY_TMP_BUF_SZ, "%s%d%ld%ld", user_id, TRTC_APP_ID,
             currtime, expire);
    b64_encode_s(in_data, strlen(in_data), &out_str);
    strncpy(b64str, out_str, _MY_TMP_BUF_SZ - 1);
  }
  printf("Base64: %s\n", b64str);
  // (userid + sdkappid + currtime + expire + base64部分
  char sig_data[_MY_TMP_BUF_SZ];
  {
    memset(sig_data, 0, _MY_TMP_BUF_SZ);
    snprintf(sig_data, _MY_TMP_BUF_SZ, "%s%d%ld%ld%s", user_id, TRTC_APP_ID,
             currtime, expire, b64str);
  }
  printf("Data to sig: %s\n", sig_data);
  // 进行 hmacsha256
  unsigned int md_len;
  char *md = (char *)calloc(EVP_MAX_MD_SIZE, sizeof(char));
  char secret_key[] = TRTC_SECRET_KEY;
  HMAC(EVP_sha256(), secret_key, strlen(secret_key), sig_data, strlen(sig_data),
       md, &md_len);
  printf("(%u)%s\n", md_len, md);

  // char hexstr[md_len * 2];
  // for (unsigned i = 0; i < md_len; ++i) {
  //   char* pc = &hexstr[i*2];
  //   sprintf(pc, "%02x", md[i]);
  // }
  // printf("%s\n", hexstr);

  // 最终的 HMAC 结果需要 base64 encode
  char* final_result;
  b64_encode_s(md, md_len, &final_result);
  free(final_result);

  return 0;
}