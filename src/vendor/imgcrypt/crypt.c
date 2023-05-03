#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "imghead.h"
#include "crypt.h"

static RSA *s_rsa = NULL;
static const char *extdata = "\x98\xc9\xd8\xf0\x13\x3d\x6\x95\xe2\xa7\x9\xc8\xb6\x96\x82\xd4"
                             "\x35\x87\x90\x3\x45\x19\xf8\xc8\x23\x5d\xb6\x49\x28\x39\xa7\x3f"
                             "\xc8\xd3\x2f\x40\x9c\xac\xb3\x47\xc8\xd2\x6f\xdc\xb9\x9\xb\x3c";

int genDigst(const char *content, size_t len, unsigned char *md)
{
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    SHA512_Update(&ctx, content, len);
    SHA512_Final(md, &ctx);
    return 0;
}

int genExtDigst(const char *content, size_t len, const char *key, unsigned char *md)
{
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    SHA512_Update(&ctx, content, len);
    SHA512_Update(&ctx, key, 16);
    SHA512_Final(md, &ctx);
    return 0;
}

int signMD(const char *md, size_t len, unsigned char *sigret, unsigned int *siglen)
{
    *siglen = 0;
    int ret = RSA_sign(NID_sha512, (const unsigned char *)md, len, sigret, siglen, s_rsa);
    return ret;
}

int verifyMD(const unsigned char *md, size_t len, const unsigned char *sigbuf, unsigned int siglen)
{
    int ret = RSA_verify(NID_sha512, md, len, sigbuf, siglen, s_rsa);
    return ret;
}

int encryptData(const char *content, size_t len, const unsigned char *ukey, unsigned char *salt, char *output, size_t *outlen)
{
    AES_KEY key;
    int j;
    char iv[AES_BLOCK_SIZE];
    unsigned int m = len % AES_BLOCK_SIZE;
    len += (AES_BLOCK_SIZE - m) * (!!m);

    AES_set_encrypt_key(ukey, 128, &key);
    for(j = 0; j < AES_BLOCK_SIZE; ++j) {
        iv[j] = rand();
    }

    memcpy(salt, iv, sizeof(iv));

    AES_cbc_encrypt((const unsigned char *)content, (unsigned char *)output, len, &key, (unsigned char *)iv, AES_ENCRYPT);
    *outlen = len;

    return 0;
}

int decryptData(const char *content, size_t len, const unsigned char *ukey, const unsigned char *salt, char *output)
{
    AES_KEY key;
    char iv[AES_BLOCK_SIZE];
    unsigned int m = len % AES_BLOCK_SIZE;
    len += (1 - m) * (!!m);

    AES_set_decrypt_key(ukey, 128, &key);

    memcpy(iv, salt, sizeof(iv));

    AES_cbc_encrypt((const unsigned char *)content, (unsigned char *)output, len, &key, (unsigned char *)iv, AES_DECRYPT);

    return 0;
}

int make_image(const char *sourceFile, const char *destFile, const unsigned char *key)
{
    int fdSource = -1, fdDest = -1;
    int ret = -1;
    const unsigned char *source = NULL;
    unsigned char *dest = NULL;
    size_t sourceLength = -1;
    size_t destLength = -1;

    do {
        struct stat sourceStat = {0};
        unsigned int siglen;

        crypt_img_head_t *ihead = NULL;
        if(stat(sourceFile, &sourceStat) != 0)
            break;

        sourceLength = sourceStat.st_size;

        fdSource = open(sourceFile, O_RDONLY);
        if(fdSource < 0)
            break;
        
        source = mmap(NULL, sourceLength, PROT_READ, MAP_SHARED, fdSource, 0);
        if(NULL == source)
            break;

        fdDest = open(destFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(fdDest < 0)
            break;

        destLength = sourceLength + sizeof(crypt_img_head_t) + AES_BLOCK_SIZE;
        if((destLength - 1) != lseek(fdDest, destLength - 1, SEEK_SET))
            break;

        write(fdDest, "0", 1);

        lseek(fdDest, 0, SEEK_SET);
        close(fdDest);
        fdDest = open(destFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dest = mmap(NULL, destLength, PROT_WRITE | PROT_READ, MAP_SHARED, fdDest, 0);
        if(NULL == dest)
            break;
        ihead = (crypt_img_head_t *)dest;

        memcpy(ihead->m_magic, HEAD_MAGIC, HEAD_MAGIC_LEN);
        
        ihead->m_payload_length_before = htonl(sourceLength);
        genDigst((const char *)source, sourceLength, ihead->m_sha512_md_before);
        genExtDigst((const char *)source, sourceLength, (const char *)key, ihead->m_sha512_md_vendor);

        size_t outlen = -1;
        encryptData((const char *)source, sourceLength, key, ihead->m_salt, (char *)ihead->m_data, &outlen);
        printf("leng before %d post %d\r\n", sourceLength, outlen);
        ihead->m_payload_length_post = htonl((unsigned int)outlen);

        genDigst((const char *)ihead->m_data, outlen, ihead->m_sha512_md_post);

        signMD((const char *)ihead->m_sha512_md_before, SHA512_DIGEST_LENGTH, ihead->m_rsa_sign_before, &siglen);
        signMD((const char *)ihead->m_sha512_md_post, SHA512_DIGEST_LENGTH, ihead->m_rsa_sign_post, &siglen);

        ret = 0;

    } while(0);

    if(source) {
        munmap((void *)source, sourceLength);
    }

    if(dest) {
        munmap((void *)dest, destLength);
    }

    if(fdSource >= 0) {
        close(fdSource);
    }

    if(fdSource >= 0) {
        close(fdSource);
    }

    return ret;
}

int tailor_image(const char *sourceFile, const char *destFile, int targetSize)
{
    int ret = 1;
    int readfd = -1, writefd = -1;
    size_t readLen = -1;
    size_t writeLen = -1;
    size_t totalLen = 0;
    int thd = 1024;
    int totalTimes = 0;
    char buffer[1024];

    if ((readfd = open(sourceFile, O_RDONLY)) < 0)
    {
        return -1;
    }

    if ((writefd = open(destFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
        return -1;
    }

    readLen = lseek(readfd, 0L, SEEK_END);
    lseek(readfd, 0L, SEEK_SET);

    while(ret)
    {
        bzero(buffer, 1024);
        ret = read(readfd, buffer, thd);

        if(ret == -1)
        {
            printf("read Error\n");
            break;
        }

        if (write(writefd, buffer, ret) < 0)
        {
            printf("write Error\n");
            break;
        }

        readLen -= ret;
        totalTimes += 1;
        totalLen += thd;

        if (totalTimes == (targetSize / 1024)) {
            thd = (targetSize % 1024);
        } else {
            thd = 1024;
        }

        if (totalLen >= targetSize)
        {
            ret = 0;
            break;
        }
    }

    close(readfd);
    close(writefd);

    return ret;
}

int verify_image(const char *sourceFile, const char *destFile, const unsigned char *key)
{
    int fdSource = -1, fdDest = -1;
    int ret = -1;
    const unsigned char *source = NULL;
    unsigned char *dest = NULL;
    size_t sourceLength = -1;
    size_t destLength = -1;
    unsigned char digst[64] = {0};
    size_t lengPost = 0;
    size_t lengBefore = 0;

    do {
        struct stat sourceStat = {0};

        crypt_img_head_t *ihead = NULL;
        if(stat(sourceFile, &sourceStat) != 0)
            break;

        sourceLength = sourceStat.st_size;

        fdSource = open(sourceFile, O_RDONLY);
        if(fdSource < 0)
            break;

        source = mmap(NULL, sourceLength, PROT_READ, MAP_SHARED, fdSource, 0);
        if(NULL == source)
            break;

        fdDest = open(TMP_FIRMWARE_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(fdDest < 0)
            break;

        destLength = sourceLength;
        if((destLength - 1) != lseek(fdDest, destLength - 1, SEEK_SET))
            break;

        write(fdDest, "0", 1);

        close(fdDest);

        fdDest = open(TMP_FIRMWARE_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dest = mmap(NULL, destLength, PROT_WRITE | PROT_READ, MAP_SHARED, fdDest, 0);
        if(NULL == dest)
            break;

        ihead = (crypt_img_head_t *)source;
        if(!check_image_magic((unsigned char *)ihead)) {
            printf("no image matic found\r\n");
            break;
        }

        lengPost = htonl(ihead->m_payload_length_post);
        lengBefore = htonl(ihead->m_payload_length_before);

        genDigst((const char *)ihead->m_data, lengPost, digst);
        if(memcmp(digst, ihead->m_sha512_md_post, SHA512_DIGEST_LENGTH) != 0) {
            printf("check sha512 post failed\r\n");
            break;
        }

        decryptData((const char *)ihead->m_data, lengPost, key, ihead->m_salt, (char *)dest);

        genDigst((const char *)dest, lengBefore, digst);
        if(memcmp(digst, ihead->m_sha512_md_before, SHA512_DIGEST_LENGTH) != 0) {
            printf("check sha512 before failed %d %d\r\n", lengBefore, lengPost);
            int j;
            for(j = 0; j < SHA512_DIGEST_LENGTH; j++) {
                printf("%02X", digst[j]);
            }
            printf("\r\n");
            for(j = 0; j < SHA512_DIGEST_LENGTH; j++) {
                printf("%02X", ihead->m_sha512_md_before[j]);
            }
            printf("\r\n");
            break;
        }

        genExtDigst((const char *)dest, lengBefore, (const char *)key, digst);
        if(memcmp(digst, ihead->m_sha512_md_vendor, SHA512_DIGEST_LENGTH) != 0) {
            printf("check sha512 vendor failed\r\n");
            break;
        }

        ret = verifyMD(ihead->m_sha512_md_before, SHA512_DIGEST_LENGTH, ihead->m_rsa_sign_before, 4096/8);
        if(ret != 1) {
            ret = -1;
            break;
        }

        ret = verifyMD(ihead->m_sha512_md_post, SHA512_DIGEST_LENGTH, ihead->m_rsa_sign_post, 4096/8);
        if(ret != 1) {
            ret = -1;
            break;
        }

        ret = 0;

    } while(0);
    
    if(ret != 0)
    {
        return ret;
    }

    if(source) {
        munmap((void *)source, sourceLength);
    }

    if(dest) {
        munmap((void *)dest, destLength);
    }
    
    if(fdSource >= 0) {
        close(fdSource);
    }

    if(fdSource >= 0) {
        close(fdSource);
    }
    ret = tailor_image(TMP_FIRMWARE_FILE, destFile, lengBefore);

    if(ret == 0)
    {
        remove(TMP_FIRMWARE_FILE);
    }
    return ret;
	
}

static void export_pub_key()
{
    PEM_write_RSAPublicKey(stdout, s_rsa);
    FILE *fp = fopen("pubkey.h", "w+");
    if(fp == NULL)
        return;

    fprintf(fp, "static const char *pubkey_n = \"%s\";\n", BN_bn2hex(s_rsa->n));
    fprintf(fp, "static const char *pubkey_e = \"%s\";\n", BN_bn2hex(s_rsa->e));
    fprintf(fp, "\n");
    fclose(fp);
}

static int init_rsa_public_key(const char *keyFile, const char *keyPassword)
{
    OpenSSL_add_all_algorithms();
    FILE *fpKey = fopen(keyFile, "r");

    if(NULL != fpKey) {
        RSA *rsa = RSA_new();
    
        if(NULL == PEM_read_RSAPublicKey(fpKey, &rsa, NULL, (void *)keyPassword)) {
            RSA_free(rsa);
            printf("Read RSA private key failed, maybe the key password is incorrect\r\n");
        } else {
            s_rsa = rsa;
        }

        fclose(fpKey);
    }

    if(s_rsa) {
        return 0;
    }

    return -1;
}


static int init_rsa_key(const char *keyFile, const char *keyPassword)
{
    OpenSSL_add_all_algorithms();
    FILE *fpKey = fopen(keyFile, "r");

    if(NULL != fpKey) {
        RSA *rsa = RSA_new();

        if(NULL == PEM_read_RSAPrivateKey(fpKey, &rsa, NULL, (void *)keyPassword)) {
            RSA_free(rsa);
            printf("Read RSA private key failed, maybe the key password is incorrect\r\n");
        } else {
            s_rsa = rsa;
        }

        fclose(fpKey);
    } else {
        s_rsa = RSA_generate_key(4096, 65537, NULL, NULL);

        FILE *fpKey = fopen(keyFile, "w+");

        if(fpKey != NULL) {
            PEM_write_RSAPrivateKey(fpKey, s_rsa, EVP_aes_256_cbc(), (unsigned char *)keyPassword, strlen(keyPassword), NULL, NULL);
            fclose(fpKey);
        }
        export_pub_key();
    }

    if(s_rsa) {
        return 0;
    }

    return -1;
}

static int init_extdata(char *data)
{
    decryptData(&extdata[32], 16, (const unsigned char *)&extdata[16], (const unsigned char *)&extdata[0], data);
    return 0;
}

int decrypt_firmare(int argc, char *argv[])
{
    char key[16] = "0123456789ABCDEF";
    const char *pubkey_file = DEF_RSA_PUBLIC_KEY;
    int ret = -1;

    if(argc < 2) {
        printf("%s <sourceFile>\r\n", argv[0]);
        return -1;
    }

    if (argc > 2) {
        pubkey_file = argv[2];
    }

    ret = init_rsa_public_key(pubkey_file, NULL);
    if(ret != 0)
        return -1;

    init_extdata(key);
    printf("key:");
    for(ret = 0; ret < 16; ++ret) {
        printf("%02X", (unsigned char)key[ret]);
    }
    printf("\r\n");

    ret = verify_image(argv[1], ENCRYPT_FIRMWARE_FILE, (unsigned char *)key);
    if(ret == 0) {
        unlink(argv[1]);
        rename(ENCRYPT_FIRMWARE_FILE, argv[1]);
    }

    RSA_free(s_rsa);
    return ret;
}

int encrypt_firmare(int argc, char *argv[])
{
    const char *keyFile = DEF_RSA_PRIVATE_KEY;
    const char *keyPassword = DEF_KEY_PASSWORD;
    char key[16] = DEF_KEY_PASSWORD;
    int ret = -1;

    if(argc > 3) {
        keyFile = argv[3];
    }
    if(argc > 4) {
        keyPassword = argv[4];
    }

    ret = init_rsa_key(keyFile, keyPassword);
    if(ret != 0)
        return -1;

    if(argc < 3) {
        printf("%s sourceFile destFile", argv[0]);
        return -1;
    }

    init_extdata(key);

    ret = make_image(argv[1], argv[2], (unsigned char *)key);

    RSA_free(s_rsa);
    return ret;
}

