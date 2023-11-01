// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#import "AUIView.h"
#include <OpenGLES/EAGLDrawable.h>
#include <OpenGLES/ES2/glext.h>

#include <string>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/AWindow.h>
@implementation AUIView
{
    CADisplayLink* displayLink;
    BOOL animating;
    NSMutableArray<UITouch*>* trackedTouches;
}

@synthesize context;

static AUIView* view = nullptr;

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

static std::string ios_get_path_in_bundle() {
    std::string s;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* docsDir = [paths objectAtIndex:0];
    s = [docsDir UTF8String];
    return s;
}
static const _<AWindow>& auiWindow() {
    return AWindow::getWindowManager().getWindows().front();
}


extern int(* _gEntry)(AStringVector);

static GLuint defaultFb, colorBuffer = 0;

- (instancetype) initWithFrame:(CGRect)frame
{    
    if ((self = [super initWithFrame:frame]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            assert(0);
            return nil;
		}
        
        trackedTouches = [[NSMutableArray<UITouch*> alloc] init];
        animating = NO;
        displayLink = nil;
        view = self;
        
        // setup "default" framebuffer
        glGenFramebuffers(1, &defaultFb);
        

        chdir(ios_get_path_in_bundle().c_str());
        _gEntry({});
    }
	
    return self;
}

- (void) drawRect:(CGRect)rect
{
    [super drawRect:rect];
}

- (void) drawView:(id)sender
{
    [EAGLContext setCurrentContext:context];
    
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        
        assert(defaultFb == 1);
        
        [self layoutSubviews];
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }
    
    if (!AWindow::getWindowManager().getWindows().empty()) {
        AThread::processMessages();
        AWindow::getWindowManager().getWindows().front()->redraw();
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        [context presentRenderbuffer:GL_RENDERBUFFER];
    }
}
- (void) layoutSubviews
{
    [EAGLContext setCurrentContext:context];
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFb);
    
    if (colorBuffer != 0) {
        glDeleteRenderbuffers(1, &colorBuffer);
    }
    
    glGenRenderbuffers(1, &colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    if (![self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer]) {
        assert(0);
    }
    
    CGSize size = self.bounds.size;
    float scale = (float)self.contentScaleFactor;
    auiWindow()->setSize({size.width * scale, size.height * scale});
}

- (void) startAnimation
{
    if (!animating)
    {
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];

        [displayLink setFrameInterval:1];

        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
        [self setNeedsDisplay];
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [displayLink invalidate];
        displayLink = nil;
        animating = FALSE;
    }
}

- (void) dealloc
{
    // tear down context
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auto vec = glm::ivec2{location.x * scale, location.y * scale};
        auiWindow()->onPointerPressed({vec, APointerIndex::finger([trackedTouches count])});
        [trackedTouches addObject:touch];
    }
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auiWindow()->onPointerMove(glm::vec2{location.x * scale, location.y * scale}, APointerMoveEvent{APointerIndex::finger([trackedTouches indexOfObject:touch])});
    }
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auiWindow()->onPointerReleased({glm::ivec2{location.x * scale, location.y * scale}, APointerIndex::finger([trackedTouches indexOfObject:touch])});
        [trackedTouches removeObject:touch];
    }
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auiWindow()->onPointerReleased({.position = glm::ivec2{location.x * scale, location.y * scale}, .pointerIndex = APointerIndex::finger([trackedTouches indexOfObject:touch]), .triggerClick = false});
        [trackedTouches removeObject:touch];
    }
}

extern "C" void _aui_ios_redraw() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [view setNeedsDisplay];
    });
}

@end
