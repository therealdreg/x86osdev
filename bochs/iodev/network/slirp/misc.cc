/////////////////////////////////////////////////////////////////////////
// $Id: misc.cc 13932 2020-09-02 08:35:44Z vruppert $
/////////////////////////////////////////////////////////////////////////
/*
 * Copyright (c) 1995 Danny Gasparovski.
 *
 * Please read the file COPYRIGHT for the
 * terms and conditions of the copyright.
 */

#include "slirp.h"
#include "libslirp.h"

#ifndef _WIN32
#include <dirent.h>
#endif

#if BX_NETWORKING && BX_NETMOD_SLIRP

#ifdef DEBUG
int slirp_debug = DBG_CALL|DBG_MISC|DBG_ERROR;
#endif

struct quehead {
	struct quehead *qh_link;
	struct quehead *qh_rlink;
};

void insque(void *a, void *b)
{
	struct quehead *element = (struct quehead *) a;
	struct quehead *head = (struct quehead *) b;
	element->qh_link = head->qh_link;
	head->qh_link = (struct quehead *)element;
	element->qh_rlink = (struct quehead *)head;
	((struct quehead *)(element->qh_link))->qh_rlink
	= (struct quehead *)element;
}

void remque(void *a)
{
  struct quehead *element = (struct quehead *) a;
  ((struct quehead *)(element->qh_link))->qh_rlink = element->qh_rlink;
  ((struct quehead *)(element->qh_rlink))->qh_link = element->qh_link;
  element->qh_rlink = NULL;
}

int add_exec(struct ex_list **ex_ptr, int do_pty, char *exec,
             struct in_addr addr, int port)
{
	struct ex_list *tmp_ptr;

	/* First, check if the port is "bound" */
	for (tmp_ptr = *ex_ptr; tmp_ptr; tmp_ptr = tmp_ptr->ex_next) {
		if (port == tmp_ptr->ex_fport &&
		    addr.s_addr == tmp_ptr->ex_addr.s_addr)
			return -1;
	}

	tmp_ptr = *ex_ptr;
	*ex_ptr = (struct ex_list *)malloc(sizeof(struct ex_list));
	(*ex_ptr)->ex_fport = port;
	(*ex_ptr)->ex_addr = addr;
	(*ex_ptr)->ex_pty = do_pty;
	(*ex_ptr)->ex_exec = (do_pty == 3) ? exec : strdup(exec);
	(*ex_ptr)->ex_next = tmp_ptr;
	return 0;
}

#ifndef HAVE_STRERROR

/*
 * For systems with no strerror
 */

extern int sys_nerr;
extern char *sys_errlist[];

char *
strerror(error)
	int error;
{
	if (error < sys_nerr)
	   return sys_errlist[error];
	else
	   return "Unknown error.";
}

#endif


#ifdef _WIN32

int
fork_exec(struct socket *so, const char *ex, int do_pty)
{
    /* not implemented */
    return 0;
}

#else

/*
 * XXX This is ugly
 * We create and bind a socket, then fork off to another
 * process, which connects to this socket, after which we
 * exec the wanted program.  If something (strange) happens,
 * the accept() call could block us forever.
 *
 * do_pty = 0   Fork/exec inetd style
 * do_pty = 1   Fork/exec using slirp.telnetd
 * do_ptr = 2   Fork/exec using pty
 */
int
fork_exec(struct socket *so, const char *ex, int do_pty)
{
	int s;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int opt;
	const char *argv[256];
	/* don't want to clobber the original */
	char *bptr;
	const char *curarg;
	int c, i, ret;
	pid_t pid;

	DEBUG_CALL("fork_exec");
	DEBUG_ARG("so = %lx", (long)so);
	DEBUG_ARG("ex = %lx", (long)ex);
	DEBUG_ARG("do_pty = %lx", (long)do_pty);

	if (do_pty == 2) {
                return 0;
	} else {
		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = INADDR_ANY;

		if ((s = qemu_socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
		    bind(s, (struct sockaddr *)&addr, addrlen) < 0 ||
		    listen(s, 1) < 0) {
#ifdef DEBUG
			printf("Error: inet socket: %s\n", strerror(errno));
#endif
			closesocket(s);

			return 0;
		}
	}

	pid = fork();
	switch(pid) {
	 case -1:
#ifdef DEBUG
		printf("Error: fork failed: %s\n", strerror(errno));
#endif
		close(s);
		return 0;

	 case 0:
                setsid();

		/* Set the DISPLAY */
                getsockname(s, (struct sockaddr *)&addr, &addrlen);
                close(s);
                /*
                 * Connect to the socket
                 * XXX If any of these fail, we're in trouble!
                 */
                s = qemu_socket(AF_INET, SOCK_STREAM, 0);
                addr.sin_addr = loopback_addr;
                do {
                    ret = connect(s, (struct sockaddr *)&addr, addrlen);
                } while (ret < 0 && errno == EINTR);

		dup2(s, 0);
		dup2(s, 1);
		dup2(s, 2);
#ifdef ANDROID
		{
			/* No getdtablesize() on Android, we will use /proc/XXX/fd/ Linux virtual FS instead */
			char proc_fd_path[256];
			sprintf(proc_fd_path, "/proc/%u/fd", (unsigned)getpid());
			DIR *proc_dir = opendir(proc_fd_path);
			if (proc_dir) {
				for (struct dirent *fd = readdir(proc_dir); fd != NULL; fd = readdir(proc_dir)) {
					if (atoi(fd->d_name) >= 3 && fd->d_name[0] != '.') /* ".." and "." will return 0 anyway */
						close(atoi(fd->d_name));
				}
				closedir(proc_dir);
			}
		}
#else
		for (s = getdtablesize() - 1; s >= 3; s--)
		   close(s);
#endif

		i = 0;
		bptr = strdup(ex); /* No need to free() this */
		if (do_pty == 1) {
			/* Setup "slirp.telnetd -x" */
			argv[i++] = "slirp.telnetd";
			argv[i++] = "-x";
			argv[i++] = bptr;
		} else
		   do {
			/* Change the string into argv[] */
			curarg = bptr;
			while (*bptr != ' ' && *bptr != (char)0)
			   bptr++;
			c = *bptr;
			*bptr++ = (char)0;
			argv[i++] = strdup(curarg);
		   } while (c);

                argv[i] = NULL;
		execvp(argv[0], (char **)argv);

		/* Ooops, failed, let's tell the user why */
        fprintf(stderr, "Error: execvp of %s failed: %s\n",
                argv[0], strerror(errno));
		close(0); close(1); close(2); /* XXX */
		exit(1);

	 default:
		slirp_warning(so->slirp, "qemu_add_child_watch(pid) not implemented");
                /*
                 * XXX this could block us...
                 * XXX Should set a timer here, and if accept() doesn't
                 * return after X seconds, declare it a failure
                 * The only reason this will block forever is if socket()
                 * of connect() fail in the child process
                 */
                do {
                    so->s = accept(s, (struct sockaddr *)&addr, &addrlen);
                } while (so->s < 0 && errno == EINTR);
                closesocket(s);
                socket_set_fast_reuse(so->s);
                opt = 1;
                qemu_setsockopt(so->s, SOL_SOCKET, SO_OOBINLINE, &opt, sizeof(int));
		qemu_set_nonblock(so->s);

		/* Append the telnet options now */
                if (so->so_m != NULL && do_pty == 1)  {
			sbappend(so, so->so_m);
                        so->so_m = NULL;
		}

		return 1;
	}
}
#endif

#ifndef HAVE_STRDUP
char *
strdup(str)
	const char *str;
{
	char *bptr;

	bptr = (char *)malloc(strlen(str)+1);
	strcpy(bptr, str);

	return bptr;
}
#endif

#endif
