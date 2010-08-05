//
//  cocoa_testAppDelegate.h
//  cocoa_test
//
//  Created by Anton Vaynshtok on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface cocoa_testAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
