//
//  ProcessNameCell.m
//  netusage
//
//  Created by System Administrator on 8/5/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ProcessNameCell.h"
#import "MainController.h"

@implementation ProcessNameCell

-(void)awakeFromNib
{
	[super awakeFromNib];
}


- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView*)controlView
{		
	NSRect buttonFrame = cellFrame;
 	buttonFrame.size.width = cellFrame.size.height;

	NSImage *image = ((ProcInfo *)self.objectValue).icon;
	
	if (image != nil) {
		NSRect imageRect;
		imageRect.origin = NSZeroPoint;
		imageRect.size = [image size];
		[image setFlipped:YES];
		
		[image drawInRect:buttonFrame fromRect:imageRect operation:NSCompositeSourceOver fraction:1];
	}
	 
	
	cellFrame.origin.x += 20;
	[super drawWithFrame:cellFrame inView:controlView];
}

@end
