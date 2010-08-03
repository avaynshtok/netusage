#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <nfs/rpcv2.h>

#include "lsof.h"

_PROTOTYPE(extern char *lkup_port,(int p, int pr, int src));
_PROTOTYPE(static char *sv_fmt_str,(char *f));

int main (int argc, const char * argv[]) {
	// we want
	struct lfile *lf;
	struct lproc **slp = (struct lproc **)NULL;
	int i, n;
	int sp = 0;
	MALLOC_S len;
	char options[128];
	
	// gather_proc_info wants
	Namech = (char *)malloc(MAXPATHLEN + 1);
	Namechl = (size_t)(MAXPATHLEN + 1);
	
	// internet only, no convert
	Fnet = 1;
	FnetTy = 0;
	Selall = 0;
	Selflags |= SELNET;
	Fsv &= (unsigned char)~FSV_NI;
	//Selflags &= (unsigned char)~SELNA;
	Fport = 0;
	Fhost = 0;
	
	gather_proc_info();
	
	/*
	 * Define the size and offset print formats.
	 */
  (void) snpf(options, sizeof(options), "%%%su", INODEPSPEC);
  InodeFmt_d = sv_fmt_str(options);
  (void) snpf(options, sizeof(options), "%%#%sx", INODEPSPEC);
  InodeFmt_x = sv_fmt_str(options);
  (void) snpf(options, sizeof(options), "0t%%%su", SZOFFPSPEC);
  SzOffFmt_0t = sv_fmt_str(options);
  (void) snpf(options, sizeof(options), "%%%su", SZOFFPSPEC);
  SzOffFmt_d = sv_fmt_str(options);
  (void) snpf(options, sizeof(options), "%%*%su", SZOFFPSPEC);
  SzOffFmt_dv = sv_fmt_str(options);
  (void) snpf(options, sizeof(options), "%%#%sx", SZOFFPSPEC);
  SzOffFmt_x = sv_fmt_str(options);
	
	if (Nlproc > 1) {
    if (Nlproc > sp) {
			len = (MALLOC_S)(Nlproc * sizeof(struct lproc *));
			sp = Nlproc;
			if (!slp)
				slp = (struct lproc **)malloc(len);
			else
				slp = (struct lproc **)realloc((MALLOC_P *)slp, len);
			if (!slp) {
				(void) fprintf(stderr,
											 "%s: no space for %d sort pointers\n", Pn, Nlproc);
				Exit(1);
			}
    }
    for (i = 0; i < Nlproc; i++) {
			slp[i] = &Lproc[i];
    }
    (void) qsort((QSORT_P *)slp, (size_t)Nlproc,
								 (size_t)sizeof(struct lproc *), comppid);
	}
	
	/*
	 //for (lf = Lf, print_init(); PrPass < 2; PrPass++) {
	 lf = Lf;
	 print_init();
	 PrPass = 1;
	 for (i = n = 0; i < Nlproc; i++) {
	 Lp = (Nlproc > 1) ? slp[i] : &Lproc[i];
	 if (Lp->pss) {
	 if (print_proc())
	 n++;
	 }
	 if (RptTm && PrPass)
	 (void) free_lproc(Lp);
	 }
	 //}
	 */
	for (i = 0; i < Nlproc; i++) {
		//printf("\ngot pid: %i, %s  ", Lproc[i].pid, Lproc[i].cmd);
		struct lfile *file = Lproc[i].file;
		
		while (file != NULL) {
			//printf("file: %c", file->fsdev);
			if (file->li[0].af && file->li[1].af) {
				char *from = gethostnm((unsigned char *)&file->li[0].ia, file->li[0].af);
				char *to = gethostnm((unsigned char *)&file->li[1].ia, file->li[1].af);
				
				int src = 0;
				
				char *port;
				char *from_port, *to_port;
				if (strcasecmp(file->iproto, "TCP") == 0) {
					port = lkup_port(file->li[0].p, 0, src);
					from_port = malloc(strlen(port));
					strcpy(from_port, port);
					
					port = lkup_port(file->li[1].p, 0, src);
					to_port = malloc(strlen(port));
					strcpy(to_port, port);
				}
		    else if (strcasecmp(Lf->iproto, "UDP") == 0) {
					port = lkup_port(file->li[0].p, 1, src);
					from_port = malloc(strlen(port));
					strcpy(from_port, port);
					
					port = lkup_port(file->li[1].p, 1, src);
					to_port = malloc(strlen(port));
					strcpy(to_port, port);
				}
				
				printf("%s %s:%s -> %s:%s %s\n", Lproc[i].cmd, from, from_port, to, to_port, file->iproto);
			}
			file = file->next;
		}
	}
	
	return 0;
}

static char *
sv_fmt_str(f)
char *f;      /* format string */
{
  char *cp;
  MALLOC_S l;
	
  l = (MALLOC_S)(strlen(f) + 1);
  if (!(cp = (char *)malloc(l))) {
		(void) fprintf(stderr,
									 "%s: can't allocate %d bytes for format: %s\n", Pn, (int)l, f);
		Exit(1);
  }
  (void) snpf(cp, l, "%s", f);
  return(cp);
}

