/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include <utmp.h>
#include <sys/resource.h>
#include <syslog.h>

#if ENABLE_SELINUX
#include <selinux/selinux.h>  /* for is_selinux_enabled()  */
#include <selinux/get_context_list.h> /* for get_default_context() */
#include <selinux/flask.h> /* for security class definitions  */
#endif

enum {
	TIMEOUT = 600,
	EMPTY_USERNAME_COUNT = 10,
	USERNAME_SIZE = 32,
	TTYNAME_SIZE = 32,
};

static char* short_tty;

#if ENABLE_FEATURE_UTMP
/* vv  Taken from tinylogin utmp.c  vv */
/*
 * read_or_build_utent - see if utmp file is correct for this process
 *
 *	System V is very picky about the contents of the utmp file
 *	and requires that a slot for the current process exist.
 *	The utmp file is scanned for an entry with the same process
 *	ID.  If no entry exists the process exits with a message.
 *
 *	The "picky" flag is for network and other logins that may
 *	use special flags.  It allows the pid checks to be overridden.
 *	This means that getty should never invoke login with any
 *	command line flags.
 */

static void read_or_build_utent(struct utmp *utptr, int picky)
{
	struct utmp *ut;
	pid_t pid = getpid();

	setutent();

	/* First, try to find a valid utmp entry for this process.  */
	while ((ut = getutent()))
		if (ut->ut_pid == pid && ut->ut_line[0] && ut->ut_id[0] &&
		(ut->ut_type == LOGIN_PROCESS || ut->ut_type == USER_PROCESS))
			break;

	/* If there is one, just use it, otherwise create a new one.  */
	if (ut) {
		*utptr = *ut;
	} else {
		if (picky)
			bb_error_msg_and_die("no utmp entry found");

		memset(utptr, 0, sizeof(*utptr));
		utptr->ut_type = LOGIN_PROCESS;
		utptr->ut_pid = pid;
		strncpy(utptr->ut_line, short_tty, sizeof(utptr->ut_line));
		/* This one is only 4 chars wide. Try to fit something
		 * remotely meaningful by skipping "tty"... */
		strncpy(utptr->ut_id, short_tty + 3, sizeof(utptr->ut_id));
		strncpy(utptr->ut_user, "LOGIN", sizeof(utptr->ut_user));
		utptr->ut_time = time(NULL);
	}
	if (!picky)	/* root login */
		memset(utptr->ut_host, 0, sizeof(utptr->ut_host));
}

/*
 * write_utent - put a USER_PROCESS entry in the utmp file
 *
 *	write_utent changes the type of the current utmp entry to
 *	USER_PROCESS.  the wtmp file will be updated as well.
 */
static void write_utent(struct utmp *utptr, const char *username)
{
	utptr->ut_type = USER_PROCESS;
	strncpy(utptr->ut_user, username, sizeof(utptr->ut_user));
	utptr->ut_time = time(NULL);
	/* other fields already filled in by read_or_build_utent above */
	setutent();
	pututline(utptr);
	endutent();
#if ENABLE_FEATURE_WTMP
	if (access(bb_path_wtmp_file, R_OK|W_OK) == -1) {
		close(creat(bb_path_wtmp_file, 0664));
	}
	updwtmp(bb_path_wtmp_file, utptr);
#endif
}
#else /* !ENABLE_FEATURE_UTMP */
#define read_or_build_utent(utptr, picky) ((void)0)
#define write_utent(utptr, username) ((void)0)
#endif /* !ENABLE_FEATURE_UTMP */

static void die_if_nologin_and_non_root(int amroot)
{
	FILE *fp;
	int c;

	if (access(bb_path_nologin_file, F_OK))
		return;

	fp = fopen(bb_path_nologin_file, "r");
	if (fp) {
		while ((c = getc(fp)) != EOF)
			putchar((c=='\n') ? '\r' : c);
		fflush(stdout);
		fclose(fp);
	} else
		puts("\r\nSystem closed for routine maintenance\r");
	if (!amroot)
		exit(1);
	puts("\r\n[Disconnect bypassed -- root login allowed.]\r");
}

#if ENABLE_FEATURE_SECURETTY
static int check_securetty(void)
{
	FILE *fp;
	int i;
	char buf[BUFSIZ];

	fp = fopen(bb_path_securetty_file, "r");
	if (!fp) {
		/* A missing securetty file is not an error. */
		return 1;
	}
	while (fgets(buf, sizeof(buf)-1, fp)) {
		for (i = strlen(buf)-1; i>=0; --i) {
			if (!isspace(buf[i]))
				break;
		}
		buf[++i] = '\0';
		if ((buf[0]=='\0') || (buf[0]=='#'))
			continue;
		if (strcmp(buf, short_tty) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
#else
static inline int check_securetty(void) { return 1; }
#endif

static void get_username_or_die(char *buf, int size_buf)
{
	int c, cntdown;
	cntdown = EMPTY_USERNAME_COUNT;
prompt:
	/* skip whitespace */
	print_login_prompt();
	do {
		c = getchar();
		if (c == EOF) exit(1);
		if (c == '\n') {
			if (!--cntdown) exit(1);
			goto prompt;
		}
	} while (isspace(c));

	*buf++ = c;
	if (!fgets(buf, size_buf-2, stdin))
		exit(1);
	if (!strchr(buf, '\n'))
		exit(1);
	while (isgraph(*buf)) buf++;
	*buf = '\0';
}

static void motd(void)
{
	int fd;

	fd = open(bb_path_motd_file, O_RDONLY);
	if (fd) {
		fflush(stdout);
		bb_copyfd_eof(fd, STDOUT_FILENO);
		close(fd);
	}
}

static void alarm_handler(int sig ATTRIBUTE_UNUSED)
{
	/* This is the escape hatch!  Poor serial line users and the like
	 * arrive here when their connection is broken.
	 * We don't want to block here */
	ndelay_on(1);
	ndelay_on(2);
	printf("\r\nLogin timed out after %d seconds\r\n", TIMEOUT);
	exit(EXIT_SUCCESS);
}

int login_main(int argc, char **argv);
int login_main(int argc, char **argv)
{
	enum {
		LOGIN_OPT_f = (1<<0),
		LOGIN_OPT_h = (1<<1),
		LOGIN_OPT_p = (1<<2),
	};
	char fromhost[512];
	char username[USERNAME_SIZE];
	const char *tmp;
	int amroot;
	unsigned opt;
	int count = 0;
	struct passwd *pw;
	char *opt_host = NULL;
	char *opt_user = NULL;
	char full_tty[TTYNAME_SIZE];
	USE_SELINUX(security_context_t user_sid = NULL;)
	USE_FEATURE_UTMP(struct utmp utent;)
#if 0
	FILE *fp=NULL;
	char szBuffer[256]={0};
#endif
	short_tty = full_tty;
	username[0] = '\0';
	amroot = (getuid() == 0);
	//signal(SIGALRM, alarm_handler);
	//alarm(TIMEOUT);
#if 0
	fp=fopen("/var/factorymode.txt","r");
	if(fp==NULL)
	{
		fclose(fp);
	}
	else
	{
		fgets(szBuffer,sizeof(szBuffer),fp);
	}
#endif	
	/* Mandatory paranoia for suid applet:
	 * ensure that fd# 0,1,2 are opened (at least to /dev/null)
	 * and any extra open fd's are closed.
	 * (The name of the function is misleading. Not daemonizing here.) */
	bb_daemonize_or_rexec(DAEMON_ONLY_SANITIZE | DAEMON_CLOSE_EXTRA_FDS, NULL);

	opt = getopt32(argc, argv, "f:h:p", &opt_user, &opt_host);
	if (opt & LOGIN_OPT_f) {
		if (!amroot)
			bb_error_msg_and_die("-f is for root only");
		safe_strncpy(username, opt_user, sizeof(username));
	}
	if (optind < argc) /* user from command line (getty) */
		safe_strncpy(username, argv[optind], sizeof(username));

	/* Let's find out and memorize our tty */
	if (!isatty(0) || !isatty(1) || !isatty(2))
		return EXIT_FAILURE;		/* Must be a terminal */
	safe_strncpy(full_tty, "UNKNOWN", sizeof(full_tty));
	tmp = ttyname(0);
	if (tmp) {
		safe_strncpy(full_tty, tmp, sizeof(full_tty));
		if (strncmp(full_tty, "/dev/", 5) == 0)
			short_tty = full_tty + 5;
	}

	read_or_build_utent(&utent, !amroot);

	if (opt_host) {
		USE_FEATURE_UTMP(
			safe_strncpy(utent.ut_host, opt_host, sizeof(utent.ut_host));
		)
		snprintf(fromhost, sizeof(fromhost)-1, " on '%.100s' from "
					"'%.200s'", short_tty, opt_host);
	} else
		snprintf(fromhost, sizeof(fromhost)-1, " on '%.100s'", short_tty);

	// Was breaking "login <username>" from shell command line:
	// bb_setpgrp();

	openlog(applet_name, LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_AUTH);
#if 0
if (strstr(szBuffer, "fullstart") == NULL)
{
#endif
	while (1) {
		if (!username[0])
			get_username_or_die(username, sizeof(username));

		pw = getpwnam(username);
		if (!pw) {
			strcpy(username, "UNKNOWN");
			goto fake_it;
		}

		if (pw->pw_passwd[0] == '!' || pw->pw_passwd[0] == '*')
			goto auth_failed;

		if (opt & LOGIN_OPT_f)
			break; /* -f USER: success without asking passwd */

		if (pw->pw_uid == 0 && !check_securetty())
			goto auth_failed;

		/* Don't check the password if password entry is empty (!) */
		if (!pw->pw_passwd[0])
			break;
 fake_it:
		/* authorization takes place here */
		if (correct_password(pw))
			break;
 auth_failed:
		opt &= ~LOGIN_OPT_f;
		bb_do_delay(FAIL_DELAY);
		puts("Login incorrect");
		if (++count == 3) {
			syslog(LOG_WARNING, "invalid password for '%s'%s",
						username, fromhost);
			return EXIT_FAILURE;
		}
		username[0] = '\0';
	}
#if 0
}	
else
{
	strcpy(username, "root");
	pw = getpwnam(username);
}
#endif	
	alarm(0);
	die_if_nologin_and_non_root(pw->pw_uid == 0);

	write_utent(&utent, username);

#ifdef CONFIG_SELINUX
	if (is_selinux_enabled()) {
		security_context_t old_tty_sid, new_tty_sid;

		if (get_default_context(username, NULL, &user_sid)) {
			bb_error_msg_and_die("cannot get SID for %s",
					username);
		}
		if (getfilecon(full_tty, &old_tty_sid) < 0) {
			bb_perror_msg_and_die("getfilecon(%s) failed",
					full_tty);
		}
		if (security_compute_relabel(user_sid, old_tty_sid,
					SECCLASS_CHR_FILE, &new_tty_sid) != 0) {
			bb_perror_msg_and_die("security_change_sid(%s) failed",
					full_tty);
		}
		if (setfilecon(full_tty, new_tty_sid) != 0) {
			bb_perror_msg_and_die("chsid(%s, %s) failed",
					full_tty, new_tty_sid);
		}
	}
#endif
	/* Try these, but don't complain if they fail.
	 * _f_chown is safe wrt race t=ttyname(0);...;chown(t); */
	fchown(0, pw->pw_uid, pw->pw_gid);
	fchmod(0, 0600);

	if (ENABLE_LOGIN_SCRIPTS) {
		char *t_argv[2];

		t_argv[0] = getenv("LOGIN_PRE_SUID_SCRIPT");
		if (t_argv[0]) {
			t_argv[1] = NULL;
			xsetenv("LOGIN_TTY", full_tty);
			xsetenv("LOGIN_USER", pw->pw_name);
			xsetenv("LOGIN_UID", utoa(pw->pw_uid));
			xsetenv("LOGIN_GID", utoa(pw->pw_gid));
			xsetenv("LOGIN_SHELL", pw->pw_shell);
			xspawn(t_argv); /* NOMMU-friendly */
			/* All variables are unset by setup_environment */
			wait(NULL);
		}
	}

	change_identity(pw);
	tmp = pw->pw_shell;
	if (!tmp || !*tmp)
		tmp = DEFAULT_SHELL;
	setup_environment(tmp, 1, !(opt & LOGIN_OPT_p), pw);

	motd();

	if (pw->pw_uid == 0)
		syslog(LOG_INFO, "root login%s", fromhost);
#ifdef CONFIG_SELINUX
	/* well, a simple setexeccon() here would do the job as well,
	 * but let's play the game for now */
	set_current_security_context(user_sid);
#endif

	// util-linux login also does:
	// /* start new session */
	// setsid();
	// /* TIOCSCTTY: steal tty from other process group */
	// if (ioctl(0, TIOCSCTTY, 1)) error_msg...
	// BBox login used to do this (see above):
	// bb_setpgrp();
	// If this stuff is really needed, add it and explain why!

	/* set signals to defaults */
	signal(SIGALRM, SIG_DFL);
	/* Is this correct? This way user can ctrl-c out of /etc/profile,
	 * potentially creating security breach (tested with bash 3.0).
	 * But without this, bash 3.0 will not enable ctrl-c either.
	 * Maybe bash is buggy?
	 * Need to find out what standards say about /bin/login -
	 * should it leave SIGINT etc enabled or disabled? */
	signal(SIGINT, SIG_DFL);
#ifdef CLI_LOGIN
	//execl("/usr/bin/cmd", (char *)0);
/*为了生产效率，如果是工厂模式的时候，进入shell，否则进入cli用户模式*/
	if (strstr(szBuffer, "fullstart") == NULL)
	{
		run_shell(tmp, 1, 0, 0);
		fclose(fp);
	}
	else
	{
		run_shell("/usr/bin/cmd", 1, 0, 0);
		fclose(fp);
	}
#else
	run_shell(tmp, 1, 0, 0);	/* exec the shell finally */
#endif
	return EXIT_FAILURE;
}
