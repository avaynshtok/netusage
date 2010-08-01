/* nethogs.cpp */

#include "nethogs.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <string.h>

#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "cui.h"

extern "C" {
	#include "decpcap.h"
}

#include "packet.h"
#include "connection.h"
#include "process.h"
#include "refresh.h"

extern Process * unknownudp; 

unsigned refreshdelay = 1;
bool tracemode = false;
bool needrefresh = true;
//packet_type packettype = packet_ethernet;
//dp_link_type linktype = dp_link_ethernet;
const char version[] = "cool version";// " version " VERSION "." SUBVERSION "." MINORVERSION;

char * currentdevice = NULL;

timeval curtime;

bool local_addr::contains (const in_addr_t & n_addr) {
	if ((sa_family == AF_INET)
	    && (n_addr == addr))
		return true;
	if (next == NULL)
		return false;
	return next->contains(n_addr);
}

bool local_addr::contains(const struct in6_addr & n_addr) {
	if (sa_family == AF_INET6)
	{
		/*
		if (DEBUG) {
			char addy [50];
			std::cerr << "Comparing: ";
			inet_ntop (AF_INET6, &n_addr, addy, 49);
			std::cerr << addy << " and ";
			inet_ntop (AF_INET6, &addr6, addy, 49);
			std::cerr << addy << std::endl;
		}
		*/
		//if (addr6.s6_addr == n_addr.s6_addr)
		if (memcmp (&addr6, &n_addr, sizeof(struct in6_addr)) == 0)
		{
			if (DEBUG)
				std::cerr << "Match!" << std::endl;
			return true;
		}
	}
	if (next == NULL)
		return false;
	return next->contains(n_addr);
}

struct dpargs {
	int sa_family;
	in_addr ip_src;
	in_addr ip_dst;
	in6_addr ip6_src;
	in6_addr ip6_dst;
};

int process_tcp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
	std::cout << "Processing TCP packet with len " << header->len << std::endl;
	struct dpargs * args = (struct dpargs *) userdata;
	struct tcphdr * tcp = (struct tcphdr *) m_packet;

	curtime = header->ts;

	/* TODO get info from userdata, then call getPacket */
	Packet * packet; 
	switch (args->sa_family)
	{
		case (AF_INET):
			packet = new Packet (args->ip_src, ntohs(tcp->th_sport), args->ip_dst, ntohs(tcp->th_dport), header->len, header->ts);
			break;
		case (AF_INET6):
			packet = new Packet (args->ip6_src, ntohs(tcp->th_sport), args->ip6_dst, ntohs(tcp->th_dport), header->len, header->ts);
			break;
	}

	//if (DEBUG)
	//	std::cout << "Got packet from " << packet->gethashstring() << std::endl;

	Connection * connection = findConnection(packet);

	if (connection != NULL)
	{
		/* add packet to the connection */
		connection->add(packet);
	} else {
		/* else: unknown connection, create new */
		connection = new Connection (packet);
		getProcess(connection, currentdevice);
	}
	delete packet;

	if (needrefresh)
	{
		do_refresh();
		needrefresh = false;
	}

	/* we're done now. */
	return true;
}

int process_udp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
	struct dpargs * args = (struct dpargs *) userdata;
	//struct tcphdr * tcp = (struct tcphdr *) m_packet;
	struct udphdr * udp = (struct udphdr *) m_packet;

	curtime = header->ts;

	/* TODO get info from userdata, then call getPacket */
	Packet * packet; 
	switch (args->sa_family)
	{
		case (AF_INET):
			packet = new Packet (args->ip_src, ntohs(udp->uh_sport), args->ip_dst, ntohs(udp->uh_dport), header->len, header->ts);
			break;
		case (AF_INET6):
			packet = new Packet (args->ip6_src, ntohs(udp->uh_sport), args->ip6_dst, ntohs(udp->uh_dport), header->len, header->ts);
			break;
	}

	//if (DEBUG)
	//	std::cout << "Got packet from " << packet->gethashstring() << std::endl;

	Connection * connection = findConnection(packet);

	if (connection != NULL)
	{
		/* add packet to the connection */
		connection->add(packet);
	} else {
		/* else: unknown connection, create new */
		connection = new Connection (packet);
		getProcess(connection, currentdevice);
	}
	delete packet;

	if (needrefresh)
	{
		do_refresh();
		needrefresh = false;
	}

	/* we're done now. */
	return true;
}

int process_ip (u_char * userdata, const dp_header * header, const u_char * m_packet) {
	struct dpargs * args = (struct dpargs *) userdata;
	struct ip * ip = (struct ip *) m_packet;
	args->sa_family = AF_INET;
	args->ip_src = ip->ip_src;
	args->ip_dst = ip->ip_dst;

	/* we're not done yet - also parse tcp :) */
	return false;
}

int process_ip6 (u_char * userdata, const dp_header * header, const u_char * m_packet) {
	struct dpargs * args = (struct dpargs *) userdata;
	const struct ip6_hdr * ip6 = (struct ip6_hdr *) m_packet;
	args->sa_family = AF_INET6;
	args->ip6_src = ip6->ip6_src;
	args->ip6_dst = ip6->ip6_dst;

	/* we're not done yet - also parse tcp :) */
	return false;
}

void quit_cb (int i)
{
	procclean();
	if ((!tracemode) && (!DEBUG))
		exit_ui();
	exit(0);
}

void forceExit(const char *msg)
{
	forceExit (msg, NULL);
}

void forceExit(const char *msg, const char* msg2)
{
	if ((!tracemode)&&(!DEBUG)){
		exit_ui();
	}
	std::cerr << msg;
	if (msg2 != NULL)
	{
		std::cerr << msg2;
	}
	std::cerr << std::endl;

        exit(0);
}

class device {
public:
	device (char * m_name, device * m_next = NULL)
	{
		name = m_name; next = m_next;
	}
	char * name;
	device * next;
};

class handle {
public:
	handle (dp_handle * m_handle, char * m_devicename = NULL, 
			handle * m_next = NULL) {
		content = m_handle; next = m_next; devicename = m_devicename;
	}
	dp_handle * content;
	char * devicename;
	handle * next;
};

extern local_addr * local_addrs;

int main (int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Please, enter a filename" << std::endl;
		exit(-1);
	}

	local_addrs = new local_addr (/*TODO*/1, NULL);
	needrefresh = false;

	process_init();

	tracemode = 1;
	if ((!tracemode) && (!DEBUG)){
		init_ui();
	}

	char errbuf[DP_ERRBUF_SIZE];

	//dp_handle * newhandle = dp_open_live(current_dev->name, BUFSIZ, promisc, 100, errbuf); 
	dp_handle * newhandle = dp_open_offline(argv[1], errbuf); 
	dp_addcb (newhandle, dp_packet_ip, process_ip);
	dp_addcb (newhandle, dp_packet_ip6, process_ip6);
	dp_addcb (newhandle, dp_packet_tcp, process_tcp);
	dp_addcb (newhandle, dp_packet_udp, process_udp);

	signal (SIGALRM, &alarm_cb);
	signal (SIGINT, &quit_cb);
	//alarm (refreshdelay);
	//fprintf(stderr, "Waiting for first packet to arrive (see sourceforge.net bug 1019381)\n");
	struct dpargs * userdata = (dpargs *) malloc (sizeof (struct dpargs));
	userdata->sa_family = AF_UNSPEC;
	int ret = dp_dispatch (newhandle, -1, (u_char *)userdata, sizeof (struct dpargs));
	free (userdata);
	if (ret == -1)
	{
		std::cout << "Error dispatching: " << dp_geterr(newhandle);
	}
	std::cout << "Done dispatching. " << dp_geterr(newhandle);
	do_refresh();
}

