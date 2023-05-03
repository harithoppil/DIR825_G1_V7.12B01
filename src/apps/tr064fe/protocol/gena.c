/*
    $Copyright Open Broadcom Corporation$   
*/
#include <sys/time.h>
#include "upnp_osl.h"
#include "upnp_dbg.h"
#include "upnp.h"
#include "tr64defs.h"
#include "cms.h"

struct gena_connection {
    struct net_connection   net;
    PSubscription	    sub;
};

#define MAX_ADJUSTMENT 10
#define MAXFDS	128
typedef unsigned char uuid[16];

extern struct net_connection *net_connections;
extern const char* rfc1123_fmt;

extern void send_error( int, int, char*, char* , char* );
extern char *strip_chars(char *, char *);
extern char *getsvcval(PService psvc, int i);
extern void remove_net_connection(int fd);

static void gena_send_initial_events(timer_t, void *);
static void gena_connection_handler(caction_t, struct gena_connection *, PService);
static void service_notify_replies(PService psvc, fd_set *fds);
static void reap_service_subscriptions(PService psvc, time_t now);

void reap_subscription_event_receipts(PSubscription);
void reap_service_event_receipts(PService);
extern int gettimeofday ( struct timeval * tv , struct timezone * tz );

PDevice find_dev_by_udn(char *udn)
{
   PDevice pdev = NULL;

   forall_devices(pdev) {
      if (strcmp(udn, pdev->udn) == 0) 
         break;
   }
   
   return pdev;
}


// Given a path of the form "<UDN>/<svcname>", find the corresponding struct Service.
// Returns NULL if no matching service was found.
//
PService find_svc_by_url(char *fname)
{
   PDevice pdev;
   PService psvc = NULL;
   char *udn = fname;
   char *svcname = rindex(fname, '/');

   if (svcname) 
   {
      *svcname++ = '\0';

      if ((pdev = find_dev_by_udn(udn)) != NULL) 
      {
         forall_services(pdev, psvc) {
            if (strcmp(svcname, psvc->template->name) == 0) 
               break;
         }
      }
   }

   return psvc;
}


// Given a service and an SID, usually of the form "uuid:3cfa7fb8-a2e7-4f19-ad41-ffa3aff6d2bd",
// find the corresponding subscription.
//
// If a matching subscription is NOT found, return NULL.
// If a matching subscription is found and it has already expired, return NULL.
// Otherwise, return a pointer to the subscription.
//
PSubscription find_by_sid(const PService psvc, const char *sid)
{
   PSubscription psub;
   time_t now;

   for (psub = psvc->subscriptions; psub; psub = psub->next)
   {
      if (strcmp(psub->sid, sid) == 0) {
         now = time(NULL);
         if (psub->expires && psub->expires < now) 
            psub = NULL;
         break;
      }
   }
   return psub;
}


int split_url(char *url, char **host, int *port, char **path)
{
   int status = FALSE;
   char *p;

   do {
      if (!MATCH_PREFIX(url, "http://"))
         break;

      p = *host = &url[7];
      if ((p = strpbrk(p, ":")) != NULL) 
      {
         *p++ = '\0';
         *port = atoi(p);
      } 
      if (*port == 0) 
         *port = 80;

      p = strpbrk(p, "/");
      if (p) 
         *path = ++p;
      else
         *path = "";

      if (strlen(*host) == 0)
         break;

      status = TRUE;
   } while (0);

   return status;
}

void gena_reply(UFILE *up, PSubscription psub, int timeout)
{
   // now create the response.
   uprintf(up, "HTTP/1.1 200 OK\r\n");
   uprintf(up, "Server: %s\r\n", SERVER);
   uprintf(up, "SID: %s\r\n", psub->sid);
   if (timeout == -1)
      uprintf(up, "TIMEOUT: Second-infinite\r\n");
   else 
      uprintf(up, "TIMEOUT: Second-%d\r\n", timeout);
   uprintf(up, "\r\n" );

   uflush(up);
}

int get_random_fd(void)
{
    int fd;

    while (1) 
    {
        fd = ((int) random()) % MAXFDS;
        if (fd > 2)
        {
            return fd;
        }
    }
}

/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void get_random_bytes(void *buf, int nbytes)
{
    int i, n = nbytes, fd = get_random_fd();
    int lose_counter = 0;
    unsigned char *cp = (unsigned char *) buf;

    if (fd >= 0) 
    {
        while (n > 0) 
        {
            i = read(fd, cp, n);
            if (i <= 0) 
            {
                if (lose_counter++ > 16)
                {
                    break;
                }
                continue;
            }
            n -= i;
            cp += i;
            lose_counter = 0;
        }
    }

    /*
     * We do this all the time, but this is the only source of
     * randomness if /dev/random/urandom is out to lunch.
     */
    for (cp = buf, i = 0; i < nbytes; i++)
    {
        *cp++ ^= (rand() >> 7) & 0xFF;
    }
    return;
}

static int get_clock(uint32 *clock_high, uint32 *clock_low, uint16 *ret_clock_seq)
{
    static int              adjustment = 0;
    static struct timeval   last = {0, 0};
    static uint16           clock_seq;
    struct timeval          tv;
    unsigned long long      clock_reg;

try_again:
    gettimeofday(&tv, 0);
    if ((last.tv_sec == 0) && (last.tv_usec == 0)) 
    {
        get_random_bytes(&clock_seq, sizeof(clock_seq));
        clock_seq &= 0x3FFF;
        last = tv;
        last.tv_sec--;
    }
    if ((tv.tv_sec < last.tv_sec) ||
        ((tv.tv_sec == last.tv_sec) &&
         (tv.tv_usec < last.tv_usec))) 
    {
        clock_seq = (clock_seq+1) & 0x3FFF;
        adjustment = 0;
        last = tv;
    } 
    else if ((tv.tv_sec == last.tv_sec) &&
             (tv.tv_usec == last.tv_usec)) 
    {
        if (adjustment >= MAX_ADJUSTMENT)
        {
            goto try_again;
        }
        adjustment++;
    } 
    else 
    {
        adjustment = 0;
        last = tv;
    }
  
    clock_reg = tv.tv_usec*10 + adjustment;
    clock_reg += ((unsigned long long) tv.tv_sec)*10000000;
    clock_reg += (((unsigned long long) 0x01B21DD2) << 32) + 0x13814000;

    *clock_high = clock_reg >> 32;
    *clock_low = clock_reg;
    *ret_clock_seq = clock_seq;
    return 0;
}

void uuid_pack(const struct _uuid_t *uu, uuid ptr)
{
    uint32  tmp;
    unsigned char  *out = ptr;
   
    tmp = uu->time_low;
    out[3] = (unsigned char) tmp;
    tmp >>= 8;
    out[2] = (unsigned char) tmp;
    tmp >>= 8;
    out[1] = (unsigned char) tmp;
    tmp >>= 8;
    out[0] = (unsigned char) tmp;

    tmp = uu->time_mid;
    out[5] = (unsigned char) tmp;
    tmp >>= 8;
    out[4] = (unsigned char) tmp;

    tmp = uu->time_hi_and_version;
    out[7] = (unsigned char) tmp;
    tmp >>= 8;
    out[6] = (unsigned char) tmp;

    tmp = uu->clock_seq;
    out[9] = (unsigned char) tmp;
    tmp >>= 8;
    out[8] = (unsigned char) tmp;

    memcpy(out+10, uu->node, 6);
}

void uuid_generate_time(uuid out)
{
    //InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    //LanEthIntfObject *ethObj=NULL;
    CmsRet ret;

    static unsigned char mac_id[6];
    static int has_init = 0;
    struct _uuid_t uu;
    uint32  clock_mid;

    if (!has_init) 
    {
       /*
       if ((ret = cmsLck_acquireLockWithTimeout(TR64C_LOCK_TIMEOUT)) != CMSRET_SUCCESS)
       {
          //cmsLog_error("failed to get lock, ret=%d", ret);
       }

       if ((ret = cmsObj_getNext(MDMOID_LAN_ETH_INTF, &iidStack, (void **) &ethObj)) == CMSRET_SUCCESS)
       {
           ret = cmsUtl_macStrToNum(ethObj->MACAddress, mac_id);
           if (ret != CMSRET_SUCCESS)
           {
             //cmsLog_error("Unable to convert macAddrString=%s, unable to release mac addr", ethObj->MACAddress);
           }
           cmsObj_free((void **) &ethObj);
       }

       if(ret != CMSRET_SUCCESS)
       {
           get_random_bytes(mac_id, 6);
           //Set multicast bit, to prevent conflicts  with IEEE 802 addresses obtained from network cards
           mac_id[0] |= 0x01;
       }
   
       cmsLck_releaseLock();
	   */

       has_init = 1;
    }
    get_clock(&clock_mid, &uu.time_low, &uu.clock_seq);
    uu.clock_seq |= 0x8000;
    uu.time_mid = (uint16) clock_mid;
    uu.time_hi_and_version = ((clock_mid >> 16) & 0x0FFF) | 0x1000;
    memcpy(uu.node, mac_id, 6);
    uuid_pack(&uu, out);
}


/*
功能:	对指定接口调用ifconfig命令，并分析输出，查找指定参数。
参数:	ifname - [in] 指定的接口名称，如ath0
		itemname - [in] 要查找的接口参数
		seperator - [in] 参数名和参数值的分隔字符集
		value - [out] 对有值的参数(比如MTU)，需要提供缓冲区来保存参数值，
				对没有参数名的(比如UP, BROADCAST等)，设置value为NULL
		size - [in] value缓冲区的大小。
返回:	0 - 成功
		其他 - 失败
*/
static int IfConfigRead(const char* ifname, const char *pszPrefix, const char* itemname, const char* seperator, char* value, size_t size)
{
	int ret = 0;
	FILE* fp = NULL;
	char cmd[64];
	char valuetemp[64]={0};
	char buff[256];
	char* pos = NULL;

	sprintf(cmd, "ifconfig %s 2>/dev/null", ifname);
	fp = popen(cmd, "r");
	if (!fp) {
		UPNP_TRACE(("err: open pipe of '%s' for read failed - %s!\n", cmd, strerror(errno)));
		return -1;
	}

	ret = -1;		/* default not found */
	while (!feof(fp)) {
		if (!fgets(buff, sizeof(buff), fp)) {
			break;
		}
              //wei added it
              if(pszPrefix) 
              {/*to continue the followint strstr() marked 2, it must meet the condition in this embrace*/
                  char *tp = strstr(buff, pszPrefix);
                  if(!tp) continue;
              }
              //END wei added it
		pos = strstr(buff, itemname);   // 2 
		if (!pos) {
			continue;
		}
		ret = 0;
		UPNP_TRACE(("msg: found '%s' in '%s'.\n", itemname, buff));
		if (value && size) {
			pos += strlen(itemname);
			pos = strtok(pos, seperator);
			if (strlen(pos)<size) 
			{				
		        for(;;)//去掉字符串开始的空格
		      	{
		      		if(*pos==' ')
		      			pos++;
				else	
					break;
		      	}
				
				if(*pos=='\0'){
					break;//字符串为空,直接返回  
				}
				
				strcpy(valuetemp,pos);//dest 和src不能重叠
				strcpy(value,valuetemp);
				pos=value;
				while(*pos!=' '&&*pos!='\0')//去掉字符串后面的部分,加上结束符
				{
					pos++;
				}
				*pos='\0';
				break;
			} else {
				UPNP_TRACE(("err: buffer size too small!\n"));
				ret = -1;
				break;
			}
		}
		break;
	}
	/* 把输出读完，然后关闭文件，否则所执行的命令有可能发生SIG_PIPE异常 */
	while (!feof(fp)) 
	{
		if (!fgets(buff, sizeof(buff), fp)) 
		{
			break;
		}
	}
	pclose(fp);
	return ret;
}

/*
 * get Mac Address
 */
static void GetMacAddr(char* pcMacAddr)
{
    char szIfMac[32] ={0};
    IfConfigRead(TR64_LAN_INTF_NAME, NULL, "HWaddr", " ", szIfMac, sizeof(szIfMac));
    char szMac1[3] ={0};
    char szMac2[3] ={0};
    char szMac3[3] ={0};
    char szMac4[3] ={0};
    char szMac5[3] ={0};
    char szMac6[3] ={0};
    sscanf(szIfMac ,"%2s:%2s:%2s:%2s:%2s:%2s" , szMac1,szMac2,szMac3,szMac4,szMac5,szMac6);
    sprintf(pcMacAddr,"%s%s%s%s%s%s",szMac1,szMac2,szMac3,szMac4,szMac5,szMac6);
}

void uuidstr_create(char *str, int len)
{
    unsigned char d[16];
    uuid_generate_time(d);
	
	char szMacAddr[16]={0};
	GetMacAddr(szMacAddr);
	
    snprintf(str, len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%s",
            (u_int8)d[0], (u_int8)d[1], (u_int8)d[2], (u_int8)d[3], (u_int8)d[4], (u_int8)d[5], (u_int8)d[6], (u_int8)d[7], 
            (u_int8)d[8], (u_int8)d[9], szMacAddr);
}

void gena_new_subscription(UFILE *up, PService psvc, char *cb, char *sid, char *to, char *nt)
{
   struct  itimerspec  timespec;
   PSubscription sr;
   int timeout, port;
   char *host, *path, *endp;

   UPNP_SUBSCRIBE(("Subscribe \"%s\"\n", psvc->template->name));
   if (sid || !cb || !nt || !split_url(cb, &host, &port, &path)) 
   {
      UPNP_SUBSCRIBE(("\tSubscribe HTTP_BAD_REQUEST\n"));
      http_error( up, HTTP_BAD_REQUEST );
   }
   else
   {
      // new subscription request
      sr = (PSubscription) malloc(sizeof(Subscription));
      if (sr)
      {
         memset((char*) sr, 0, sizeof(Subscription));
	    
         // store the split callback URL in the subscription record
         sr->cb_host = strdup(host);
         sr->cb_path = strdup(path);
         sr->cb_port = port;

         // allocate a unique SID for this subscription.
         strcpy(sr->sid, "uuid:");
         uuidstr_create(((char *)sr->sid)+5, sizeof(sr->sid)-5);
         
         if (to && IMATCH_PREFIX(to, "second-")) 
         {
            to += 7;
            if (strcasecmp(to, "infinite") == 0) 
            {
               timeout = -1;
            }
            else
            {
               timeout = strtoul(to, &endp, 10);
               if (*endp || to == endp)
                  timeout = 1800;
            }
         } 
         else
         {
            timeout = 1800;
         }
	    
         if (timeout == -1)
         {
            sr->expires = 0;
         }
         else 
         {
            sr->expires = time(NULL) + timeout;
         }

         UPNP_SUBSCRIBE(("\tsid=\"%s\"\n", sr->sid));
         UPNP_SUBSCRIBE(("\ttimeout %d seconds\n", timeout));

         gena_reply(up, sr, timeout);

         // link the new subscription to this service.
         sr->next = psvc->subscriptions;
         psvc->subscriptions = sr;
	    
         memset(&timespec, 0, sizeof(timespec));
         timespec.it_value.tv_sec = 1;
         enqueue_event(&timespec, (event_callback_t)gena_send_initial_events, NULL);
      } 
      else
      {
         UPNP_ERROR(("\tnew subscription malloc failed\n"));
      }
   }
}

void gena_renew_subscription( UFILE *up, PService psvc, char *cb, char *sid, char *to, char *nt ) 
{
   PSubscription sr;
   int timeout;
   char *endp;

   UPNP_SUBSCRIBE(("Renewing subscription for service \"%s\".\n", psvc->template->name));  
   if (!sid || (sr = find_by_sid(psvc, sid)) == NULL)
   {
      UPNP_SUBSCRIBE(("cannot renew subscription for service \"%s\", sid=%s - not found.\n", psvc->template->name, sid));
      http_error( up, HTTP_PRECONDITION );
   } 
   else if (cb || nt || !to)
   {
      UPNP_SUBSCRIBE(("cannot renew subscription for service \"%s\", sid=%s - bad request.\n", psvc->template->name, sid));
      http_error( up, HTTP_BAD_REQUEST );
   } 
   else 
   {
      if (IMATCH_PREFIX(to, "second-")) 
      {
         to += 7;
         if (strcasecmp(to, "infinite") == 0) 
         {
            timeout = -1;
         } 
         else
         {
            timeout = strtoul(to, &endp, 10);
            if (*endp || to == endp)
               timeout = 1800;
         }
      } 
      else
      {
         timeout = 1800;
      }

      UPNP_SUBSCRIBE(("Renew subscription for service \"%s\", sid=%s.\n", psvc->template->name, sid));
      UPNP_SUBSCRIBE(("\twas set to expire in %d secs, now will expire in %d secs.\n", (int) (sr->expires-time(NULL)), timeout));
      
      if (timeout == -1)
      {
         sr->expires = 0;
      }
      else {
         sr->expires = time(NULL) + timeout;
      }
	
      gena_reply(up, sr, timeout);
   }
}


void delete_subscription(PService psvc, PSubscription sr) 
{
   PSubscription *psr = NULL;

   for (psr = &(psvc->subscriptions); *psr; psr = &((*psr)->next))
   {
      if (*psr == sr) 
      {
         *psr = (*psr)->next;
         // NB: need to remove matching notify reply connections too
         //
         free(sr->cb_host);
         free(sr->cb_path);
         free(sr);
         break;
      }
   }
}


/* 

   Process a GENA UNSUBSCRIBE message that look like this,

    UNSUBSCRIBE /WANCONNECTION/WANPOTSLinkConfig HTTP/1.1
    SID: uuid:3cfa7fb8-a2e7-4f19-ad41-ffa3aff6d2bd
    User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows NT/5.1)
    Host: 10.19.13.136:5431
    Content-Length: 0
    Pragma: no-cache

*/
void process_unsubscribe(UFILE *up, char *fname, char *msg)
{
   PSubscription psub = NULL;
   PService psvc;
   char *lines = msg, *line = NULL, *sid = NULL;
   int badheaders = FALSE;

   if ((psvc = find_svc_by_url(fname)) == NULL) 
   {
      UPNP_SUBSCRIBE(("Cannot unsubscribe- service \"%s\" not found.\n", fname));
      http_error( up, HTTP_NOT_FOUND );
   }
   else 
   {
      // find the SID: header and lookup the correspoding subscription.
      while ((line = strsep(&lines, "\r\n")) != NULL) 
      {
         // If SID header and one of NT or CALLBACK headers are present, 
         // the publisher must respond with HTTP error 400 Bad Request. 
         // UPNP 1.0 Sec 4.1.3
         if (IMATCH_PREFIX(line, "Callback:") || 
             IMATCH_PREFIX(line, "NT:"))
         {
            badheaders = TRUE;
         }
         else if (IMATCH_PREFIX(line, "SID:")) 
         {
            sid = strip_chars(&line[4], " \t");
         }
      }

      if (!sid || (psub = find_by_sid(psvc, sid)) == NULL) 
      {
         UPNP_SUBSCRIBE(("cannot unsubscribe for service \"%s\", sid=%s - not found.\n", psvc->template->name, sid));
         http_error( up, HTTP_PRECONDITION );
      } 
      else if (badheaders) 
      {
         UPNP_SUBSCRIBE(("cannot unsubscribe subscription for service \"%s\", sid=%s - bad request.\n", psvc->template->name, sid));
         http_error( up, HTTP_BAD_REQUEST );
      } 
      else 
      {
         UPNP_SUBSCRIBE(("subscription deleted for service \"%s\", sid=%s\n", psvc->template->name, sid));
         delete_subscription(psvc, psub);
         http_response(up, HTTP_OK, NULL, 0);
      }
   }
}


/*

    NOTIFY delivery path HTTP/1.1
    HOST: delivery host:delivery port
    CONTENT-TYPE: text/xml
    CONTENT-LENGTH: Bytes in body
    NT: upnp:event
    NTS: upnp:propchange
    SID: uuid:subscription-UUID
    SEQ: event key

    <e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
      <e:property>
	<variableName>new value</variableName>
      </e:property>
      Other variable names and values (if any) go here.
    </e:propertyset>

*/

static int build_notify_body(UFILE *up, PService psvc, u_int flags)
{
   StateVar    *pvar;
   VarTemplate *pvartmpl;
   int i, nvars;
   int changed = FALSE;

   /* Build the body of the NOTIFY message... */
   uprintf(up, "<e:propertyset xmlns:e=\"urn:%s:event-1-0\">\n",TR64_DSLFORUM_SCHEMA);
   pvar = psvc->vars;
   pvartmpl = psvc->template->variables;
   nvars = psvc->template->nvariables;
   for (i = 0; i < nvars; i++) 
   {
      if ((pvar->flags & flags) == flags)
      {
         uprintf(up, "<e:property><%s>%s</%s></e:property>\n", 
                 pvartmpl->name, getsvcval(psvc, i), pvartmpl->name);
         changed = TRUE;
      }

      if (flags & VAR_CHANGED) 
         pvar->flags &= ~VAR_CHANGED;

      pvar++;
      pvartmpl++;
   }
   uprintf(up, "</e:propertyset>\n");

   // only append a NULL when talking with WinME systems.
   // *p++ = '\0';

   return changed;
}

static int hostGetByName(char *name)
{
   int addr = INADDR_ANY;
   struct hostent *hent;

   if ((hent = gethostbyname(name)) != NULL) {
      assert(hent->h_addrtype == AF_INET);
      memcpy(&(addr), hent->h_addr, sizeof(addr));
   } else {
      UPNP_ERROR(("hostGetByName(%s) failed\n", name));
   }
   return addr;
}

static struct net_connection *make_gena_connection(PService psvc, PSubscription sr)
{
   struct gena_connection *c = NULL;
   int fd;
   struct linger optval;
   extern int mbufShow ();

   /* create our GENA socket. */
   fd = socket( AF_INET, SOCK_STREAM, 0);
   UPNP_SOCKET(("%s: socket returns %d\n", __FUNCTION__, fd));
    
   if (fd < 0) 
   {
      UPNP_ERROR(("%s: socket failed - err %d\n", __FUNCTION__, errno));
      goto error;
   }

   optval.l_onoff = 1;
   optval.l_linger = 0;
   setsockopt(sr->sock, SOL_SOCKET, SO_LINGER, &optval, sizeof (optval));

   /** if we have not yet resolved the address of this host, do so now.
    * cache the result in the subscription.
    */
   if (sr->addr.sin_addr.s_addr == INADDR_ANY) 
   {
      sr->addr.sin_family = AF_INET;
      sr->addr.sin_port = htons(sr->cb_port);
      if ( !UPNP_INET_ATON(sr->cb_host, &sr->addr.sin_addr) )
      {
         sr->addr.sin_addr.s_addr = hostGetByName(sr->cb_host);
      }
   }
    
   if (sr->addr.sin_addr.s_addr == INADDR_ANY) 
   {
      UPNP_ERROR(("sr->addr.sin_addr.s_addr == INADDR_ANY in %s\n", __FUNCTION__));
      goto error;
   }
    
   if (connect(fd, (struct sockaddr *) &sr->addr, sizeof(sr->addr)) == -1) {
      UPNP_ERROR(("connect failed in %s\n", __FUNCTION__));
      goto error;
   }

   c = (struct gena_connection *) malloc(sizeof(struct gena_connection));
   if (c == NULL) 
   {
      UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
      goto error;
   }
    memset(c, 0, sizeof(struct net_connection));
    
    c->net.fd = fd;
    c->net.expires = time(NULL) + NOTIFY_RECEIPT_TIMEOUT;
    c->net.func = (CONNECTION_HANDLER) gena_connection_handler;
    c->net.arg = psvc;
    c->sub = sr;
    
    return (struct net_connection *) c;

 error:
    /* cleanup code in case we need to bail out. */
    UPNP_ERROR(("%s: failed - err %d\n", __FUNCTION__, errno));
    if (fd >= 0) 
    {
       UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, fd));
       close(fd);
    }
	
    return NULL;
}


static void gena_connection_handler(caction_t flag, struct gena_connection *gc, PService psvc)
{
   int nbytes;
   char buf[1024], *lines, *line;
    
   if (flag == CONNECTION_RECV)
   {
      nbytes = read(gc->net.fd, buf, sizeof(buf));
	
      if (nbytes < 0) 
         nbytes = 0;
	
      buf[nbytes] = '\0';

      /* process first line here to find out what kind of request it is... */
      lines = buf;
      line = strsep(&lines, "\r\n");
      if (line == NULL || !IMATCH_PREFIX(line, "HTTP/1.1 200 OK"))
      {
         /** we expected "HTTP/1.1 200 OK"
          * for any error, we terminate the subscription.
          */
	    delete_subscription(psvc, gc->sub);
      } 
      remove_net_connection(gc->net.fd);
   } 
   else if (flag == CONNECTION_DELETE) 
   {
      UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, gc->net.fd));
      close(gc->net.fd);
      free(gc);
   }
}

static int gena_notify(PService psvc, PSubscription sr, char *body, int bodylen)
{
   struct net_connection *c = NULL;
   char header[1024];
   struct iovec iovec[2];
   UFILE *uheader;
    
   c = make_gena_connection(psvc, sr);
   if (c != NULL) 
   {
      /* Build an appropriate NOTIFY header and send it to each subscriber... */
      uheader = usopen(header, sizeof(header));
      if (uheader != NULL) 
      {
         uprintf(uheader, "NOTIFY /%s HTTP/1.1\r\n", sr->cb_path);
         uprintf(uheader, "HOST: %s:%d\r\n", sr->cb_host, sr->cb_port);
         uprintf(uheader, "CONTENT-TYPE: text/xml\r\n");
         uprintf(uheader, "CONTENT-LENGTH: %d\r\n", bodylen);
         uprintf(uheader, "NT: upnp:event\r\n");
         uprintf(uheader, "NTS: upnp:propchange\r\n");
         uprintf(uheader, "SID: %s\r\n", sr->sid);
         uprintf(uheader, "SEQ: %u\r\n", sr->event_key);
         uprintf(uheader, "\r\n");
	
         /** now send the notification to just this particular subscription
          * set up the io-vector for use with writev(2).
          */
         iovec[0].iov_base = ubuffer(uheader);
         iovec[0].iov_len = utell(uheader);
         iovec[1].iov_base = body;
         iovec[1].iov_len = bodylen;
	    
         UPNP_PREVENT(("%s\n", body));

         if (writev(c->fd, iovec, 2) > 0)
         {
            /* increment and wrap event key, per sec. 4.2 of UPNP1.0 spec. */
            if (++sr->event_key == 0)
               sr->event_key = 1;
         }

         uclose(uheader);
      }

      /** whether or not our notification was successful,
       * stash this connection away in anticipation of a reply.
       * If no reply is received, this connection will be cleaned up
       * after some delay.
       */
      c->next = net_connections;
      net_connections = (struct net_connection *) c;
   }
   return TRUE;
}

static void svc_send_initial_events(PService psvc)
{
   PSubscription psub, nextsub;
   UFILE *ubody = NULL;
   int need_update = FALSE;
   
   /** optimization - before going through the expense of creating the notification body,
    * first see if there are any subscriptions that need updating.
    */
   for (psub = psvc->subscriptions; psub; psub = psub->next) 
   {
      if (psub->event_key == 0) 
      {
         need_update = TRUE;
         break;
      }
   }

   if (need_update && (ubody = usopen(NULL, 0)) != NULL) 
   {
      if (build_notify_body(ubody, psvc, VAR_EVENTED)) 
      {
         for (psub = psvc->subscriptions; psub; psub = nextsub) 
         {
            /** pre-cache pointer to next subscription 
             * because the current psub may get deleted 
             * if there is an error during notification.
             */
            nextsub = psub->next;
		
            /** if we have never notified this subscription, 
             * its event_key will be zero.
             */
            if (psub->event_key == 0) 
            {
               if ( !gena_notify(psvc, psub, ubuffer(ubody), utell(ubody)) ) 
               {
                  UPNP_SUBSCRIBE(("Subscriber notification failed\nRemoving subscription for %s.\n", psvc->template->name));
                  delete_subscription(psvc, psub);
               }
            }
         }
      }
	
      uclose(ubody);
   }
}

/* send the complete variable list for a service to a particular subscription. */
static void gena_send_initial_events(timer_t t, void *arg)
{
   PDevice pdev;
   PService psvc;

   forall_devices(pdev) {
      forall_services(pdev, psvc) {
         svc_send_initial_events(psvc);
      }
   }

   /* delete this timer and free the resources. */
   timer_delete(t);
}


void update_all_subscriptions(PService psvc)
{
   PSubscription psub, nextsub;
   UFILE *ubody;

   /** clear the service-wide VAR_CHANGED flag.
    * build_notify_body() will take care of clearing this 
    * flag on a per-state variable basis, if necessary.
    */
   psvc->flags &= ~VAR_CHANGED;

   if (psvc->subscriptions) 
   {
      /* build the body of the notification. */
      if ((ubody = usopen(NULL, 0))) 
      {
         build_notify_body(ubody, psvc, VAR_EVENTED|VAR_CHANGED);

         UPNP_PREVENT(("update_all_subscriptions %s\n%s\n", psvc->template->name, ubuffer(ubody)));
	
         for (psub = psvc->subscriptions; psub; psub = nextsub) 
         {
            /** pre-cache pointer to next subscription 
             * because the current psub may get deleted 
             * if there is an error during notification.
             */
            nextsub = psub->next;
		
            if ( !gena_notify(psvc, psub, ubuffer(ubody), utell(ubody)) ) 
            {
               UPNP_SUBSCRIBE(("Subscriber notification failed\nRemoving subscription for %s.\n", psvc->template->name));
               delete_subscription(psvc, psub);
            }
         }
         uclose(ubody);
      }
   }
}



/* Big hammer to delete all existing subscriptions. 
   Typically this should only be used in preparation for shutting down the UPNP server. 
*/
void delete_all_subscriptions()
{
   PDevice pdev;
   PService psvc;
   PSubscription psub, nextsub;

   forall_devices(pdev) {
      forall_services(pdev, psvc) {
         for (psub = psvc->subscriptions; psub; psub = nextsub) 
         {
            /** pre-cache pointer to next subscription 
             * because the current psub may get deleted 
             * if there is an error during notification.
             */
            nextsub = psub->next;
	
            delete_subscription(psvc, psub);
         }
      }
   }
}

void receive_notify_replies(fd_set *fds)
{
   PDevice pdev;
   PService psvc;

   forall_devices(pdev) {
      forall_services(pdev, psvc) {
         service_notify_replies(psvc, fds);
      }
   }
}

/** helper routine for receive_notify_replies.
 * This routine will take a fd_set (set of bits representing file descriptors for which received data is available)
 * and check each servicer subscription to see whether that subscription was expecting a reply or not.
 */
static void service_notify_replies(PService psvc, fd_set *fds)
{
   int nbytes;
   PSubscription sr, *psr;
   char buf[1024], *lines, *line, *replycode;

   /** Loop through all the subscriptions in this service 
    * looking ones that might be expecting a reply.
    * The loop iteration is a little strange because we may need to 
    * remove a subscription while in the middle of the loop.
    */    
   psr = &(psvc->subscriptions);
   while (*psr) 
   {
      if (FD_ISSET((*psr)->sock, fds)) 
      {
         FD_CLR((*psr)->sock, fds);
	    
         nbytes = read((*psr)->sock, buf, sizeof(buf));
	    
         /* we are going to close this socket no matter what happens, so do it now. */
         UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, (*psr)->sock));
         (*psr)->sock = 0;
         
         if (nbytes < 0) 
         {
            perror("recv");
         } 
         else
         {
            buf[nbytes] = '\0';
            /* process first line here to find out what kind of request it is... */
            lines = buf;
            line = strsep(&lines, "\r\n");     /* get the first line off the reply. */

            /* Now start processing the first line of the reply. */
            (void) strsep(&line, " \t");	   // do this just to skip the first token.
            replycode = strsep(&line, " \t");  // we actually want the second token...
            if (strcmp(replycode, "200") != 0)
            {
               /** we expected "HTTP/1.1 200 OK"
                * for any error, we terminate the subscription.
                */
               UPNP_SUBSCRIBE(("Received error reply code to a notification:\nReply code was \"%s\"\nRemoving subscription for %s.\n", 
                               replycode, psvc->template->name));
               /* remove the matching subscription from the list */
               sr = *psr;
               *psr = (*psr)->next;
               free(sr->cb_host);
               free(sr->cb_path);
               free(sr);
               
               /* do NOT advance to the next subscription - it has already been done. */
               continue;
            } 
         } /* end-else */
      } /* end-if */
      psr = &((*psr)->next);
   }
}

 
void gena_subscription_reaper(timer_t t, void *arg)
{
   PDevice pdev;
   PService psvc;
   time_t now;
   
   now = time(NULL);

   forall_devices(pdev) {
      forall_services(pdev, psvc) {
         reap_service_subscriptions(psvc, now);
      }
   }
}

static void reap_service_subscriptions(PService psvc, time_t now)
{
   PSubscription *ppsr, sr;
   int i = 0;

   ppsr = &(psvc->subscriptions);
   while (*ppsr) 
   {
      i++;
      if ((*ppsr)->expires && (*ppsr)->expires < now) 
      {
         UPNP_SUBSCRIBE(("Reaping subscription for %s.\n", psvc->template->name));
         sr = *ppsr;
         *ppsr = (*ppsr)->next;
		
         free(sr->cb_host);
         free(sr->cb_path);
         free(sr);
         continue;
      }
      ppsr = &((*ppsr)->next);
   }
}
 
