//
//  main.m
//  cocoa_test
//
//  Created by Anton Vaynshtok on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

void launchGUI() {	
	AuthorizationRef auth = NULL;
	OSStatus err;
	
	AuthorizationItem myItems = {kAuthorizationRightExecute, 0, NULL, 0};
	AuthorizationRights myRights = {1, &myItems};
	//myRights.count = sizeof (myItems) / sizeof (myItems[0]);
	//myRights.items = myItems;	
	
	AuthorizationFlags myFlags;
	myFlags = kAuthorizationFlagDefaults |	kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize;
	
	err = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, myFlags, &auth);
	if ( err != errAuthorizationSuccess ) {
		NSLog(@"authorizationcreate failed");
	}
	
	// get app path
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFStringRef toolName = (CFStringRef) @"netusage";
	char *toolPath = (char *)malloc(10000);
	CFURLRef nethogsUrl = CFBundleCopyAuxiliaryExecutableURL(bundle, toolName);
	CFURLGetFileSystemRepresentation(nethogsUrl, true, (UInt8 *) toolPath, 10000);
	
	NSLog(@"toolPath: %s", toolPath);
	char *arg = "--do-gui";

	//char *abc = "23";
	const char *args[2];
	args[0] = toolPath;
	args[1] = arg;
	
	// call the tool
	err = AuthorizationExecuteWithPrivileges(auth, toolPath, kAuthorizationFlagDefaults, &arg, NULL);
	//NSLog(@"err: %@", err);
}

int main(int argc, char *argv[])
{
	//NSLog(@"num args: %i", argc);
	//if (argc == 2) { // && strcmp(argv[1], "--do-gui") == 0) {
		return NSApplicationMain(argc,  (const char **) argv);
	//}		
//	else {
//		launchGUI();
//		exit(1);
//	}
}