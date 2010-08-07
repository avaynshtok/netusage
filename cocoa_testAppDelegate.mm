//
//  cocoa_testAppDelegate.m
//  cocoa_test
//
//  Created by Anton Vaynshtok on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "cocoa_testAppDelegate.h"
#import "MainController.h"
//#import <sys/socket.h>

@implementation cocoa_testAppDelegate

@synthesize window;

//extern int nh_main (int argc, char** argv);

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	[NSThread detachNewThreadSelector:@selector(launchCommandLine) toTarget:self withObject:nil];
}

- (void) launchCommandLine {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	AuthorizationRef auth = NULL;
	OSStatus err;
	/*
	AuthorizationItem myItems[1];
	
	myItems[0].name = "system.privilege.admin";
	myItems[0].valueLength = 0;
	myItems[0].value = NULL;
	myItems[0].flags = 0;
	
	myItems[1].name = "com.myOrganization.myProduct.myRight2";
	myItems[1].valueLength = 0;
	myItems[1].value = NULL;
	myItems[1].flags = 0;
	*/
	
	//AuthorizationItem myItems = {kAuthorizationRightExecute, 0, NULL, 0};
	//AuthorizationRights myRights = {1, &myItems};
	//myRights.count = sizeof (myItems) / sizeof (myItems[0]);
	//myRights.items = myItems;	
	
	//	err = AuthorizationExecuteWithPrivileges(auth, command, kAuthorizationFlagDefaults, args, NULL);
	
	AuthorizationFlags myFlags;
	myFlags = kAuthorizationFlagDefaults |	kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize;
	
	/*
	err = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, myFlags, &auth);
	if ( err != errAuthorizationSuccess ) {
		NSLog(@"authorizationcreate failed");
	}
	err = AuthorizationCopyRights(auth, &myRights, kAuthorizationEmptyEnvironment, myFlags, NULL);
	if ( err != errAuthorizationSuccess ) {
		NSLog(@"AuthorizationCopyRights failed");
	}*/
	/*
	err = AuthorizationCreate (&myRights, kAuthorizationEmptyEnvironment, myFlags, NULL);
	
	if (socket(AF_INET, SOCK_RAW, htons(0x0806)) < 0) {
		NSLog(@"can't do auth");
	}
	*/
	char *c;
	//nh_main(1, &c);
	
	[pool release];
}

@end
