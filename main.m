//
//  main.m
//  kale
//
//  Created by exe on 8/16/10.
//  Copyright cobbler 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "kaleAppDelegate.h"

int main(int argc, char *argv[]) {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([kaleAppDelegate class]));
    [pool release];
    return retVal;
}
