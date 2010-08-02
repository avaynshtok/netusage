/*
 *  netstat.h
 *  netusage
 *
 *  Created by System Administrator on 8/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *	Stolen from: http://fxr.watson.org/fxr/source/sys/unpcb.h
 */

#include <netinet/tcp.h>
#include <sys/protosw.h>
#include <netinet/in.h>
#include <sys/sysctl.h>
#include <sys/socketvar.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <arpa/inet.h>

#include <netinet/in_pcb.h>

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <sys/un.h>
#include <sys/unpcb.h>

#include <netinet/in.h>

LIST_HEAD(unp_head, unpcb);

struct lock_object {
	   const   char *lo_name;          /* Individual lock name. */
	         u_int   lo_flags;
	         u_int   lo_data;                /* General class specific data. */
	         struct  witness *lo_witness;    /* Data for witness. */
	};

struct mtx {
	         struct lock_object      lock_object;    /* Common lock properties. */
	         volatile uintptr_t      mtx_lock;       /* Owner and flags. */
	};

struct unpcb {
	LIST_ENTRY(unpcb) unp_link;     /* glue on list of all PCBs */
	struct  socket *unp_socket;     /* pointer back to socket */
//	struct  file *unp_file;         /* back-pointer to file for gc. */
	struct  vnode *unp_vnode;       /* if associated with file */
	ino_t   unp_ino;                /* fake inode number */
	struct  unpcb *unp_conn;        /* control block of connected socket */
		struct  unp_head unp_refs;      /* referencing socket linked list */
	LIST_ENTRY(unpcb) unp_reflink;  /* link in unp_refs list */
	struct  sockaddr_un *unp_addr;  /* bound address of socket */
	int     unp_cc;                 /* copy of rcv.sb_cc */
	int     unp_mbcnt;              /* copy of rcv.sb_mbcnt */
	unp_gen_t unp_gencnt;           /* generation count of this instance */
//	short   unp_flags;              /* flags */
//	short   unp_gcflag;             /* Garbage collector flags. */
//	struct  xucred unp_peercred;    /* peer credentials, if applicable */
//	u_int   unp_refcount;
//	u_int   unp_msgcount;           /* references from message queue */
//	struct  mtx unp_mtx;            /* mutex */
};

struct  unpcb_oldest {
	struct  socket *unp_socket;     /* pointer back to socket */
	struct  vnode *unp_vnode;       /* if associated with file */
	ino_t   unp_ino;                /* fake inode number */
	struct  unpcb *unp_conn;        /* control block of connected socket */
	struct  unpcb *unp_refs;        /* referencing socket linked list */
	struct  unpcb *unp_nextref;     /* link in unp_refs list */
	struct  mbuf *unp_addr;         /* bound address of socket */
	int     unp_cc;                 /* copy of rcv.sb_cc */
	int     unp_mbcnt;              /* copy of rcv.sb_mbcnt */
};


struct xunpcb {
	size_t  xu_len;                 /* length of this structure */
	struct  unpcb *xu_unpp;         /* to help netstat, fstat */
	struct  unpcb xu_unp;           /* our information */
	union {
		struct  sockaddr_un xuu_addr;   /* our bound address */
		char    xu_dummy1[256];
	} xu_au;
#define xu_addr xu_au.xuu_addr
	union {
		struct  sockaddr_un xuu_caddr; /* their bound address */
		char    xu_dummy2[256];
	} xu_cau;
#define xu_caddr xu_cau.xuu_caddr
	struct  xsocket xu_socket;
	u_quad_t        xu_alignment_hack;
};






struct xunpcb64_list_entry {
	u_int64_t   le_next;
	u_int64_t   le_prev;
};

struct xunpcb64 {
	u_int32_t                       xu_len;         /* length of this structure */
	u_int64_t                       xu_unpp;        /* to help netstat, fstat */
	struct xunpcb64_list_entry      xunp_link;      /* glue on list of all PCBs */
	u_int64_t                       xunp_socket;    /* pointer back to socket */
	u_int64_t                       xunp_vnode;     /* if associated with file */
	u_int64_t                       xunp_ino;       /* fake inode number */
	u_int64_t                       xunp_conn;      /* control block of connected socket */
	u_int64_t                       xunp_refs;      /* referencing socket linked list */
	struct xunpcb64_list_entry      xunp_reflink;   /* link in unp_refs list */
	int                             xunp_cc;                /* copy of rcv.sb_cc */
	int                             xunp_mbcnt;     /* copy of rcv.sb_mbcnt */
	unp_gen_t                       xunp_gencnt;    /* generation count of this instance */
	int                             xunp_flags;     /* flags */
	union {
		struct sockaddr_un              xuu_addr;
		char                            xu_dummy1[256];
	}                               xu_au;          /* our bound address */
#define xunp_addr xu_au.xuu_addr
	union {
		struct sockaddr_un              xuu_caddr;
		char                            xu_dummy2[256];
	}                               xu_cau;         /* their bound address */
#define xunp_caddr xu_cau.xuu_caddr
	struct xsocket64        xu_socket;
};
