//
//  MainController.h
//  netusage
//
//  Created by System Administrator on 8/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MainController : NSObject<NSTableViewDataSource, NSTableViewDelegate> {
	NSTableView *myTableView;
}
@property(nonatomic,assign) NSTableView *myTableView;

@end


@interface ProcInfo : NSObject {
	NSString *name;
	pid_t pid;
	double sent;
	double received;
}
@property(nonatomic,copy) NSString *name;
@property(nonatomic) pid_t pid;
@property(nonatomic) double sent;
@property(nonatomic) double received;
@end