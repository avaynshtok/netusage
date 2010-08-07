//
//  cocoa_testAppDelegate.m
//  cocoa_test
//
//  Created by Anton Vaynshtok on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "cocoa_testAppDelegate.h"
#import "MainController.h"

@implementation cocoa_testAppDelegate

@synthesize window;

extern int nh_main (int argc, char** argv);

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	[NSThread detachNewThreadSelector:@selector(launchCommandLine) toTarget:self withObject:nil];
}

- (void) launchCommandLine {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	char *c;
	nh_main(1, &c);
	
	[pool release];
}

@end
