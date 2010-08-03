/*
 *  lsof.cpp
 *  netusage
 *
 *  Created by Anton Vaynshtok on 8/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "lsof.h"
#define	MALLOC_S	size_t

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>

static void
fill_portmap()
{
  char buf[128], *cp, *nm;
  CLIENT *c;
  int h, port, pr;
  MALLOC_S nl;
  struct pmaplist *p = (struct pmaplist *)NULL;
  struct porttab *pt;
  struct rpcent *r;
  struct TIMEVAL_LSOF tm;
	
//#if !defined(CAN_USE_CLNT_CREATE)
  struct hostent *he;
  struct sockaddr_in ia;
  int s = RPC_ANYSOCK;
//#endif  /* !defined(CAN_USE_CLNT_CREATE) */
	
	/*
	 * Construct structures for communicating with the portmapper.
	 */

//#if !defined(CAN_USE_CLNT_CREATE)
  //zeromem(&ia, sizeof(ia));
  bzero(&ia, sizeof(sockaddr_in));
  ia.sin_family = AF_INET;
	
	if ((he = gethostbyname("localhost"))) {
		MEMMOVE((caddr_t)&ia.sin_addr, he->h_addr, he->h_length);
	}
	else {
		perror("gethostbyname");
	}

  ia.sin_port = htons(PMAPPORT);
//#endif  /* !defined(CAN_USE_CLNT_CREATE) */
	
	
  tm.tv_sec = 60;
  tm.tv_usec = 0;

	if (!(c = clnt_create("localhost", PMAPPROG, PMAPVERS, "tcp"))) {
//	if (!(c = clnttcp_create(&ia, PMAPPROG, PMAPVERS, &s, 50, 500))) {
		perror("clnt_create");
		printf("pmapprog: %i", PMAPPROG);
	}

}

int main (int argc, const char * argv[]) {
	fill_portmap();
}

