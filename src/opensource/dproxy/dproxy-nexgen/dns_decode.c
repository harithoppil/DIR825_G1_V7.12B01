
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "dns_decode.h"

/*
 * The following macros set num to the host version of the number pointed to 
 * by buf and increment the buff (usually a pointer) and count (usually and 
 * int counter)
 */
#define SET_UINT16( num, buff) num = htons(*(uint16*)*buff); *buff += 2
#define SET_UINT32( num, buff) num = htonl(*(uint32*)*buff); *buff += 4
/*****************************************************************************/
/* reverse lookups encode the IP in reverse, so we need to turn it around
 * example 2.0.168.192.in-addr.arpa => 192.168.0.2
 * this function only returns the first four numbers in the IP
 *  NOTE: it assumes that name points to a buffer BUF_SIZE long
 */
void dns_decode_reverse_name(char *name)
{
   char *temp;//临时变量
  char *octet[4];//存放分割的几个字符串

  int i;
  temp = 0;//临时变量

  //break the supplied string into tokens on the '.' chars
  octet[0] = strtok( name, "." );//存放第一个字符串，也就是IP第四个字符串
  octet[1] = strtok( NULL, "." );//存放第二个字符串，也就是IP第三个字符串
  octet[2] = strtok( NULL, "." );//存放第三个字符串，也就是IP第二个字符串
  octet[3] = strtok( NULL, "." );//存放第四个字符串，也就是IP第一个字符串
  //reconstuct the tokens in reverse order, being carful to check for NULLs
  for( i = 3 ; i >= 0 ; i--)//循环计算得到IP
  {
    if( octet[i] != 0 )//判断是否为空
    {
      if( temp == 0 )//第一个IP字符串
      {
        temp = octet[i];//返回给name
      }
      else
      {
        temp = strcat( temp, octet[i] );//把后面的字符串连接起来成一个IP字符串形式
      }
      
      if( i != 0 ) temp = strcat( temp, "." );//添加IP字符串里面的"."
    }
  }
  strcpy( name, temp );
}
/*****************************************************************************/
/* Queries are encoded such that there is and integer specifying how long 
 * the next block of text will be before the actuall text. For eaxmple:
 *             www.linux.com => \03www\05linux\03com\0
 * This function assumes that buf points to an encoded name.
 * The name is decoded into name. Name should be at least 255 bytes long.
 */
void dns_decode_name(char *name, char **buf)
{
  int i, k, len, j;

  i = k = 0;
  while( **buf ){
         len = *(*buf)++;
         for( j = 0; j<len ; j++)
	      name[k++] = *(*buf)++;
         name[k++] = '.';
  }
  name[k-1] = *(*buf)++;
}
/*****************************************************************************/
/* Decodes the raw packet in buf to create a rr. Assumes buf points to the 
 * start of a rr. 
 * Note: Question rrs dont have rdatalen or rdata. Set is_question when
 * decoding question rrs, else clear is_question
 */
void dns_decode_rr(struct dns_rr *rr, char **buf, int is_question,char *header)
{
  /* if the first two bits the of the name are set, then the message has been
     compressed and so the next byte is an offset from the start of the message
     pointing to the start of the name */
  if( **buf & 0xC0 ){
    (*buf)++;
    header += *(*buf)++;
    dns_decode_name( rr->name, &header );
  }else{
    /* ordinary decode name */
    dns_decode_name( rr->name, buf );
  }  

  SET_UINT16( rr->type, buf );
  SET_UINT16( rr->class, buf);

  if( is_question != 1 ){
    SET_UINT32( rr->ttl, buf );
    SET_UINT16( rr->rdatalen, buf );
//fix for dproxy exit use coredump is memcpy err,may be memcpy dest size not enough,so need to check
		if(rr->rdatalen > NAME_SIZE)
		{
			printf("%s(%d)rr->rdatalen=%d\n", __FUNCTION__, __LINE__, rr->rdatalen);
			return;
		}
		
        memcpy( rr->data, *buf, rr->rdatalen );
    *buf += rr->rdatalen;
    /*
    for(i = 0; i < rr->rdatalen; i+=4 )
      SET_UINT32( (uint32)rr->data[i], buf );
    */
  }

  if( rr->type == PTR ){ /* reverse lookup */
    dns_decode_reverse_name( rr->name );
  }

}
/*****************************************************************************/
/* A raw packet pointed to by buf is decoded in the assumed to be alloced 
 * dns_message structure.
 * RETURNS: 0
 */
int dns_decode_message(struct dns_message *m, char **buf)
{
  int i;
  char *header_start = *buf;

  SET_UINT16( m->header.id, buf );
  SET_UINT16( m->header.flags.flags, buf );
  SET_UINT16( m->header.qdcount, buf );
  SET_UINT16( m->header.ancount, buf );
  SET_UINT16( m->header.nscount, buf );
  SET_UINT16( m->header.arcount, buf );

  if( m->header.ancount > 1 ){
	/*
    printf("Lotsa answers\n");
	*/
  }

  /* decode all the question rrs */
  for( i = 0; i < m->header.qdcount && i < NUM_RRS; i++){
    dns_decode_rr( &m->question[i], buf, 1, header_start );
  }  
  /* decode all the answer rrs */
  for( i = 0; i < m->header.ancount && i < NUM_RRS; i++){
    dns_decode_rr( &m->answer[i], buf, 0, header_start );
  }  

  return 0;
}
/*****************************************************************************/
void dns_decode_request(dns_request_t *m)
{
  struct in_addr *addr;
  char *ptr;
  int i;

  m->here = m->original_buf;

  dns_decode_message( &m->message, &m->here );
  
  if( m->message.question[0].type == PTR ) {
    strncpy( m->ip, m->message.question[0].name, sizeof(m->ip)-1);
  } else if ( m->message.question[0].type == A || 
	          m->message.question[0].type == AAAA) { 
    strncpy( m->cname, m->message.question[0].name, sizeof(m->cname)-1 );
  }

  /* set according to the answer */
  for( i = 0; i < m->message.header.ancount && i < NUM_RRS; i++) {
     m->ttl = m->message.answer[i].ttl;
    /* make sure we ge the same type as the query incase there are multiple
       and unrelated answers */
    if( m->message.question[0].type == m->message.answer[i].type ){
      if( m->message.answer[i].type == A
	  	  || m->message.answer[i].type == AAAA ){
	    /* Standard lookup so convert data to an IP */
	    addr = (struct in_addr *)m->message.answer[i].data;
	    strncpy( m->ip, inet_ntoa( addr[0] ), sizeof(m->ip) );
	    m->cache=1;
	    break;
      } else if( m->message.answer[i].type == PTR ){
	    /* Reverse lookup so convert data to a nume */
	    ptr = m->message.answer[i].data;
	    dns_decode_name( m->cname, &ptr );
	    strncpy(m->ip, m->message.answer[i].name, sizeof(m->ip));
	    m->cache=1;
	    break;
      }
    } /* if( question == answer ) */
  } /* for */
}

