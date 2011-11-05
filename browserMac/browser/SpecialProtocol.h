
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>


	/* our custom NSURLProtocol is implemented as a subclass. */
@interface SpecialProtocol : NSURLProtocol {
}
+ (NSString*) specialProtocolScheme;
+ (void) registerSpecialProtocol;
@end

	/* utility category on NSImage used for converting
	NSImage to jfif data.  */
@interface NSImage (JFIFConversionUtils)

	/* returns jpeg file interchange format encoded data for an NSImage regardless of the
	original NSImage encoding format.  compressionValue is between 0 and 1.  
	values 0.6 thru 0.7 are fine for most purposes.  */
- (NSData *)JFIFData:(float) compressionValue;

@end

