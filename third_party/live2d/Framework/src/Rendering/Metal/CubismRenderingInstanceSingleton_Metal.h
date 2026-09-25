
#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

@interface CubismRenderingInstanceSingleton_Metal : NSObject {
    id <MTLDevice> mtlDevice;
    CAMetalLayer* metalLayer;
}
+ (id)sharedManager;
- (void)setMTLDevice:(id <MTLDevice>)param;
- (id <MTLDevice>)getMTLDevice;

- (void)setMetalLayer:(CAMetalLayer*)param;
- (CAMetalLayer*)getMetalLayer;

@end
