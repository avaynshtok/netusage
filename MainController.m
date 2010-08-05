//
//  MainController.m
//  netusage
//
//  Created by System Administrator on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MainController.h"
#import "cui.h"

@implementation MainController
@synthesize myTableView;

NSMutableDictionary *procs = [[NSMutableDictionary alloc] init];
MainController *instance;

- (void) awakeFromNib {
	instance = self;
	NSLog(@"awoken from nib");
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	myTableView = tableView;
    return [procs count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	myTableView = tableView;
	int count = [procs count];
	
	if (count > row) {
		ProcInfo *pi = (ProcInfo *) [[procs allValues] objectAtIndex:row];
		
		if ([[tableColumn identifier] isEqualToString:@"pid"]) {
			NSString *pid = [NSString stringWithFormat:@"%i", pi.pid];
			return pid;
		}
		else if ([[tableColumn identifier] isEqualToString:@"sent"]) {
			NSString *sent = [NSString stringWithFormat:@"%10.3f", pi.sent];
			return sent;
		}
		else if ([[tableColumn identifier] isEqualToString:@"received"]) {
			NSString *rcvd = [NSString stringWithFormat:@"%10.3f", pi.received];
			return rcvd;
		}
		else {
			return pi.name;
		}
	}
	
	return NULL;
}

void updateGUI(NHLine **lines, int numprocs) {
	NSLog(@"hey there, we have %i procs", numprocs);
	
	ProcessSerialNumber psn;
//	ProcessInfoRec info;
	CFStringRef processName = NULL;
	OSStatus result;
	
	for (int i = 0; i < numprocs; i++) {
		NHLine *line = lines[i];
		
		pid_t pid = line->m_pid;
		if (pid != 0) {
			result = GetProcessForPID(pid, &psn);
			result = CopyProcessName(&psn, &processName);
		
			if (processName != NULL) {
				NSLog(@"got proc %@", processName);
				
				NSString *processNameString = (NSString*) processName;
				NSNumber *pidNum = [NSNumber numberWithInt:pid];
				
				ProcInfo *pi = [procs objectForKey:pidNum];
				if (pi == nil) {
					pi = [[ProcInfo alloc] init];
					pi.pid = pid;
				}
				
				pi.name = processNameString;
				pi.sent = line->sent_value;
				pi.received = line->recv_value;

				[procs setObject:pi forKey:pidNum];
				CFRelease(processName);
				[pidNum release];
			}
		}
	}
	
	[instance.myTableView reloadData];
}
@end


@implementation ProcInfo
@synthesize pid, name, sent, received;
@end