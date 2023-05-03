/**
 * \file hmac_md5.c
 * \brief Implementation of HMAC: Keyed-Hashing for Message Authentication
 *
 * Code taken from RFC2104.
 */


#include <string.h>

#include "hmac_md5.h"



//

#include <time.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h> 

#define BUFF_SIZE1 256
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
#define MPR_ERR_BASE (-200)

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// F, G, H and I are basic MD5 functions.

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.

#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

#define CRYPT_HASH_SIZE   16

////////////////////////////////////////////////////////////////////////////////
// MD5 algorithms
////////////////////////////////////////////////////////////////////////////////

typedef struct {
	unsigned long state[4];
	unsigned long count[2];
	unsigned char buffer[64];
} MD5_CONTEXT;



////////////////////////////  Declarations /////////////////////////////////////
static void MD5Transform(unsigned long [4], unsigned char [64]);
static void Encode(unsigned char *, unsigned long *, unsigned int);
static void Decode(unsigned long *, unsigned char *, unsigned int);
static void MD5_memcpy(unsigned char *, unsigned char *, unsigned int);
static void MD5_memset(unsigned char *, int, unsigned int);
//////////////////////////////////// Code //////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//	MD5 basic transformation. Transforms state based on block.
//

static void MD5Transform(unsigned long state[4], unsigned char block[64])
{
	unsigned long a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	// Round 1
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); // 1
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); // 2
	FF (c, d, a, b, x[ 2], S13, 0x242070db); // 3
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); // 4
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); // 5
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); // 6
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); // 7
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); // 8
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); // 9
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); // 10
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); // 11
	FF (b, c, d, a, x[11], S14, 0x895cd7be); // 12
	FF (a, b, c, d, x[12], S11, 0x6b901122); // 13
	FF (d, a, b, c, x[13], S12, 0xfd987193); // 14
	FF (c, d, a, b, x[14], S13, 0xa679438e); // 15
	FF (b, c, d, a, x[15], S14, 0x49b40821); // 16

	// Round 2
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); // 17
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); // 18
	GG (c, d, a, b, x[11], S23, 0x265e5a51); // 19
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); // 20
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); // 21
	GG (d, a, b, c, x[10], S22,  0x2441453); // 22
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); // 23
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); // 24
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); // 25
	GG (d, a, b, c, x[14], S22, 0xc33707d6); // 26
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); // 27
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); // 28
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); // 29
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); // 30
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); // 31
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32

	// Round 3
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); // 33
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); // 34
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); // 35
	HH (b, c, d, a, x[14], S34, 0xfde5380c); // 36
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); // 37
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); // 38
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); // 39
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); // 40
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); // 41
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); // 42
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); // 43
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); // 44
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); // 45
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); // 46
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); // 47
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); // 48

	// Round 4
	II (a, b, c, d, x[ 0], S41, 0xf4292244); // 49
	II (d, a, b, c, x[ 7], S42, 0x432aff97); // 50
	II (c, d, a, b, x[14], S43, 0xab9423a7); // 51
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); // 52
	II (a, b, c, d, x[12], S41, 0x655b59c3); // 53
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); // 54
	II (c, d, a, b, x[10], S43, 0xffeff47d); // 55
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); // 56
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); // 57
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58
	II (c, d, a, b, x[ 6], S43, 0xa3014314); // 59
	II (b, c, d, a, x[13], S44, 0x4e0811a1); // 60
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); // 61
	II (d, a, b, c, x[11], S42, 0xbd3af235); // 62
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); // 63
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); // 64

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// Zeroize sensitive information.
	MD5_memset ((unsigned char*) x, 0, sizeof (x));
}

//
//	Encodes input (ulong) into output (uchar). Assumes len is a multiple of 4.
//

static void Encode(unsigned char *output, unsigned long *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char) (input[i] & 0xff);
		output[j+1] = (unsigned char) ((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char) ((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char) ((input[i] >> 24) & 0xff);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
//	Decodes input (uchar) into output (ulong). Assumes len is a multiple of 4.
//

static void Decode(unsigned long *output, unsigned char *input, unsigned int len)
{
	unsigned int 	i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((unsigned long) input[j]) | (((unsigned long) input[j+1]) << 8) |
			(((unsigned long) input[j+2]) << 16) | (((unsigned long) input[j+3]) << 24);
}

////////////////////////////////////////////////////////////////////////////////
//
//	FUTURE: Replace "for loop" with standard memcpy if possible.
//

static void MD5_memcpy(unsigned char *output, unsigned char *input, unsigned int len)
{
	unsigned int 	i;

	for (i = 0; i < len; i++)
		output[i] = input[i];
}

////////////////////////////////////////////////////////////////////////////////
//
// FUTURE: Replace "for loop" with standard memset if possible.
//
static void MD5_memset(unsigned char *output, int value, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((char*) output)[i] = (char) value;
}



//////////////////////////////////// PUBLIC //////////////////////////////////////
void MD5_Init(MD5_CONTEXT *);
void MD5_Final(unsigned char [], MD5_CONTEXT *);
void MD5_Update(MD5_CONTEXT *, unsigned char *,unsigned int);
char *MD5_String(char *string);
int CalcNonce(char **nonce);
int CalcDigest(char *,char *,char *,char *,char *,char *,char *,char *,char *,char **);
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//	MD5 initialization. Begins an MD5 operation, writing a new context.
//
void MD5_Init(MD5_CONTEXT *context)
{
	context->count[0] = context->count[1] = 0;

	//
	// Load constants
	//
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

////////////////////////////////////////////////////////////////////////////////
//
//	MD5 block update operation. Continues an MD5 message-digest operation,
//	processing another message block, and updating the context.
//
void MD5_Update(MD5_CONTEXT *context, unsigned char *input, unsigned int inputLen)
{
	unsigned int 	i, index, partLen;

	index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

	if ((context->count[0] += ((unsigned long)inputLen << 3)) < ((unsigned long)inputLen << 3)){
		context->count[1]++;
	}
	context->count[1] += ((unsigned long)inputLen >> 29);
	partLen = 64 - index;

	if (inputLen >= partLen) {
		MD5_memcpy((unsigned char*) &context->buffer[index], (unsigned char*) input, partLen);
		MD5Transform(context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64) {
			MD5Transform (context->state, &input[i]);
		}
		index = 0;
	} else {
		i = 0;
	}

	MD5_memcpy((unsigned char*) &context->buffer[index], (unsigned char*) &input[i],
		inputLen-i);
}

////////////////////////////////////////////////////////////////////////////////
//
//	MD5 finalization. Ends an MD5 message-digest operation, writing the message
//	digest and zeroizing the context.
//
void MD5_Final(unsigned char digest[16], MD5_CONTEXT *context)
{
	unsigned char 	bits[8];
	unsigned int	index, padLen;

	// Save number of bits
	Encode(bits, context->count, 8);

	// Pad out to 56 mod 64.
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5_Update(context, PADDING, padLen);

	// Append length (before padding)
	MD5_Update(context, bits, 8);
	// Store state in digest
	Encode(digest, context->state, 16);

	// Zeroize sensitive information.
	MD5_memset((unsigned char*)context, 0, sizeof (*context));
}

char *MD5_Binary(unsigned char *buf, int length)
{
    const char		*hex = "0123456789abcdef";
    MD5_CONTEXT		md5ctx;
    unsigned char   hash[CRYPT_HASH_SIZE];
    char			*r, *str;
	char			result[(CRYPT_HASH_SIZE * 2) + 1];
    int				i;

	//
	//	Take the MD5 hash of the string argument.
	//
    MD5_Init(&md5ctx);
    MD5_Update(&md5ctx, buf, (unsigned int) length);
    MD5_Final(hash, &md5ctx);

    for (i = 0, r = result; i < 16; i++) {
		*r++ = hex[hash[i] >> 4];
		*r++ = hex[hash[i] & 0xF];
    }
    *r = '\0';

	str = (char*) malloc(sizeof(result));
	if (str != NULL)
		strcpy(str, result);
	else
		return NULL;

    return str;
}

char *MD5_String(char *string)
{
	return MD5_Binary((unsigned char*)string, strlen(string));
}

////////////////////////////////////////////////////////////////////////////////
//
//	Get a Nonce value for passing along to the client.  This function composes
//	the string "secret:eTag:time:realm" and calculates the MD5 digest.
//
int CalcNonce(char **nonce)
{
	time_t		now;
	char		*nonceBuf;
	if((nonceBuf=(char *)malloc(BUFF_SIZE1))==NULL)
	{
		syslog(LOG_WARNING,"unable to allocate memory exiting\n");
//		ssl_cleanup();
		exit(1);
	}

	time(&now);
	//sprintf(nonceBuf, sizeof(nonceBuf), "%s:%s:%x:%s", randomvalue, etag, now,pid);
	srandom((unsigned int)now);
	sprintf(nonceBuf,"%ld:%s:%x:%d",random(), "", (unsigned int)now, getpid());
	syslog(LOG_WARNING,"XXX\tNonceBuf:%s\n",nonceBuf);
    *nonce = MD5_String(nonceBuf);
	syslog(LOG_WARNING,"XXX\tNonce:%s\n",*nonce);
	free(nonceBuf);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Get a Digest value using the MD5 algorithm -- See RFC 2617 to understand
//	this code.
//
int CalcDigest(char *userName, char *password, char *realm, char *uri,
		char *nonce, char *qop, char *nc, char *cnonce, char *method,
		char **digest)
{
	char	*a1Buf,*a2Buf,*digestBuf;
	char	*ha1, *ha2;


	if((a1Buf=(char *)malloc(BUFF_SIZE1*sizeof(char)))==NULL)
	{
		syslog(LOG_WARNING,"unable to allocate memory exiting\n");
		exit(1);
	}
	if((a2Buf=(char *)malloc(BUFF_SIZE1*sizeof(char)))==NULL)
	{
		syslog(LOG_WARNING,"unable to allocate memory exiting\n");
		free(a1Buf);
		exit(1);
	}
	if((digestBuf=(char *)malloc(BUFF_SIZE1*sizeof(char)))==NULL)
	{
		syslog(LOG_WARNING,"unable to allocate memory exiting\n");
		free(a1Buf);
		free(a2Buf);
		exit(1);
	}

	if(!qop)
	{
		syslog(LOG_NOTICE,"MD5 Error Exiting");
	}
	//
	//	Compute HA1. If userName == 0, then the password is already expected
	//	to be in the HA1 format (MD5(userName:realm:password). This is the
	//	format that httpPassword stores all passwords, so we are "sweet".
	//
	if (userName == 0) {
		ha1 = strdup(password);
	} else {
		//sprintf(a1Buf, sizeof(a1Buf), "%s:%s:%s", userName, realm, password);
		sprintf(a1Buf,"%s:%s:%s", userName, realm, password);

		ha1 = MD5_String(a1Buf);
	}

	//
	//	HA2
	//
	//sprintf(a2Buf, sizeof(a2Buf), "%s:%s", method, uri);
	sprintf(a2Buf,"%s:%s", method, uri);
	ha2 = MD5_String(a2Buf);

	//
	//	H(HA1:nonce:HA2)
	//
	if (strcmp(qop, "auth") == 0) {
	//sprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s:%s:%s:%s", ha1,nonce, nc, cnonce, qop, ha2);
		sprintf(digestBuf,"%s:%s:%s:%s:%s:%s", ha1,nonce, nc, cnonce, qop, ha2);
	} else if (strcmp(qop, "auth-int") == 0) {
		//sprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s:%s:%s:%s", ha1,nonce, nc, cnonce, qop, ha2);
		sprintf(digestBuf,"%s:%s:%s:%s:%s:%s", ha1,nonce, nc, cnonce, qop, ha2);
	} else {
		//sprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s", ha1, nonce, ha2);
		sprintf(digestBuf,"%s:%s:%s", ha1, nonce, ha2);
	}

	*digest = MD5_String(digestBuf);

	free(ha1);
	free(ha2);
	free(a1Buf);
	free(a2Buf);
	free(digestBuf);
	return 0;
}



/******************************************************************************
 *       BASE64 encode/decode
 ******************************************************************************/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
static inline void ILibencodeblock( unsigned char in[3], unsigned char out[4], int len )
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/* Decode 4 '6-bit' characters into 3 8-bit binary bytes */
static inline void ILibdecodeblock( unsigned char in[4], unsigned char out[3] )
{
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}


/*! \fn ILibBase64Encode(unsigned char* input, const int inputlen)
	\brief Base64 encode a stream adding padding and line breaks as per spec.
	\para
	\b Note: The encoded stream must be freed
	\param input The stream to encode
	\param inputlen The length of \a input
	\returns output The encoded stream
*/
char *Base64_Encode(const char* input, const int inputlen)
{
	unsigned char* out;
	unsigned char* in;
    unsigned char* inptr;
    char *result;

	if (input == NULL || inputlen == 0)
	{
		return NULL;
	}

    result = malloc(((inputlen * 4) / 3) + 5);
	out = (unsigned char*)result;
	in  = (unsigned char*)input;
	inptr  = (unsigned char*)input;

	while ((in+3) <= (inptr + inputlen))
	{
		ILibencodeblock(in, out, 3);
		in += 3;
		out += 4;
	}
    
	if ((inptr + inputlen)-in == 1)
	{
		ILibencodeblock(in, out, 1);
		out += 4;
	}
	else if ((inptr + inputlen)-in == 2)
	{
		ILibencodeblock(in, out, 2);
		out += 4;
	}
	*out = 0;

	return result;
}

/*! \fn Base64_Decode(unsigned char* input, const int inputlen)
	\brief Decode a base64 encoded stream discarding padding, line breaks and noise
	\para
	\b Note: The decoded stream must be freed
	\param input The stream to decode
	\param inputlen The length of \a input
	\returns output The decoded stream
*/
char *Base64_Decode(const char* input, const int inputlen)
{
    unsigned char* out;
	unsigned char* in;
    unsigned char* inptr;
    char *result;
	unsigned char v;
	unsigned char inarr[4];
	int i, len;

	if (input == NULL || inputlen == 0)
	{
		return NULL;
	}

	result = (unsigned char*)malloc(((inputlen * 3) / 4) + 4);
    out = (unsigned char*)result;
	in  = (unsigned char*)input;
	inptr  = (unsigned char*)input;

	while( in <= (inptr+inputlen) )
	{
		for( len = 0, i = 0; i < 4 && in <= (inptr+inputlen); i++ )
		{
			v = 0;
			while( in <= (inptr+inputlen) && v == 0 ) {
				v = (unsigned char) *in;
				in++;
				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}
			if( in <= (inptr+inputlen) ) {
				len++;
				if( v ) {
					inarr[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				inarr[i] = 0;
			}
		}
		if( len )
		{
			ILibdecodeblock( inarr, out );
			out += len-1;
		}
	}
	*out = 0;
    
	return result;
}



//

/**
 * \brief Keyed-Hashing for Message Authentication
 *
 * \param [in]	text		pointer to data stream
 * \param [in]	text_len	length of data stream
 * \param [in]	key		pointer to authentication key
 * \param [in]	key_len		length of authentication key
 * \param [out]	digest		caller digest to be filled in
 */
void hmac_md5(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest)
{
	MD5_CONTEXT context;
	unsigned char k_ipad[65];    /* inner padding key XORd with ipad */
	unsigned char k_opad[65];    /* outer padding key XORd with opad */
	unsigned char tk[16];
	int i;

	/* if key is longer than 64 bytes reset it to key=MD5(key) */
	if (key_len > 64)
	{
		MD5_CONTEXT tctx;
		MD5_Init(&tctx);
		MD5_Update(&tctx, key, key_len);
		MD5_Final(tk, &tctx);

		key = tk;
                key_len = 16;
        }

	/*
	 * the HMAC_MD5 transform looks like:
	 *
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
         *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */

	/* start out by storing key in pads */
	memset(k_ipad,0,sizeof(k_ipad));
	memcpy(k_ipad,key,key_len);
	memset(k_opad,0,sizeof(k_opad));
	memcpy(k_opad,key,key_len);

	/* XOR key with ipad and opad values */
	for (i=0; i<64; i++)
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/*
	 * perform inner MD5
	 */
	MD5_Init(&context);			/* init context for 1st pass */
	MD5_Update(&context, k_ipad, 64);	/* start with inner pad */
	MD5_Update(&context, text, text_len);	/* then text of datagram */
	MD5_Final(digest, &context);		/* finish up 1st pass */

	/*
	 * perform outer MD5
	 */
	MD5_Init(&context);			/* init context for 2nd pass */
	MD5_Update(&context, k_opad, 64);	/* start with outer pad */
	MD5_Update(&context, digest, 16);	/* then results of 1st hash */
	MD5_Final(digest, &context);		/* finish up 2nd pass */
}
