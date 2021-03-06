/* NetHogs console UI */
#include <ncurses.h>
#include <pcap/pcap.h>

#include <string>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>

#include <ncurses.h>
#include "nethogs.h"
#include "process.h"
#include "decpcap.h"

#include "cui.h"

extern void updateGUI(NHLine ** lines, int numprocs);
extern bool in_ui;

std::string * caption;
//extern char [] version;
const char version[] = "cool version"; //" version " VERSION "." SUBVERSION "." MINORVERSION;
extern ProcList * processes;
time_t curtime;

extern Process * unknowntcp;
extern Process * unknownudp;
extern Process * unknownip;

// sort on sent or received?
bool sortRecv = true;
// viewMode: kb/s or total
int VIEWMODE_KBPS = 0;
int VIEWMODE_TOTAL_KB = 1;
int VIEWMODE_TOTAL_B = 2;
int VIEWMODE_TOTAL_MB = 3;
int viewMode = VIEWMODE_KBPS;
int nViewModes = 4;

char * uid2username (uid_t uid)
{
	struct passwd * pwd = NULL;
	/* getpwuid() allocates space for this itself,
	 * which we shouldn't free */
	pwd = getpwuid(uid);

	if (pwd == NULL)
	{
		assert(false);
		return strdup ("unlisted");
	} else {
		return strdup(pwd->pw_name);
	}
}


void NHLine::show (int row)
{
	assert (m_pid >= 0);
	assert (m_pid <= 100000);

	if (DEBUG || tracemode)
	{
		std::cout << m_name << '/' << m_pid << '/' << m_uid << "\t" << sent_value << "\t" << recv_value << std::endl;
		return;
	}

	mvprintw (3+row, 0, "%d", m_pid);
	char * username = uid2username(m_uid);
	mvprintw (3+row, 6, "%s", username);
	free (username);
	if (strlen (m_name) > PROGNAME_WIDTH) {
		// truncate oversized names
		char * tmp = strdup(m_name);
		char * start = tmp + strlen (m_name) - PROGNAME_WIDTH;
		start[0] = '.';
		start[1] = '.';
		mvprintw (3+row, 6 + 9, "%s", start);
		free (tmp);
	} else {
		mvprintw (3+row, 6 + 9, "%s", m_name);
	}
	mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2, "%s", "");
	mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6, "%10.3f", sent_value);
	mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6 + 9 + 3, "%10.3f", recv_value);
	if (viewMode == VIEWMODE_KBPS)
	{
		mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6 + 9 + 3 + 11, "KB/sec");
	}
	else if (viewMode == VIEWMODE_TOTAL_MB)
	{
		mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6 + 9 + 3 + 11, "MB    ");
	}
	else if (viewMode == VIEWMODE_TOTAL_KB)
	{
		mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6 + 9 + 3 + 11, "KB    ");
	}
	else if (viewMode == VIEWMODE_TOTAL_B)
	{
		mvprintw (3+row, 6 + 9 + PROGNAME_WIDTH + 2 + 6 + 9 + 3 + 11, "B     ");
	}
}

int GreatestFirst (const void * ma, const void * mb)
{
	NHLine ** pa = (NHLine **)ma;
	NHLine ** pb = (NHLine **)mb;
	NHLine * a = *pa;
	NHLine * b = *pb;
	double aValue;
	if (sortRecv)
	{
		aValue = a->recv_value;
	}
	else
	{
		aValue = a->sent_value;
	}

	double bValue;
	if (sortRecv)
	{
		bValue = b->recv_value;
	}
	else
	{
		bValue = b->sent_value;
	}

	if (aValue > bValue)
	{
		return -1;
	}
	if (aValue == bValue)
	{
		return 0;
	}
	return 1;
}

void init_ui ()
{
	WINDOW * screen = initscr();
	raw();
	noecho();
	cbreak();
	nodelay(screen, TRUE);
	caption = new std::string ("NetHogs");
	caption->append(version);
	//caption->append(", running at ");
}

void exit_ui ()
{
	clear();
	endwin();
	delete caption;
}

void ui_tick ()
{
	switch (getch()) {
		case 'q':
			/* quit */
			quit_cb(0);
			break;
		case 's':
			/* sort on 'sent' */
			sortRecv = false;
			break;
		case 'r':
			/* sort on 'received' */
			sortRecv = true;
			break;
		case 'm':
			/* switch mode: total vs kb/s */
			viewMode = (viewMode + 1) % nViewModes;
			break;
	}
}

float tomb (u_int32_t bytes)
{
	return ((double)bytes) / 1024 / 1024;
}
float tokb (u_int32_t bytes)
{
	return ((double)bytes) / 1024;
}
float tokbps (u_int32_t bytes)
{
	return (((double)bytes) / PERIOD) / 1024;
}

/** Get the kb/s values for this process */
void getkbps (Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;

	/* walk though all this process's connections, and sum
	 * them up */
	ConnList * curconn = curproc->connections;
	ConnList * previous = NULL;
	while (curconn != NULL)
	{
		if (curconn->getVal()->getLastPacket() <= curtime - CONNTIMEOUT)
		{
			/* stalled connection, remove. */
			ConnList * todelete = curconn;
			Connection * conn_todelete = curconn->getVal();
			curconn = curconn->getNext();
			if (todelete == curproc->connections)
				curproc->connections = curconn;
			if (previous != NULL)
				previous->setNext(curconn);
			delete (todelete);
			delete (conn_todelete);
		}
		else
		{
			u_int32_t sent = 0, recv = 0;
			curconn->getVal()->sumanddel(curtime, &recv, &sent);
			sum_sent += sent;
			sum_recv += recv;
			previous = curconn;
			curconn = curconn->getNext();
		}
	}
	*recvd = tokbps(sum_recv);
	*sent = tokbps(sum_sent);
}

/** get total values for this process */
void gettotal(Process * curproc, u_int32_t * recvd, u_int32_t * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	ConnList * curconn = curproc->connections;
	while (curconn != NULL)
	{
		Connection * conn = curconn->getVal();
		sum_sent += conn->sumSent;
		sum_recv += conn->sumRecv;
		curconn = curconn->getNext();
	}
	//std::cout << "Sum sent: " << sum_sent << std::endl;
	//std::cout << "Sum recv: " << sum_recv << std::endl;
	*recvd = sum_recv;
	*sent = sum_sent;
}

void gettotalmb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	gettotal(curproc, &sum_recv, &sum_sent);
	*recvd = tomb(sum_recv);
	*sent = tomb(sum_sent);
}

/** get total values for this process */
void gettotalkb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	gettotal(curproc, &sum_recv, &sum_sent);
	*recvd = tokb(sum_recv);
	*sent = tokb(sum_sent);
}

void gettotalb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	gettotal(curproc, &sum_recv, &sum_sent);
	//std::cout << "Total sent: " << sum_sent << std::endl;
	*sent = sum_sent;
	*recvd = sum_recv;
}

// Display all processes and relevant network traffic using show function
void do_refresh()
{
	curtime = time(NULL);
	
	//refreshconninode();
	if (in_ui || DEBUG || tracemode)
	{
		std::cout << "\nRefreshing:\n";
	}
	else
	{
		clear();
		mvprintw (0, 0, "%s", caption->c_str());
		attron(A_REVERSE);
		mvprintw (2, 0, "  PID USER     PROGRAM                      DEV        SENT      RECEIVED       ");
		attroff(A_REVERSE);
	}
	ProcList * curproc = processes;
	ProcList * previousproc = NULL;
	int nproc = processes->size();
	/* initialise to null pointers */
	NHLine * lines [nproc];
	int n = 0, i = 0;
	double sent_global = 0;
	double recv_global = 0;

#ifndef NDEBUG
	// initialise to null pointers
	for (int i = 0; i < nproc; i++)
		lines[i] = NULL;
#endif

	while (curproc != NULL)
	{
		int lastPacket = curproc->getVal()->getLastPacket();
		int age = curtime - lastPacket;
		
		//std::cout << "checking out: " << curproc->getVal()->name << " age: " << age << "\n";
		
		// walk though its connections, summing up their data, and
		// throwing away connections that haven't received a package
		// in the last PROCESSTIMEOUT seconds.
		assert (curproc != NULL);
		assert (curproc->getVal() != NULL);
		assert (nproc == processes->size());

		/* remove timed-out processes (unless it's one of the the unknown process) */
		if ((curproc->getVal()->getLastPacket() + PROCESSTIMEOUT <= curtime)
				&& (curproc->getVal() != unknowntcp)
				&& (curproc->getVal() != unknownudp)
				&& (curproc->getVal() != unknownip))
		{
			
			ProcList * todelete = curproc;
			Process * p_todelete = curproc->getVal();
			//if (DEBUG)
				std::cout << "PROC: Deleting process " << p_todelete->name << "\n";
			
			if (previousproc)
			{
				previousproc->next = curproc->next;
				curproc = curproc->next;
			} else {
				processes = curproc->getNext();
				curproc = processes;
			}
			delete todelete;
			delete p_todelete;
			nproc--;
			//continue;
			
		}
		else
		{
			// add a non-timed-out process to the list of stuff to show
			float value_sent = 0,
				value_recv = 0;

			if (viewMode == VIEWMODE_KBPS)
			{
				//std::cout << "kbps viemode" << std::endl;
				getkbps (curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_KB)
			{
				//std::cout << "total viemode" << std::endl;
				gettotalkb(curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_MB)
			{
				//std::cout << "total viemode" << std::endl;
				gettotalmb(curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_B)
			{
				//std::cout << "total viemode" << std::endl;
				gettotalb(curproc->getVal(), &value_recv, &value_sent);
			}
			else
			{
				forceExit("Invalid viewmode");
			}
			uid_t uid = curproc->getVal()->getUid();
#ifndef NDEBUG
			struct passwd * pwuid = getpwuid(uid);
			assert (pwuid != NULL);
			// value returned by pwuid should not be freed, according to
			// Petr Uzel.
			//free (pwuid);
#endif
			assert (curproc->getVal()->pid >= 0);
			assert (n < nproc);

			lines[n] = new NHLine (curproc->getVal()->name, value_recv, value_sent,
					curproc->getVal()->pid, uid, curproc->getVal()->devicename);
			previousproc = curproc;
			curproc = curproc->next;
			n++;
#ifndef NDEBUG
			assert (nproc == processes->size());
			if (curproc == NULL)
				assert (n-1 < nproc);
			else
				assert (n < nproc);
#endif
		}
	}

	/* sort the accumulated lines */
	qsort (lines, nproc, sizeof(NHLine *), GreatestFirst);
	
	// callback to objectivec land
	updateGUI(lines, nproc);

	/* print them */
	for (i=0; i<nproc; i++)
	{
		if (!in_ui) {
		  lines[i]->show(i);
		}
		recv_global += lines[i]->recv_value;
		sent_global += lines[i]->sent_value;
		delete lines[i];
	}
	if (tracemode || DEBUG) {
		/* print the 'unknown' connections, for debugging */
		ConnList * curr_unknownconn = unknowntcp->connections;
		while (curr_unknownconn != NULL) {
			std::cout << "Unknown connection: " <<
				curr_unknownconn->getVal()->refpacket->gethashstring() << std::endl;

			curr_unknownconn = curr_unknownconn->getNext();
		}
	}
	
	time_t seconds = time(NULL);

	if (!in_ui && (!tracemode) && (!DEBUG)){
		attron(A_REVERSE);
		mvprintw (3+1+i, 0, "  TOTAL                %i                  %10.3f  %10.3f", seconds, sent_global, recv_global);
		if (viewMode == VIEWMODE_KBPS)
		{
			mvprintw (3+1+i, 73, "KB/sec ");
		} else if (viewMode == VIEWMODE_TOTAL_B) {
			mvprintw (3+1+i, 73, "B      ");
		} else if (viewMode == VIEWMODE_TOTAL_KB) {
			mvprintw (3+1+i, 73, "KB     ");
		} else if (viewMode == VIEWMODE_TOTAL_MB) {
			mvprintw (3+1+i, 73, "MB     ");
		}
		attroff(A_REVERSE);
		mvprintw (4+1+i, 0, "");
		refresh();
	}
	
}


