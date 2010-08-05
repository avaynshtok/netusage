/* NetHogs console UI */
#ifndef __CUI_H
#define __CUI_H

class NHLine {
public:
	NHLine (const char * name, double n_recv_value, double n_sent_value, pid_t pid, uid_t uid, const char * n_devicename)
	{
		assert (pid >= 0);
		m_name = name;
		sent_value = n_sent_value;
		recv_value = n_recv_value;
		devicename = n_devicename;
		m_pid = pid;
		m_uid = uid;
		assert (m_pid >= 0);
	}
	
	void show (int row);
	
	double sent_value;
	double recv_value;

	const char * m_name;
	const char * devicename;
	pid_t m_pid;
	uid_t m_uid;
};

void do_refresh ();
void init_ui ();
void exit_ui ();

/* periodically gives some CPU-time to the UI */
void ui_tick ();

#endif