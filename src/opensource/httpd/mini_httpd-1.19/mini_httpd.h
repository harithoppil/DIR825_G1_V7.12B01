#define TR69PORT 7547
#define TR69URL "tr69"
#define TR69INF "eth1"
#define BUFF_SIZE 256
#define LARGE_BUFF 10000
#define MEDIUM_BUFF 1000
#define SMALL_BUFF 100
//#define MAX_SERVICES 5/*** 3+2 for WAN Access ***/
#ifdef USE_SSL
#define MAX_SERVICES 2
#else
#define MAX_SERVICES 2
#endif
#define HTTP_PORT 80
#ifdef USE_OTHER_PORT
#define HTTP_OTHER_PORT 8000
#endif

#define HTTPS_PORT 443
#define MAX_INTERFACES 2
#define MAX_ARRAY 10
#define AUTH_DGST_FILE ".htdgst"
#define START_TELNET 1
#define CLOSE_TELNET 0

extern int CalcDigest(char *,char *,char *,char *,char *,char *,char *,char *,char *,char **);
void send_emptyResponse();
//char* read_env_str(const char * varname, const char * result, int len) ;

