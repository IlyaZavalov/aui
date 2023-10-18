#include "RenderTargets.h"

using namespace gl;


std::unique_ptr<Framebuffer::IRenderTarget> RenderTargetDepth::attach(Framebuffer& to) { 
/*
    using Impl = ImplementationManager<RenderTarget,
#if !AUI_PLATFORM_ANDROID && !AUI_PLATFORM_IOS
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT24, GLType::FLOAT>,
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT, GLType::FLOAT>,
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT16, GLType::FLOAT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT24, GLType::FLOAT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT, GLType::FLOAT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT16, GLType::FLOAT>,
#endif
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT, GLType::UNSIGNED_INT>,
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT, GLType::UNSIGNED_SHORT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT, GLType::UNSIGNED_INT>,
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT24, GLType::UNSIGNED_SHORT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT24, GLType::UNSIGNED_INT>,
            //TextureRenderTarget<GLInternalFormat::DEPTH_COMPONENT16, GLType::UNSIGNED_SHORT>,
            RenderbufferDepthRenderTarget<GLInternalFormat::DEPTH_COMPONENT16, GLType::UNSIGNED_SHORT>
            >;

    return Impl::tryAllUntilSuccess([&](std::unique_ptr<ms::RenderTarget> variant) {
        variant->attach(to, GL_DEPTH_ATTACHMENT);
        return variant;
    });*/
    return nullptr;
}


std::unique_ptr<Framebuffer::IRenderTarget> RenderTargetRGBA8::attach(Framebuffer& to, GLenum attachment) {
/*
    using Impl = ImplementationManager<Framebuffer::IRenderTarget,
            TextureRenderTarget<GLInternalFormat::RGBA8, GLType::UNSIGNED_BYTE, GLFormat::RGBA>,
            TextureRenderTarget<GLInternalFormat::RGBA,  GLType::UNSIGNED_BYTE, GLFormat::RGBA>
            >;

    return Impl::tryAllUntilSuccess([&](std::unique_ptr<ms::RenderTarget> variant) {
        variant->attach(to, attachment);
        return variant;
    });*/
    return nullptr;
}
