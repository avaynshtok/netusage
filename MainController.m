//
//  MainController.m
//  netusage
//
//  Created by System Administrator on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MainController.h"
//#import "cui.h"
#import "ProcessNameCell.h"

@implementation MainController
@synthesize myTableView;


MainController *instance;

- (void) awakeFromNib {
	instance = self;
	procs = [[NSMutableDictionary alloc] init];
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
			NSString *sent = [NSString stringWithFormat:@"%.3f", pi.sent];
			return sent;
		}
		else if ([[tableColumn identifier] isEqualToString:@"received"]) {
			NSString *rcvd = [NSString stringWithFormat:@"%.3f", pi.received];
			return rcvd;
		}
		else {
			// process name cell
			return pi;// pi.name;
		}
	}
	
	return NULL;
}
/*

void updateGUI(NHLine **lines, int numprocs) {
	NSLog(@"hey there, we have %i procs", numprocs);
		
	NSMutableSet *currentPids = [[[NSMutableSet alloc] init] autorelease];
	for (int i = 0; i < numprocs; i++) {
		NHLine *line = lines[i];
		
		pid_t pid = line->m_pid;
		if (pid != 0) {
			NSRunningApplication *app = [[NSRunningApplication runningApplicationWithProcessIdentifier:pid] autorelease];			
			NSString *processNameString = app.localizedName;
			
			if (processNameString == NULL) {
				processNameString = [NSString stringWithCString:line->m_name encoding:NSUTF8StringEncoding];
			}
			
			NSNumber *pidNum = [NSNumber numberWithInt:pid];
			
			ProcInfo *pi = [procs objectForKey:pidNum];
			if (pi == nil) {
				pi = [[ProcInfo alloc] init];
				pi.pid = pid;
			}
			
			pi.name = processNameString;
			pi.sent = line->sent_value;
			pi.received = line->recv_value;
			pi.icon = app.icon;
			
			if (app.icon != nil) {
				NSLog(@"app %@ has icon", processNameString);
			}

			[procs setObject:pi forKey:pidNum];
			[currentPids addObject:pidNum];
			
			[pidNum release];		
		}
	}
	
	for (NSNumber *pid in [procs allKeys]) {
		if (! [currentPids member:pid] ) {
			[procs removeObjectForKey:pid];
		}
	}
	
	[instance.myTableView reloadData];
}
*/
@end


@implementation ProcInfo
@synthesize pid, name, sent, received, icon;

- (id)copyWithZone:(NSZone *)zone {
	[self retain];
	return self;
}

- (NSString *)description {
	return name;
}
@end