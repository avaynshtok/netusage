/*
 *  getcons.cpp
 *  netusage
 *
 *  Created by System Administrator on 8/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "getcons.h"
#include <netinet/tcp.h>
#include <sys/protosw.h>
#include <netinet/in.h>
#include <sys/sysctl.h>
#include <sys/socketvar.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <arpa/inet.h>

//TAILQ_HEAD(netinfohead, netinfo) netcb = TAILQ_HEAD_INITIALIZER(netcb);
//static	int aflag = 0;

struct tcp_index {
	//struct asn_oid	index;
	struct xtcpcb	*tp;
};

int main (int argc, const char * argv[]) {
/*	struct netinfo *p;
	int idx;
	struct xinpgen *inpg;
	char *cur, *end;
	struct inpcb *inpcb;
	struct xinpcb *xip = NULL;
	struct xtcpcb *xtp = NULL;
	int plen;
	size_t lsz;
*/
	struct xinpgen *ptr;
	struct xinpgen *xinpgen;
	struct xtcpcb *tp;
	struct tcp_index *tcpoids;
	in_addr_t inaddr1, inaddr2;
	
	size_t len = 0;
	if (sysctlbyname("net.inet.tcp.pcblist", NULL, &len, NULL, 0) == -1) {
		perror("sysctlbyname len");
		return (-1);
	}
	
	xinpgen = (struct xinpgen *) malloc(len);
	
	if (sysctlbyname("net.inet.tcp.pcblist", xinpgen, &len, NULL, 0) == -1) {
		perror("sysctlbyname fetch");
		return (-1);
	}
		
	u_int tcp_count = 0;
	u_int tcp_total = 0;
	for (ptr = (struct xinpgen *)(void *)((char *)xinpgen + xinpgen->xig_len);
	     ptr->xig_len > sizeof(struct xinpgen);
		 ptr = (struct xinpgen *)(void *)((char *)ptr + ptr->xig_len)) {
		tp = (struct xtcpcb *)ptr;
		if (tp->xt_inp.inp_gencnt > xinpgen->xig_gen ||
		    (tp->xt_inp.inp_vflag & INP_IPV4) == 0)
			continue;
		
		tcp_total++;
		if (tp->xt_tp.t_state == TCPS_ESTABLISHED ||
		    tp->xt_tp.t_state == TCPS_CLOSE_WAIT)
			tcp_count++;
	}	
	
	printf("tcp total: %i", tcp_total);
	
	tcpoids = (struct tcp_index *) malloc(tcp_total * sizeof(tcpoids[0]));
	
		
	//oid = tcpoids;
//	char *source;
//	char *dest;
	for (ptr = (struct xinpgen *)(void *)((char *)xinpgen + xinpgen->xig_len);
	     ptr->xig_len > sizeof(struct xinpgen);
		 ptr = (struct xinpgen *)(void *)((char *)ptr + ptr->xig_len))
	{
		tp = (struct xtcpcb *)ptr;
		if (tp->xt_inp.inp_gencnt > xinpgen->xig_gen ||
		    (tp->xt_inp.inp_vflag & INP_IPV4) == 0)
			continue;
		//oid->tp = tp;
		//oid->index.len = 10;
		inaddr1 = ntohl(tp->xt_inp.inp_faddr.s_addr);
		char *source = inet_ntoa(*((struct in_addr *)&inaddr1)); /* cast x as a struct in_addr */
		printf("%s -> ", source);
		
		inaddr2 = ntohl(tp->xt_inp.inp_laddr.s_addr);
		char *dest = inet_ntoa(*((struct in_addr *)&inaddr2)); /* cast x as a struct in_addr */
		//printf("%i -> %i\n", inaddr1, inaddr2);
		printf(" %s\n", dest);
													
	}
		
	
	
	return 0;
}