//
//  kaleAppDelegate.m
//  kale
//
//  Created by exe on 8/16/10.
//  Copyright cobbler 2010. All rights reserved.
//

#import "kaleAppDelegate.h"

#import "eagl_view.h"

#import "root.h"
#import "renderer.h"

#import "kale.h"

@implementation kaleAppDelegate

@synthesize window;

static EAGLView* gl_view;

- (void)update
{
	// calculate delta time
	CFTimeInterval now_time = CFAbsoluteTimeGetCurrent();
	static CFTimeInterval last_time = now_time;
	static CFTimeInterval delta_time = 0;
	delta_time = now_time - last_time;
	last_time = now_time;
	
	// App update
	kaleApp::Ins().Update(delta_time);
	
	// ERI update
	ERI::Root::Ins().Update();
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	// init ERI
	ERI::Root::Ins().Init(false);
	ERI::Root::Ins().renderer()->SetViewOrientation(ERI::PORTRAIT_HOME_BOTTOM);
	
	if ([UIScreen instancesRespondToSelector:@selector(scale)])
		ERI::Root::Ins().renderer()->set_content_scale([[UIScreen mainScreen] scale]);
	
	// create the OpenGL view and add it to the window
	gl_view = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[gl_view enableAccelerometer:YES withTimeInterval:1.0/10.0];
	[window addSubview:gl_view];
	
	// init App
	kaleApp::Ins().Init();
	
	// create our rendering timer
	[NSTimer scheduledTimerWithTimeInterval:0 target:self selector:@selector(update) userInfo:nil repeats:YES];	
	
    // Override point for customization after application launch
    [window makeKeyAndVisible];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	kaleApp::Ins().OnTerminate();
}

- (void)dealloc
{
	kaleApp::Ins().Release();
	
	[gl_view release];
    [window release];
    [super dealloc];
}


@end
