//
//  browserAppDelegate.h
//  browser
//
//  Created by 袁新宇 on 11-10-20.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface browserAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
    WebView *webView;
    int order;
    int file_type;
    bool caze;
    bool offline;
    bool personal;
    NSString *dir;
    int fontSize;
    int hotkey;
    int	sockfd;
    NSString *os;
    NSString *cpu;
    NSString *disk;
    NSString *ver;
    NSString *user;
}

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet WebView *webView;
@property (nonatomic, copy) NSString *dir;
@property (nonatomic, copy) NSString *os;
@property (nonatomic, copy) NSString *cpu;
@property (nonatomic, copy) NSString *disk;
@property (nonatomic, copy) NSString *ver;
@property (nonatomic, copy) NSString *user;

- (NSString*) search: (NSString*) query;
- (NSString*) stat: (NSString*) query;
- (NSString*) history;
- (BOOL) hisDel: (int) index;
- (BOOL) hisPin: (int) index;
- (BOOL) hisUnpin: (int) index;
- (BOOL) shellDefault: (NSString*) file;
- (BOOL) shellExplore: (NSString*) file;
- (BOOL) shell2: (NSString*) file action: (NSString*)action;
- (BOOL) copyPath: (NSString*) file;
- (BOOL) term: (NSString*) path;

- (NSString*) selectDir;
- (void) crashTest;
@end

