/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : imghead.h
 文件描述 : 

 函数列表 :

 修订记录 :
          1 创建 : 邱成伟
            日期 : 2017-09-18
            描述 : 创建文件

**********************************************************************/

#ifndef _IMGHEAD_H
#define _IMGHEAD_H

#ifndef HEAD_MAGIC
#define HEAD_MAGIC "SHRS"
#endif

#ifndef HEAD_MAGIC_LEN
#define HEAD_MAGIC_LEN 4
#endif

#ifndef RSA_KEY_BITS
#define RSA_KEY_BITS 4096
#endif

#ifndef SHA512_DIGEST_LENGTH
#define SHA512_DIGEST_LENGTH 64
#endif

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif

typedef struct _crypt_img_head{
    unsigned char m_magic[HEAD_MAGIC_LEN];
    uint32_t m_payload_length_before;//be
    uint32_t m_payload_length_post;//be
    unsigned char m_salt[16];
    unsigned char m_sha512_md_vendor[SHA512_DIGEST_LENGTH];
    unsigned char m_sha512_md_before[SHA512_DIGEST_LENGTH];
    unsigned char m_sha512_md_post[SHA512_DIGEST_LENGTH];
    unsigned char m_rsa_pub[4096/8];
    unsigned char m_rsa_sign_before[4096/8];
    unsigned char m_rsa_sign_post[4096/8];
    unsigned char m_data[0];
}__attribute__((packed)) crypt_img_head_t;

/*
 * check image magic
 * /param data, input the image data,include header
 * /return value, 0, false, 1, true
 */
static inline int check_image_magic(unsigned char *data)
{
    return (memcmp(data, HEAD_MAGIC, HEAD_MAGIC_LEN) == 0);
}

#endif /* _IMGHEAD_H */



