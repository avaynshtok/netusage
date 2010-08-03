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
#include <sys/sysctl.h>

//#include "netstat.h"

#include <sys/socketvar.h>
#include "unpcb.h"

#include <errno.h>
#include <sys/socket.h>

static	const char *const socktype[] =
{ "#0", "stream", "dgram", "raw", "rdm", "seqpacket" };


static int
pcblist_sysctl(int type, char **bufp)
{
	char 	*buf;
	size_t	len;
	char mibvar[sizeof "net.local.seqpacket.pcblist"];
	
	sprintf(mibvar, "net.local.%s.pcblist64", socktype[type]);
	
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
	
	
	printf("pcblist len: %i\n", len);
	//exit(0);
	
	*bufp = buf;
	return (0);
}

static void unixdomainpr(struct xunpcb64 *xunp, struct xsocket64 *so)
{
	struct unpcb *unp;
//	struct unpcb_compat *unp;
	struct sockaddr_un *sa;
	static int first = 1;
	
	/*
	unp = (unpcb*)xunp->xu_unpp;
	if (unp->unp_addr)
		sa = &xunp->xu_addr;
	else
		sa = (struct sockaddr_un *)0;
	*/
	sa = &xunp->xu_addr;
	
	if (first) {
		printf("Active UNIX domain sockets\n");
		printf(
			   "%-16.16s %-6.6s %-6.6s %-6.6s %16.16s %16.16s %16.16s %16.16s Addr\n",
			   "Address", "Type", "Recv-Q", "Send-Q",
			   "Inode", "Conn", "Refs", "Nextref");
		first = 0;
	}
	/*
	//printf("%8lx %-6.6s %6u %6u %8lx %8lx %8lx %8lx",
	//      (long)so->so_pcb, socktype[so->so_type], so->so_rcv.sb_cc,
	//       so->so_snd.sb_cc,
	//       (long)unp->unp_vnode, (long)unp->unp_conn,
	//       (long)LIST_FIRST(&unp->unp_refs), (long)LIST_NEXT(unp, unp_reflink));
	printf("%8lx %-6.6s %6u",  (long)so->so_pcb, socktype[so->so_type], so->so_rcv.sb_cc);
	 */
	printf("%16lx %-6.6s %6u %6u %16lx %16lx %16lx %16lx",
	       (long)xunp->xu_unpp, socktype[so->so_type], so->so_rcv.sb_cc,
	       so->so_snd.sb_cc,
	       (long)xunp->xunp_vnode, (long)xunp->xunp_conn,
	       (long)xunp->xunp_refs, (long)xunp->xunp_reflink.le_next);
	
	
	putchar('\n');
}

//void
//unixpr(u_long count_off, u_long gencnt_off, u_long dhead_off, u_long shead_off)
int ns_main (int argc, const char * argv[])
{
	char 	*buf;
	int	ret, type;
	struct	xsocket64 *so;
	struct	xunpgen *xug, *oxug;
	struct	xunpcb64 *xunp;
	
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
		
		int tlen = 0;
		int count = 0;
		oxug = xug = (struct xunpgen *)buf;
		for (xug = (struct xunpgen *)((char *)xug + xug->xug_len);
		     xug->xug_len > sizeof(struct xunpgen);
		     xug = (struct xunpgen *)((char *)xug + xug->xug_len))
		{
			xunp = (struct xunpcb64 *)xug;
			so = &xunp->xu_socket;
			
			count+=1;
			tlen += xunp->xu_len;
			
//			printf("len: %u\n", xunp->xu_len);
//			printf("size: %lu\n", sizeof(struct xunpcb));
//			continue;
			
			/* Ignore PCBs which were freed during copyout. */
			if (xunp->xunp_gencnt > oxug->xug_gen)
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
		printf("tlen: %i, count: %i\n", tlen, count);
		free(buf);
	}
	
	return 0;
}