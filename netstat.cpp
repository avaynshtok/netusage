/*
 *  netstat.cpp
 *  netusage
 *
 *  Created by System Administrator on 8/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 * 
 *	Mostly from: http://src.gnu-darwin.org/src/usr.bin/netstat/unix.c
 */
#include <stdio.h>
#include <stdlib.h>

#include "netstat.h"

#include <errno.h>

static	const char *const socktype[] =
{ "#0", "stream", "dgram", "raw", "rdm", "seqpacket" };


static int
pcblist_sysctl(int type, char **bufp)
{
	char 	*buf;
	size_t	len;
	char mibvar[sizeof "net.local.seqpacket.pcblist"];
	
	sprintf(mibvar, "net.local.%s.pcblist", socktype[type]);
	
	len = 0;
	if (sysctlbyname(mibvar, 0, &len, 0, 0) < 0) {
		if (errno != ENOENT) {
//			perror("sysctl: %s", mibvar);
			perror("sysctlbyname len");
		}
		return (-1);
	}
	if ((buf = (char*)malloc(len)) == 0) {
		//perror("malloc %lu bytes", (u_long)len);
		perror("malloc");
		return (-2);
	}
	if (sysctlbyname(mibvar, buf, &len, 0, 0) < 0) {
		//perror("sysctl: %s", mibvar);
		perror("sysctlbyname get");
		free(buf);
		return (-2);
	}
	*bufp = buf;
	return (0);
}


static void unixdomainpr(struct xunpcb *xunp, struct xsocket *so)
{
	struct unpcb *unp;
	struct sockaddr_un *sa;
	static int first = 1;
	
	unp = &xunp->xu_unp;
	if (unp->unp_addr)
		sa = &xunp->xu_addr;
	else
		sa = (struct sockaddr_un *)0;
	
	if (first) {
		printf("Active UNIX domain sockets\n");
		printf(
			   "%-8.8s %-6.6s %-6.6s %-6.6s %8.8s %8.8s %8.8s %8.8s Addr\n",
			   "Address", "Type", "Recv-Q", "Send-Q",
			   "Inode", "Conn", "Refs", "Nextref");
		first = 0;
	}
	printf("%8lx %-6.6s %6u %6u %8lx %8lx %8lx %8lx",
	       (long)so->so_pcb, socktype[so->so_type], so->so_rcv.sb_cc,
	       so->so_snd.sb_cc,
	       (long)unp->unp_vnode, (long)unp->unp_conn,
	       (long)LIST_FIRST(&unp->unp_refs), (long)LIST_NEXT(unp, unp_reflink));
	/*
	if (sa)
		printf(" %.*s",
			   (int)(sa->sun_len - offsetof(struct sockaddr_un, sun_path)),
			   sa->sun_path);
	*/
	putchar('\n');
}

//void
//unixpr(u_long count_off, u_long gencnt_off, u_long dhead_off, u_long shead_off)
int main (int argc, const char * argv[])
{
	char 	*buf;
	int	ret, type;
	struct	xsocket *so;
	struct	xunpgen *xug, *oxug;
	struct	xunpcb *xunp;
	
	for (type = SOCK_STREAM; type <= SOCK_SEQPACKET; type++) {
		//		if (live)
		ret = pcblist_sysctl(type, &buf);
		//		else
		//			ret = pcblist_kvm(count_off, gencnt_off,
		//							  type == SOCK_STREAM ? shead_off :
		//							  (type == SOCK_DGRAM ? dhead_off : 0), &buf);
		if (ret == -1)
			continue;
		if (ret < 0)
			return 0;
		
		oxug = xug = (struct xunpgen *)buf;
		for (xug = (struct xunpgen *)((char *)xug + xug->xug_len);
		     xug->xug_len > sizeof(struct xunpgen);
		     xug = (struct xunpgen *)((char *)xug + xug->xug_len)) {
			xunp = (struct xunpcb *)xug;
			so = &xunp->xu_socket;
			
			/* Ignore PCBs which were freed during copyout. */
			if (xunp->xu_unp.unp_gencnt > oxug->xug_gen)
				continue;
			unixdomainpr(xunp, so);
		}
		if (xug != oxug && xug->xug_gen != oxug->xug_gen) {
			if (oxug->xug_count > xug->xug_count) {
				printf("Some %s sockets may have been deleted.\n",
				       socktype[type]);
			} else if (oxug->xug_count < xug->xug_count) {
				printf("Some %s sockets may have been created.\n",
					   socktype[type]);
			} else {
				printf("Some %s sockets may have been created or deleted",
					   socktype[type]);
			}
		}
		free(buf);
	}
	
	return 0;
}