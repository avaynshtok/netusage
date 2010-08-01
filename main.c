#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>


int anton_main (int argc, const char * argv[]) {
	int i; //, mib[CTL_MAXNAME];
	size_t len;
	struct kinfo_proc kp;

	int mib[3];
	
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCARGS; //KERN_PROC;
	//name[2] = KERN_PROC_PID;
	//name[3] = pid;
	
	
	/* Fill out the first three components of the mib */
	//len = 4;
	//int size = sysctlnametomib("kern.proc.pid", mib, &len);
	//printf("size: %i", size);
	//perror("nametomib");
	
	//return 0;
	
	/* Fetch and print entries for pid's < 100 */
	i = 5670;
//	for (i = 1025; i < 4000; i++) {
		mib[2] = i;
		
		len = sizeof(kp);
		
		int argv_len;
		sysctl(mib, 3, NULL, &argv_len, NULL, 0);
		char *proc_argv = malloc(sizeof(char) * argv_len);
		//sysctl(mib, 3, proc_argv, &argv_len, NULL, 0);
		
		
		
		//if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1) {
		//if (sysctlbyname("kern.proc.pid", &kp, &len, NULL, 0) == -1) {
		if (sysctl(mib, 3, proc_argv, &argv_len, NULL, 0) == -1) {
			perror("sysctl");
		}
		else if (len > 0) {
			//printkproc(&kp);
//			printf("%s", kp.kp_eproc.e_wmesg);
			printf("progrv: %s\n", proc_argv);			
			//printf("%s", kp.kp_proc.p_pid);
			//printf("\n");
		}
//	}	
	
	
	
	printf("Hello, World!\n");
	return 0;
}
