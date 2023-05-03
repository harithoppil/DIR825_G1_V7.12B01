/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : crypt.h
 文件描述 : firmware 加密解密模块

 函数列表 :

 修订记录 :
          1 创建 : 邱成伟
            日期 : 2017-09-18
            描述 : 创建文件

**********************************************************************/

#ifndef _CRYPT_H
#define _CRYPT_H

/* 文件名定义 */
#define DEF_RSA_PUBLIC_KEY       "/etc/public.pem" // 默认的公钥文件

#define DEF_RSA_PRIVATE_KEY      "/etc/key.pem" // 默认的私钥文件

#define DEF_KEY_PASSWORD         "12345678" // 秘钥的默认密码

#define ENCRYPT_FIRMWARE_FILE    "/var/firmware.orig"

#define TMP_FIRMWARE_FILE        "/var/firmware.tmp"

int encrypt_firmare(int argc, char *argv[]);
int decrypt_firmare(int argc, char *argv[]);

#endif /* _CRYPT_H */
