
#import <Foundation/NSError.h>

#import "SpecialProtocol.h"
#import "NSImage+QuickLook.h"



/* NSImage -> jfif utility category. */

@implementation NSImage (JFIFConversionUtils)

	/* returns jpeg file interchange format encoded data for an NSImage regardless of the
	original NSImage encoding format.  compressionValue is between 0 and 1.  
	values 0.6 thru 0.7 are fine for most purposes.  */
- (NSData *)JFIFData:(float) compressionValue {

		/* convert the NSImage into a raster representation. */
	NSBitmapImageRep* myBitmapImageRep = [NSBitmapImageRep imageRepWithData: [self TIFFRepresentation]];
	
		/* convert the bitmap raster representation into a jfif data stream */
	NSDictionary* propertyDict =
		[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:compressionValue]
			forKey: NSImageCompressionFactor];
	
		/* return the jfif encoded data */
	return [myBitmapImageRep representationUsingType: NSJPEGFileType properties:propertyDict];
}

@end



@implementation SpecialProtocol

+ (NSString*) specialProtocolScheme {
	return @"gigaso";
}

+ (void) registerSpecialProtocol {
	static BOOL inited = NO;
	if ( ! inited ) {
		[NSURLProtocol registerClass:[SpecialProtocol class]];
		inited = YES;
	}
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)theRequest {
	NSString *theScheme = [[theRequest URL] scheme];
	return ([theScheme caseInsensitiveCompare: [SpecialProtocol specialProtocolScheme]] == NSOrderedSame );
}


+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

- (void)render_image:(NSImage *)myImage request:(NSURLRequest *)request  {
  NSData *data = [myImage JFIFData: 0.75];
	NSURLResponse *response = 
		[[NSURLResponse alloc] initWithURL:[request URL] 
			MIMEType:@"image/jpeg" 
			expectedContentLength:-1 
			textEncodingName:nil];
    id<NSURLProtocolClient> client = [self client];
	[client URLProtocol:self didReceiveResponse:response
			cacheStoragePolicy:NSURLCacheStorageNotAllowed];
	[client URLProtocol:self didLoadData:data];
	[client URLProtocolDidFinishLoading:self];
		/* we can release our copy */
	[response release];
		
		/* if an error occured during our load, here is the code we would
		execute to send that information back to webKit.  We're not using it here,
		but you will probably want to use this code for proper error handling.  */
	if (0) { /* in case of error */
        int resultCode;
        resultCode = NSURLErrorResourceUnavailable;
        [client URLProtocol:self didFailWithError:[NSError errorWithDomain:NSURLErrorDomain
			code:resultCode userInfo:nil]];
	}

}
- (void)startLoading {
    NSURLRequest *request = [self request];
	NSString* path = [[[request URL] path] substringFromIndex:0];
    if([@"thumb" compare:[[request URL] host]] == NSOrderedSame ){
        NSSize size = {100,100};
        NSImage *myImage = [NSImage imageWithPreviewOfFileAtPath:path ofSize:size asIcon:YES];
        [self render_image: myImage request: request];
    }else if([@"icon" compare:[[request URL] host]] == NSOrderedSame ){
        NSSize size = {16,16};
        NSImage *myImage = [NSImage iconOfFileAtPath:path ofSize:size];
        [self render_image: myImage request: request];
    }
}

- (void)stopLoading {}


@end

