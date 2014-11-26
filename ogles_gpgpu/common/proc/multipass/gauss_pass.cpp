#include "gauss_pass.h"

using namespace ogles_gpgpu;

const char *GaussProcPass::fshaderGaussSrc = TO_STR(
precision mediump float;

uniform sampler2D uInputTex;
uniform float uPxD;
varying vec2 vTexCoord;
// 7x1 Gauss kernel
void main() {
    vec4 pxC  = texture2D(uInputTex, vTexCoord);
    vec4 pxL1 = texture2D(uInputTex, vTexCoord - vec2(uPxD, 0.0));
    vec4 pxL2 = texture2D(uInputTex, vTexCoord - vec2(2.0 * uPxD, 0.0));
    vec4 pxL3 = texture2D(uInputTex, vTexCoord - vec2(3.0 * uPxD, 0.0));
    vec4 pxR1 = texture2D(uInputTex, vTexCoord + vec2(uPxD, 0.0));
    vec4 pxR2 = texture2D(uInputTex, vTexCoord + vec2(2.0 * uPxD, 0.0));
    vec4 pxR3 = texture2D(uInputTex, vTexCoord + vec2(3.0 * uPxD, 0.0));
    gl_FragColor = 0.006 * (pxL3 + pxR3)
                 + 0.061 * (pxL2 + pxR2)
                 + 0.242 * (pxL1 + pxR1)
                 + 0.382 * pxC;
}
);

int GaussProcPass::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    cout << "ogles_gpgpu::GaussProcPass " << renderPass << " - init" << endl;
    
    // create fbo for output
    createFBO();
    
    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);
    
    // calculate pixel delta values
    pxDx = 1.0f / (float)outFrameW;
    pxDy = 1.0f / (float)outFrameH;
    
    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(fshaderGaussSrc, RenderOrientationDiagonal);
    
    // get additional shader params
    shParamUPxD = shader->getParam(UNIF, "uPxD");
    
    return 1;
}

void GaussProcPass::createFBOTex(bool genMipmap) {
    assert(fbo);
    
    if (renderPass == 1) {
        fbo->createAttachedTex(outFrameH, outFrameW, genMipmap);   // swapped
    } else {
        fbo->createAttachedTex(outFrameW, outFrameH, genMipmap);
    }
    
    // update frame size, because it might be set to a POT size because of mipmapping
    outFrameW = fbo->getTexWidth();
    outFrameH = fbo->getTexHeight();
}

void GaussProcPass::render() {
    cout << "ogles_gpgpu::GaussProcPass " << renderPass << " - to framebuffer of size " << outFrameW << "x" << outFrameH << endl;
    
    filterRenderPrepare();
	
	glUniform1f(shParamUPxD, renderPass == 1 ? pxDy : pxDx);	// texture pixel delta values
    
    Tools::checkGLErr("ogles_gpgpu::GaussProcPass - render prepare");
    
    filterRenderSetCoords();
    Tools::checkGLErr("ogles_gpgpu::GaussProcPass - render set coords");
    
    filterRenderDraw();
    Tools::checkGLErr("ogles_gpgpu::GaussProcPass - render draw");
    
    filterRenderCleanup();
    Tools::checkGLErr("ogles_gpgpu::GaussProcPass - render cleanup");
}