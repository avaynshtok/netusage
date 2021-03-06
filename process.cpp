#include <iostream>
#include <strings.h>
#include <string>
#include <ncurses.h>
//#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pwd.h>
#include <map>

#include <vector.h>

#include "process.h"
#include "nethogs.h"

extern "C" {
  #include "lsof.h"
}

extern local_addr * local_addrs;

/* 
 * connection-inode table. takes information from /proc/net/tcp.
 * key contains source ip, source port, destination ip, destination 
 * port in format: '1.2.3.4:5-1.2.3.4:5'
 */
std::map <std::string, Process *> conn_to_proc;

extern bool needrefresh;
std::map <int, Process *> pid_to_proc;


/* this file includes:
 * - calls to inodeproc to get the pid that belongs to that inode
 */

/*
 * Initialise the global process-list with some special processes:
 * * unknown TCP traffic
 * * UDP traffic
 * * unknown IP traffic
 * We must take care this one never gets removed from the list.
 */
Process * unknowntcp; 
Process * unknownudp; 
Process * unknownip; 
ProcList * processes;

/* We're migrating to having several `unknown' processes that are added as 
 * normal processes, instead of hard-wired unknown processes.
 * This mapping maps from unknown processes descriptions to processes */
std::map <char *, Process*> unknownprocs;


void process_init () 
{
	unknowntcp = new Process ("", "unknown TCP");
	//unknownudp = new Process (0, "", "unknown UDP");
	//unknownip = new Process (0, "", "unknown IP");
	processes = new ProcList (unknowntcp, NULL);
	//processes = new ProcList (unknownudp, processes);
	//processes = new ProcList (unknownip, processes);
}

int Process::getLastPacket()
{
	int lastpacket=0;
	ConnList * curconn=connections;
	while (curconn != NULL)
	{
		assert (curconn != NULL);
		assert (curconn->getVal() != NULL);
		if (curconn->getVal()->getLastPacket() > lastpacket)
			lastpacket = curconn->getVal()->getLastPacket();
		curconn = curconn->getNext();
	}
	return lastpacket;
}

Process * listProcs()
{
	ProcList * current = processes;
	printf("procs: \n");
	while (current != NULL)
	{
		Process * currentproc = current->getVal();
		assert (currentproc != NULL);
		printf("%s\n", currentproc->name);
		/*
		if (node->pid == currentproc->pid)
			return current->getVal();
		current = current->next;
		*/
	}
	return NULL;
}

int ProcList::size ()
{
	int i=1;

	if (next != NULL)
		i += next->size();

	return i;
}

void check_all_procs ()
{
	ProcList * curproc = processes;
	while (curproc != NULL)
	{
		curproc->getVal()->check();
		curproc = curproc->getNext();
	}
}

/* 
 * Used when a new connection is encountered. Finds corresponding
 * process and adds the connection. If the connection  doesn't belong
 * to any known process, the process list is updated and a new process
 * is made. If no process can be found even then, it's added to the 
 * 'unknown' process.
 */
Process * getProcess (Connection * connection, char * devicename)
{
	//unsigned long inode = conninode[connection->refpacket->gethashstring()];
	std::string hash = std::string(connection->refpacket->gethashstring());
	//printf("hash: %s\n", hash);
	
	Process *proc = conn_to_proc[hash];
	if (proc == NULL) {
		//printf("refreshing\n");
		updateProcList();
		
		proc = conn_to_proc[hash];
		if (proc == NULL) {
			//printf("couldn't find proc after refresh\n");
			return unknowntcp;
		} else {
			//printf("proc after refresh: %s\n", proc->name);
			// this is a new proc
		}
	} else {
		//printf("got proc for %s first time: %s [%i]\n", hash.c_str(), proc->name, proc->pid);
	}
	
	fflush(stdout);
	
	proc->connections = new ConnList (connection, proc->connections);
	return proc;
	
	//return proc;
}

Process::~Process()	{
	//free (devicename);

	//std::cout << "PROC: deleting " << name << " ";
	//printf("%08X\n", this);

	// remove myself from important hashes
	//std::cout << "before" << std::endl;
	//dumpConnToProc();
	
	pid_to_proc.erase(pid);
	
	std::vector<std::string> to_delete;// = std::vector<std::string>
	
	for(std::map<std::string, Process*>::iterator it = conn_to_proc.begin(); it != conn_to_proc.end(); ++it) {
		if (it->second == this) {
			//std::cout << it->first << " points to me\n";
			to_delete.push_back(it->first);
		}
	}
	
	for (int i = 0; i < to_delete.size(); i++) {
		conn_to_proc.erase(to_delete[i]);
	}
		
	//std::cout << " after: " << std::endl;
	//dumpConnToProc();
	
	//if (DEBUG)
	free (name);
}

int concount(ConnList *connections) {
	int n = 0;
	ConnList *cl = connections;
	while (cl != NULL) {
		n++;
		cl = cl->getNext();
	}
	return n;
}
		

void updateProcList() {
	// we want
	//struct lfile *lf;
	struct lproc **slp = (struct lproc **)NULL;
	int i; //, n;
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
	
	/*
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
						//Exit(1);
					}
		}
		for (i = 0; i < Nlproc; i++) {
			slp[i] = &Lproc[i];
		}
		(void) qsort((QSORT_P *)slp, (size_t)Nlproc,
					 (size_t)sizeof(struct lproc *), comppid);
	}
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
					from_port = (char *)malloc(strlen(port));
					strcpy(from_port, port);
					
					port = lkup_port(file->li[1].p, 0, src);
					to_port = (char *)malloc(strlen(port));
					strcpy(to_port, port);
				}
				else if (strcasecmp(Lf->iproto, "UDP") == 0) {
					port = lkup_port(file->li[0].p, 1, src);
					from_port = (char *)malloc(strlen(port));
					strcpy(from_port, port);
					
					port = lkup_port(file->li[1].p, 1, src);
					to_port = (char *)malloc(strlen(port));
					strcpy(to_port, port);
				}
				
				// <from>:port-<to>:port
				char *hash = (char *)malloc(strlen(from) + strlen(to) + strlen(from_port) + strlen(to_port) + 3 + 1);
				sprintf(hash, "%s:%s-%s:%s", from, from_port, to, to_port);
				
				// find if proc exists
				int pid = Lproc[i].pid;
				Process *proc = pid_to_proc[pid];
				//char *name = Lproc[i].cmd;
				if (proc == NULL) {
					proc = new Process(file->fsdev, Lproc[i].cmd);
					proc->pid = pid;
					processes = new ProcList (proc, processes);
					pid_to_proc[pid] = proc;
					
					//printf("making %s with %i (var: %i), proc: %s (%08X)\n", hash, proc->pid, pid, proc->name, (int)proc);
				}

				//printf("associating %s with %i (var: %i), proc: %s (%08X)\n", hash, proc->pid, pid, proc->name, (int)proc);

				std::string hash_string = std::string(hash);
				conn_to_proc[hash_string] = proc;
				
				free(from_port);
				free(to_port);
				free(hash);
				
				//dumpConnToProc();
				//printf("%s %s:%s -> %s:%s %s\n", Lproc[i].cmd, from, from_port, to, to_port, file->iproto);
			}
			
			struct lfile *prev = file;
			file = file->next;
			
			prev->next = NULL;
			
			// kinda crazy to do this here, but otherwise we leak lsof alloc'd memory...
			free(prev->dev_ch);
			free(prev->nm);
			free(prev->nma);
			free(prev);
		}
		Lproc[i].file = NULL;
	}
	
	free(Namech);
//	needrefresh = true;
}

void dumpConnToProc() {
	//std::map<std::string, Process*>::iterator it = conn_to_proc.begin();
	
	std::cout << "Conn to proc:\n";
    for(std::map<std::string, Process*>::iterator it = conn_to_proc.begin(); it != conn_to_proc.end(); ++it)
    {
		std::cout << "From hash: " << it->first << " to " << it->second << "\n";
    }
	std::cout << "\n";
}

void procclean ()
{
	//delete conninode;
}

static char *sv_fmt_str(char *f) {
	char *cp;
	MALLOC_S l;
	
	l = (MALLOC_S)(strlen(f) + 1);
	if (!(cp = (char *)malloc(l))) {
		(void) fprintf(stderr,
					   "%s: can't allocate %d bytes for format: %s\n", Pn, (int)l, f);
		//Exit(1);
	}
	(void) snpf(cp, l, "%s", f);
	return(cp);
}
