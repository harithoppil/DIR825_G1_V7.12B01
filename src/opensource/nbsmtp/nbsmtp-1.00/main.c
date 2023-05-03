/**
 * \file main.c
 * \brief This file has the main function. Execution starts here
 *
 * \author Fernando J. Pereda <ferdy@ferdyx.org>
 * \author Ricardo Cervera Navarro <ricardo@zonasiete.org>
 *
 * This is part of nbsmtp. nbsmtp is free software;
 * you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * nbsmtp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nbsmtp; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * See COPYING for details.
 */

#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "original.h"
#include "util.h"
#include "servinfo.h"
#include "nbsmtp.h"

#ifdef HAVE_GROWLNOTIFY
#include "osx.h"
#endif

/**
 * \brief nbSMTP starts its execution here
 *
 * \param[in] argc Number of arguments
 * \param[in] argv Array with the arguments
 * \return 0 if everything goes ok, 1 if something goes wrong
 */
int main(int argc, char *argv[])
{
	int  i;
	char vmkeys[1024], vmkey_buffer[1024];
	char attchfile[256], sendfile[256], subject[1024];
	servinfo_t serverinfo;
	string_t msg_buffer;
	string_t *rcpts = NULL;

	str_init(&msg_buffer, MAX_MSG_LEN);

	serverinfo.host = (char) NULL;
	serverinfo.fromaddr = (char) NULL;
	serverinfo.domain = (char) NULL;
	serverinfo.auth_user = (char) NULL;
	serverinfo.auth_pass = (char) NULL;
	serverinfo.to = (char) NULL;
	serverinfo.auth_mech = SASL_DEFAULT;
	serverinfo.port = 0;
	serverinfo.num_rcpts = 0;
#ifdef HAVE_SSL
	serverinfo.use_tls = False;
	serverinfo.use_starttls = False;
	serverinfo.using_tls = False;
#endif

	/**
	 * HACK
	 * This is a quick and dirty hack to fix the silly options in nbSMTP
	 * I want to change them but it is better to provide backwards compatibility.
	 * This calls print_help if the call was something like 'nbmtp -h' with no aditional args
	 * Also add --help and --version
	 *
	 * \todo Fix the command line options... maybe breaking 0.8 compatibility in 1.0
	 */
	if (argc==2)
	{
		if ( strncmp(argv[1],"-h",2)==0 || strncmp(argv[1],"--help",6)==0 )
		{
			print_help(argv[0]);
			return 0;
		}
		else if ( strncmp(argv[1],"--version",9)==0 )
		{
			printf("nbSMTP version %s\n",PACKAGE_VERSION);
			return 0;
		}
	}

	/* 获取系统参数 */
	attchfile[0] = '\0';
	sendfile[0]  = '\0';
	subject[0]   = '\0';
	
	switch (parse_options(argc,argv, &serverinfo, vmkeys, attchfile, sendfile, subject))
	{
		case 1:
			return 1;
			break;
		case 2:
			return 0;
			break;
	}
#if 0
	/* 没有邮件内容 */
	/*=========================================================*/
	if (strlen(sendfile) == 0)
		return 1;
	/*=========================================================*/
#endif
#if 0	
	/*=========================================================*/
	printf("serverinfo.host:      %s \n\r" , serverinfo.host);
	printf("serverinfo.fromaddr:  %s \n\r" , serverinfo.fromaddr);
	printf("serverinfo.domain:    %s \n\r" , serverinfo.domain);
	printf("serverinfo.auth_user: %s \n\r" , serverinfo.auth_user);
	printf("serverinfo.auth_pass: %s \n\r" , serverinfo.auth_pass);
	printf("serverinfo.auth_mech: %d \n\r" , serverinfo.auth_mech);
	printf("serverinfo.port:      %d \n\r" , serverinfo.port);
	printf("serverinfo.num_rcpts: %d \n\r" , serverinfo.num_rcpts);
#ifdef HAVE_SSL
	printf("serverinfo.use_tls:      %d \n\r" , serverinfo.use_tls);
	printf("serverinfo.use_starttls: %d \n\r" , serverinfo.use_starttls);
	printf("serverinfo.using_tls:    %d \n\r" , serverinfo.using_tls);
#endif
#endif
	/*=========================================================*/
	
	/*=========================================================*/
	sprintf(vmkey_buffer, "to: %s; \n\n", vmkeys);

	/* ./nbsmtp -f deeploves@163.com  -U deeploves  -P 350627  -h smtp.163.com -d www.163.com -p 25 -A att.txt -a smtp.h -t my_sales@126.com -T testemail -V -N -n */
	/* ./nbsmtp -f deeploves@163.com  -U deeploves  -P 350627  -h smtp.163.com -d www.163.com -p 25 -A att.txt -a smtp.h -t luyanyong@163.com -T testemail -V -N -n */
	/* ./nbsmtp 
	-f deeploves@163.com 发送的源邮件地址
	-U deeploves  用户名
	-P 350627  密码
	-h smtp.163.com 主机名
	-d www.163.com 域名
	-p 25 端口
	-A att.txt  邮件附件
	-a scr.html  邮件内容
	-t luyanyong@163.com  对方邮件地址
	-T testemail 邮件标题
	-V 打印信息到控制台
	-N -n 不读取配置文件
	*/	
//	printf("\n\r ... \n\r");
	/*=========================================================*/
	/* 分离出目标邮件地址 */
	
	if((rcpts = parse_mail(&msg_buffer, &(serverinfo.num_rcpts), vmkey_buffer)) == NULL)
	{
		log_msg(LOG_DEBUG, "Error in parse_mail");
		if (debug > 0)
		{
			log_msg(LOG_ERR, "Mail NOT sent.");
			
		}
		else
		{
			log_msg(LOG_ERR, "Something went wrong. Mail NOT sent. Activate debug for more info");
		}
		printf("(sent Mail Failed)\n");
		return 1;

#ifdef HAVE_GROWLNOTIFY
		osx_notify("nbSMTP","Mail NOT sent, see log for more details");
#endif
	}
#if 0
	/*=========================================================*/
	printf("serverinfo.host:      %s \n\r" , serverinfo.host);
	printf("serverinfo.fromaddr:  %s \n\r" , serverinfo.fromaddr);
	printf("serverinfo.domain:    %s \n\r" , serverinfo.domain);
	printf("serverinfo.auth_user: %s \n\r" , serverinfo.auth_user);
	printf("serverinfo.auth_pass: %s \n\r" , serverinfo.auth_pass);
	printf("serverinfo.auth_mech: %d \n\r" , serverinfo.auth_mech);
	printf("serverinfo.port:      %d \n\r" , serverinfo.port);
	printf("serverinfo.num_rcpts: %d \n\r" , serverinfo.num_rcpts);
#ifdef HAVE_SSL
	printf("serverinfo.use_tls:      %d \n\r" , serverinfo.use_tls);
	printf("serverinfo.use_starttls: %d \n\r" , serverinfo.use_starttls);
	printf("serverinfo.using_tls:    %d \n\r" , serverinfo.using_tls);
#endif
	/*=========================================================*/
#endif
	/*=========================================================*/
	for (i = 0; i < serverinfo.num_rcpts; i++)
//		printf("rcpts[%d]: %d, %s \n\r", i, rcpts[i].len, rcpts[i].str);

//	printf("msg_buffer: %d, %s \n\r", msg_buffer.len, msg_buffer.str);
	/*=========================================================*/

	if(send_mail(&msg_buffer, &serverinfo, rcpts, subject, attchfile, sendfile))
	{
		log_msg(LOG_NOTICE, "Error in send_mail");
		log_record(LOG_DEBUG, "[email] Error in send_mail");
		if (debug > 0)
		{
			log_msg(LOG_ERR, "Mail NOT sent.");
			log_record(LOG_NOTICE, "[email] Mail NOT sent.");
		}
		else
		{
			log_msg(LOG_ERR, "Something went wrong. Mail NOT sent. Activate debug for more info");
			log_record(LOG_NOTICE, "[email] Something went wrong. Mail NOT sent.");
		}

#ifdef HAVE_GROWLNOTIFY
		osx_notify("nbSMTP","Mail NOT sent, see log for more details");
#endif
        printf("(sent Mail Failed)\n");
		return 1;
	}
    printf("(sent Mail OK)\n");
	log_msg(LOG_INFO,"[email] Mail sent for %s. Closing connection",serverinfo.fromaddr);
	char szLog[256]={0};
	sprintf(szLog,"[email] Mail sent to %s success.",vmkeys);
	log_record(LOG_NOTICE, szLog);

#ifdef HAVE_GROWLNOTIFY
	osx_notify("nbSMTP","Mail sent for %s",serverinfo.fromaddr);
#endif

	str_free(rcpts);
	free(rcpts);
	str_free(&msg_buffer);

	SERVINFO_RELEASE_OPTION(serverinfo.host);
	SERVINFO_RELEASE_OPTION(serverinfo.fromaddr);
	SERVINFO_RELEASE_OPTION(serverinfo.domain);
	SERVINFO_RELEASE_OPTION(serverinfo.auth_user);
	SERVINFO_RELEASE_OPTION(serverinfo.auth_pass);

	return 0;
}
