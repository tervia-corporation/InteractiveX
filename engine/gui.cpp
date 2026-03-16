#include "gui.h"
#include "core/camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdio>

// ---------------------------------------------------------------------------
//  Vertices
// ---------------------------------------------------------------------------
struct GUIVertex { float x,y,z,rhw; unsigned int color; };
#define IX_GUI_FVF (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

struct GUISpriteVertex { float x,y,z,rhw; unsigned int color; float u,v; };
#define IX_GUI_SPRITE_FVF (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// ---------------------------------------------------------------------------
//  Font cache
// ---------------------------------------------------------------------------
#define IX_FONT_DEFAULT_SIZE 18
#define IX_FONT_CACHE_MAX    16

struct GUIFontEntry { int size,bold; ID3DXFont* font; };
static GUIFontEntry      g_fontCache[IX_FONT_CACHE_MAX];
static int               g_fontCacheCount = 0;
static IDirect3DDevice9* g_guiDevice      = NULL;

static ID3DXFont* GUI_GetFont(int size, int bold)
{
    if (size<=0) size = IX_FONT_DEFAULT_SIZE;
    bold = bold?1:0;
    for (int i=0;i<g_fontCacheCount;i++)
        if (g_fontCache[i].size==size && g_fontCache[i].bold==bold)
            return g_fontCache[i].font;
    int slot = (g_fontCacheCount<IX_FONT_CACHE_MAX) ? g_fontCacheCount++ : IX_FONT_CACHE_MAX-1;
    if (g_fontCache[slot].font) { g_fontCache[slot].font->Release(); g_fontCache[slot].font=NULL; }
    if (!g_guiDevice) return NULL;
    ID3DXFont* font=NULL;
    D3DXCreateFontA(g_guiDevice,size,0,bold?FW_BOLD:FW_NORMAL,1,FALSE,DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial",&font);
    g_fontCache[slot]={size,bold,font};
    return font;
}
static void GUI_ReleaseAllFonts()
{
    for (int i=0;i<g_fontCacheCount;i++)
        if (g_fontCache[i].font){g_fontCache[i].font->Release();g_fontCache[i].font=NULL;}
    g_fontCacheCount=0;
}

// ---------------------------------------------------------------------------
//  Helpers internos
// ---------------------------------------------------------------------------
static unsigned int ApplyOpacity(unsigned int color, int opacity)
{
    if (opacity>=255) return color;
    if (opacity<=0)   return color&0x00FFFFFF;
    unsigned int a=((color>>24)&0xFF)*(unsigned int)opacity/255u;
    return (color&0x00FFFFFF)|(a<<24);
}
static unsigned int BlendColor(unsigned int c1, unsigned int c2, float t)
{
    if (t<=0.f) return c1; if (t>=1.f) return c2;
    int a1=(int)((c1>>24)&0xFF),r1=(int)((c1>>16)&0xFF),g1=(int)((c1>>8)&0xFF),b1=(int)(c1&0xFF);
    int a2=(int)((c2>>24)&0xFF),r2=(int)((c2>>16)&0xFF),g2=(int)((c2>>8)&0xFF),b2=(int)(c2&0xFF);
    return ((unsigned int)(a1+(int)((float)(a2-a1)*t))<<24)|
           ((unsigned int)(r1+(int)((float)(r2-r1)*t))<<16)|
           ((unsigned int)(g1+(int)((float)(g2-g1)*t))<<8)|
            (unsigned int)(b1+(int)((float)(b2-b1)*t));
}
static void DrawTextRect(int x,int y,int w,int h,const char* text,
                          unsigned int color,unsigned int flags,int fontSize,int bold)
{
    if (!g_guiDevice||!text) return;
    ID3DXFont* font=GUI_GetFont(fontSize,bold);
    if (!font) return;
    RECT r={x,y,x+w,y+h};
    font->DrawTextA(NULL,text,-1,&r,flags,color);
}
static void DrawSegment(int x1,int y1,int x2,int y2,int sw,unsigned int color)
{
    if (!g_guiDevice) return;
    if (y1==y2){GUI_DrawFilledRect(x1,y1,x2-x1,sw,color);return;}
    if (x1==x2){GUI_DrawFilledRect(x1,y1,sw,y2-y1,color);return;}
    int steps=x2>x1?x2-x1:x1-x2;
    int dxi=(x2>x1)?1:-1, dyi=(y2>y1)?1:-1;
    for (int i=0;i<=steps;i++) GUI_DrawFilledRect(x1+dxi*i,y1+dyi*i,sw,sw,color);
}
// Mede largura de texto em pixels (para ancoragem de world text)
static int MeasureTextWidth(const char* text, int fontSize, int bold)
{
    ID3DXFont* font = GUI_GetFont(fontSize, bold);
    if (!font || !text) return 0;
    RECT r = {0,0,2000,100};
    font->DrawTextA(NULL, text, -1, &r, DT_CALCRECT|DT_NOCLIP, 0xFFFFFFFF);
    return (int)(r.right - r.left);
}
static int MeasureTextHeight(int fontSize)
{
    return (fontSize > 0) ? fontSize : IX_FONT_DEFAULT_SIZE;
}

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
bool GUI_Initialize()
{
    g_guiDevice=NULL; g_fontCacheCount=0;
    for (int i=0;i<IX_FONT_CACHE_MAX;i++) g_fontCache[i]={0,0,NULL};
    return true;
}
bool GUI_InternalCreate(IDirect3DDevice9* device)
{
    if (!device) return false;
    g_guiDevice=device;
    GUI_GetFont(IX_FONT_DEFAULT_SIZE,1);
    return true;
}
void GUI_BeginFrame(){}
void GUI_EndFrame(){}
void GUI_Cleanup(){ GUI_ReleaseAllFonts(); g_guiDevice=NULL; }

// ---------------------------------------------------------------------------
//  Texto screen-space
// ---------------------------------------------------------------------------
void GUI_DrawTextLine(int x,int y,const char* text,unsigned int color)
{
    DrawTextRect(x,y,1400,40,text,color,DT_LEFT|DT_NOCLIP,IX_FONT_DEFAULT_SIZE,1);
}
void GUI_DrawTextEx(int x,int y,int w,int h,const char* text,unsigned int color,
                    int alignment,int fontSize,int bold,int wrap)
{
    unsigned int flags=wrap?DT_WORDBREAK:(DT_VCENTER|DT_SINGLELINE);
    switch(alignment){case 1:flags|=DT_CENTER;break;case 2:flags|=DT_RIGHT;break;default:flags|=DT_LEFT;}
    DrawTextRect(x,y,w,h,text,color,flags,fontSize,bold);
}

// ---------------------------------------------------------------------------
//  Texto world-space
// ---------------------------------------------------------------------------
void GUI_DrawWorldText(float worldX, float worldY,
                       const char* text, unsigned int color,
                       float anchorX, float anchorY,
                       int alignment, int fontSize, int bold)
{
    if (!text) return;
    int sx=(int)worldX, sy=(int)worldY;
    IXCamera* cam = Camera_GetActive();
    if (cam) Camera_WorldToScreen(cam, worldX, worldY, &sx, &sy);

    int fs = (fontSize>0) ? fontSize : IX_FONT_DEFAULT_SIZE;
    int tw = MeasureTextWidth(text, fs, bold);
    int th = MeasureTextHeight(fs);

    sx -= (int)(anchorX * (float)tw);
    sy -= (int)(anchorY * (float)th);

    GUI_DrawTextEx(sx, sy, tw+2, th+2, text, color, alignment, fs, bold, 0);
}

// ---------------------------------------------------------------------------
//  Geometria
// ---------------------------------------------------------------------------
void GUI_DrawFilledRect(int x,int y,int w,int h,unsigned int color)
{
    if (!g_guiDevice) return;
    float l=(float)x,t=(float)y,r=(float)(x+w),b=(float)(y+h);
    GUIVertex v[4]={{l,t,0,1,color},{r,t,0,1,color},{l,b,0,1,color},{r,b,0,1,color}};
    g_guiDevice->SetTexture(0,NULL);
    g_guiDevice->SetFVF(IX_GUI_FVF);
    g_guiDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(GUIVertex));
}
void GUI_DrawOutlinedRect(int x,int y,int w,int h,unsigned int color)
{
    GUI_DrawFilledRect(x,y,w,1,color);
    GUI_DrawFilledRect(x,y+h-1,w,1,color);
    GUI_DrawFilledRect(x,y,1,h,color);
    GUI_DrawFilledRect(x+w-1,y,1,h,color);
}
void GUI_DrawGradientRect(int x,int y,int w,int h,unsigned int ct,unsigned int cb)
{
    if (!g_guiDevice) return;
    float l=(float)x,t=(float)y,r=(float)(x+w),b=(float)(y+h);
    GUIVertex v[4]={{l,t,0,1,ct},{r,t,0,1,ct},{l,b,0,1,cb},{r,b,0,1,cb}};
    g_guiDevice->SetTexture(0,NULL);
    g_guiDevice->SetFVF(IX_GUI_FVF);
    g_guiDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(GUIVertex));
}
void GUI_DrawChamferRect(int x,int y,int w,int h,int r,unsigned int color)
{
    if (!g_guiDevice||r<=0){GUI_DrawFilledRect(x,y,w,h,color);return;}
    if (r>w/2)r=w/2; if (r>h/2)r=h/2;
    float cx=(float)(x+w/2),cy=(float)(y+h/2);
    float fx=(float)x,fy=(float)y,fw=(float)w,fh=(float)h,fr=(float)r;
    GUIVertex v[10];
    v[0]={cx,cy,0,1,color};
    v[1]={fx+fr,fy,0,1,color}; v[2]={fx+fw-fr,fy,0,1,color};
    v[3]={fx+fw,fy+fr,0,1,color}; v[4]={fx+fw,fy+fh-fr,0,1,color};
    v[5]={fx+fw-fr,fy+fh,0,1,color}; v[6]={fx+fr,fy+fh,0,1,color};
    v[7]={fx,fy+fh-fr,0,1,color}; v[8]={fx,fy+fr,0,1,color};
    v[9]=v[1];
    g_guiDevice->SetTexture(0,NULL); g_guiDevice->SetFVF(IX_GUI_FVF);
    g_guiDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,8,v,sizeof(GUIVertex));
}
void GUI_DrawChamferGradientRect(int x,int y,int w,int h,int r,unsigned int ct,unsigned int cb)
{
    if (!g_guiDevice) return;
    if (r<=0){GUI_DrawGradientRect(x,y,w,h,ct,cb);return;}
    if (r>w/2)r=w/2; if (r>h/2)r=h/2;
    float fx=(float)x,fy=(float)y,fw=(float)w,fh=(float)h,fr=(float)r;
    #define CGRAD(dy) BlendColor(ct,cb,(dy)/fh)
    GUIVertex v[10];
    v[0]={fx+fw/2,fy+fh/2,0,1,CGRAD(fh/2)};
    v[1]={fx+fr,fy,0,1,CGRAD(0)}; v[2]={fx+fw-fr,fy,0,1,CGRAD(0)};
    v[3]={fx+fw,fy+fr,0,1,CGRAD(fr)}; v[4]={fx+fw,fy+fh-fr,0,1,CGRAD(fh-fr)};
    v[5]={fx+fw-fr,fy+fh,0,1,CGRAD(fh)}; v[6]={fx+fr,fy+fh,0,1,CGRAD(fh)};
    v[7]={fx,fy+fh-fr,0,1,CGRAD(fh-fr)}; v[8]={fx,fy+fr,0,1,CGRAD(fr)};
    v[9]=v[1];
    #undef CGRAD
    g_guiDevice->SetTexture(0,NULL); g_guiDevice->SetFVF(IX_GUI_FVF);
    g_guiDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,8,v,sizeof(GUIVertex));
}
void GUI_DrawChamferOutline(int x,int y,int w,int h,int r,int sw,unsigned int color)
{
    if (r<=0){GUI_DrawOutlinedRect(x,y,w,h,color);return;}
    if (r>w/2)r=w/2; if (r>h/2)r=h/2; if (sw<1)sw=1;
    GUI_DrawFilledRect(x+r,y,w-r*2,sw,color);
    GUI_DrawFilledRect(x+r,y+h-sw,w-r*2,sw,color);
    GUI_DrawFilledRect(x,y+r,sw,h-r*2,color);
    GUI_DrawFilledRect(x+w-sw,y+r,sw,h-r*2,color);
    DrawSegment(x+r,y,x,y+r,sw,color);
    DrawSegment(x+w-r,y,x+w,y+r,sw,color);
    DrawSegment(x+w,y+h-r,x+w-r,y+h,sw,color);
    DrawSegment(x,y+h-r,x+r,y+h,sw,color);
}
void GUI_DrawSprite(IDirect3DTexture9* texture,
                    int srcX,int srcY,int srcW,int srcH,
                    int dstX,int dstY,int dstW,int dstH,unsigned int tint)
{
    if (!g_guiDevice||!texture||dstW<=0||dstH<=0) return;
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0,&desc))) return;
    float tw=(float)desc.Width,th=(float)desc.Height;
    if (srcW<=0)srcW=(int)tw; if (srcH<=0)srcH=(int)th;
    float u0=srcX/tw,v0=srcY/th,u1=(srcX+srcW)/tw,v1=(srcY+srcH)/th;
    float x0=(float)dstX,y0=(float)dstY,x1=(float)(dstX+dstW),y1=(float)(dstY+dstH);
    GUISpriteVertex v[4]={{x0,y0,0,1,tint,u0,v0},{x1,y0,0,1,tint,u1,v0},
                          {x0,y1,0,1,tint,u0,v1},{x1,y1,0,1,tint,u1,v1}};
    g_guiDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
    g_guiDevice->SetTexture(0,texture);
    g_guiDevice->SetFVF(IX_GUI_SPRITE_FVF);
    g_guiDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(GUISpriteVertex));
    g_guiDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
    g_guiDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    g_guiDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
    g_guiDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
    g_guiDevice->SetFVF(IX_GUI_FVF);
}

// ---------------------------------------------------------------------------
//  Widgets
// ---------------------------------------------------------------------------
void GUI_DrawButton(const IXButton* b)
{
    if (!b) return;
    const IXButtonStyle* s=Button_GetCurrentStyle(b);
    if (!s) return;
    int op=b->opacity, x=b->x, y=b->y, w=b->width, h=b->height, r=b->cornerRadius;
    if (r>w/2)r=w/2; if (r>h/2)r=h/2;
    if (b->spriteTexture)
    {
        int sx,sy,sw2,sh;
        if (!b->enabled)    {sx=b->spriteDisabledX;sy=b->spriteDisabledY;sw2=b->spriteDisabledW;sh=b->spriteDisabledH;}
        else if(b->pressed) {sx=b->spritePressedX; sy=b->spritePressedY; sw2=b->spritePressedW; sh=b->spritePressedH;}
        else if(b->hovered) {sx=b->spriteHoverX;   sy=b->spriteHoverY;   sw2=b->spriteHoverW;   sh=b->spriteHoverH;}
        else                {sx=b->spriteNormalX;  sy=b->spriteNormalY;  sw2=b->spriteNormalW;  sh=b->spriteNormalH;}
        GUI_DrawSprite(b->spriteTexture,sx,sy,sw2,sh,x,y,w,h,ApplyOpacity(s->imageTint,op));
    }
    else
    {
        int hasGrad=((s->gradColorTop|s->gradColorBottom)&0xFF000000)!=0;
        if (hasGrad){
            unsigned int ct=ApplyOpacity(s->gradColorTop,op),cb=ApplyOpacity(s->gradColorBottom,op);
            if (r>0) GUI_DrawChamferGradientRect(x,y,w,h,r,ct,cb);
            else     GUI_DrawGradientRect(x,y,w,h,ct,cb);
        } else {
            unsigned int bg=ApplyOpacity(s->bgColor,op);
            if (r>0) GUI_DrawChamferRect(x,y,w,h,r,bg);
            else     GUI_DrawFilledRect(x,y,w,h,bg);
        }
    }
    if (s->strokeWidth>0&&((s->strokeColor>>24)&0xFF)>0){
        unsigned int sc=ApplyOpacity(s->strokeColor,op);
        if (r>0) GUI_DrawChamferOutline(x,y,w,h,r,s->strokeWidth,sc);
        else     GUI_DrawOutlinedRect(x,y,w,h,sc);
    }
    if (b->text&&b->text[0]){
        int pad=10,tx=x+pad,ty=y,tw2=w-pad*2,th=h;
        if (b->pressed){tx+=b->textOffsetPressX;ty+=b->textOffsetPressY;}
        GUI_DrawTextEx(tx,ty,tw2,th,b->text,ApplyOpacity(s->textColor,op),
                       b->textAlignment,b->fontSize,b->fontBold,0);
    }
}

void GUI_DrawPanel(const IXPanel* p)
{
    if (!p) return;
    int x=p->x, y=p->y, w=p->width, h=p->height, r=p->cornerRadius;
    if (r>w/2)r=w/2; if (r>h/2)r=h/2;

    // Corpo
    if (p->bgTexture)
    {
        GUI_DrawSprite(p->bgTexture,0,0,0,0,x,y,w,h,p->bgTextureTint);
    }
    else
    {
        int hasGrad=((p->bgGradTop|p->bgGradBottom)&0xFF000000)!=0;
        if (hasGrad)
        {
            if (r>0) GUI_DrawChamferGradientRect(x,y,w,h,r,p->bgGradTop,p->bgGradBottom);
            else     GUI_DrawGradientRect(x,y,w,h,p->bgGradTop,p->bgGradBottom);
        }
        else
        {
            if (r>0) GUI_DrawChamferRect(x,y,w,h,r,p->backgroundColor);
            else     GUI_DrawFilledRect(x,y,w,h,p->backgroundColor);
        }
    }

    // Header
    if (p->headerHeight>0)
    {
        int hasGrad=((p->headerGradTop|p->headerGradBottom)&0xFF000000)!=0;
        if (hasGrad)
            GUI_DrawGradientRect(x,y,w,p->headerHeight,p->headerGradTop,p->headerGradBottom);
        else
            GUI_DrawFilledRect(x,y,w,p->headerHeight,p->headerColor);
        // Separador header/corpo
        GUI_DrawFilledRect(x,y+p->headerHeight-1,w,1,p->borderColor);
    }

    // Borda
    if (p->strokeWidth>0&&((p->borderColor>>24)&0xFF)>0)
    {
        if (r>0) GUI_DrawChamferOutline(x,y,w,h,r,p->strokeWidth,p->borderColor);
        else     GUI_DrawOutlinedRect(x,y,w,h,p->borderColor);
    }

    // Titulo
    if (p->title&&p->title[0])
    {
        int fs = (p->titleFontSize>0) ? p->titleFontSize : IX_FONT_DEFAULT_SIZE;
        int tx = x+p->padding;
        int ty = y+((p->headerHeight-fs)/2)-1;
        if (ty<y+4) ty=y+4;
        int tw = w - p->padding*2;
        GUI_DrawTextEx(tx,ty,tw,p->headerHeight,p->title,p->titleColor,
                       p->titleAlignment,fs,p->titleFontBold,0);
    }
}

void GUI_DrawLabel(const IXLabel* l)
{
    if (!l||!l->text||!l->text[0]) return;
    int fs  = (l->fontSize>0) ? l->fontSize : IX_FONT_DEFAULT_SIZE;
    int pad = l->backgroundPadding;
    int bx=l->x-pad, by=l->y-pad, bw=l->width+pad*2, bh=l->height+pad*2;
    int r=l->bgCornerRadius;

    // Fundo
    if (l->bgTexture)
    {
        GUI_DrawSprite(l->bgTexture,0,0,0,0,bx,by,bw,bh,l->bgTextureTint);
    }
    else if ((l->bgGradTop|l->bgGradBottom)&0xFF000000)
    {
        if (r>0) GUI_DrawChamferGradientRect(bx,by,bw,bh,r,l->bgGradTop,l->bgGradBottom);
        else     GUI_DrawGradientRect(bx,by,bw,bh,l->bgGradTop,l->bgGradBottom);
    }
    else if ((l->backgroundColor>>24)&0xFF)
    {
        if (r>0) GUI_DrawChamferRect(bx,by,bw,bh,r,l->backgroundColor);
        else     GUI_DrawFilledRect(bx,by,bw,bh,l->backgroundColor);
    }

    // Borda
    if (((l->borderColor>>24)&0xFF)&&l->strokeWidth>0)
    {
        if (r>0) GUI_DrawChamferOutline(bx,by,bw,bh,r,l->strokeWidth,l->borderColor);
        else     GUI_DrawOutlinedRect(bx,by,bw,bh,l->borderColor);
    }

    // Sombra + texto
    if (l->drawShadow)
        GUI_DrawTextEx(l->x+1,l->y+1,l->width,l->height,l->text,l->shadowColor,
                       l->alignment,fs,l->fontBold,l->wrapText);
    GUI_DrawTextEx(l->x,l->y,l->width,l->height,l->text,l->textColor,
                   l->alignment,fs,l->fontBold,l->wrapText);
}

void GUI_DrawSlider(const IXSlider* s)
{
    if (!s) return;
    int tw = s->showValue ? (s->width-54) : s->width;
    if (tw<20) tw=20;

    int range=s->maxValue-s->minValue;
    int fill=0;
    if (range>0) fill=((s->value-s->minValue)*tw)/range;
    if (fill<0)fill=0; if (fill>tw)fill=tw;

    int th    = (s->trackHeight>0) ? s->trackHeight : 6;
    int trackY = s->y+s->height-th-2;
    if (trackY<s->y+20) trackY=s->y+20;

    int kw = s->knobWidth>0  ? s->knobWidth  : 10;
    int kh = s->knobHeight>0 ? s->knobHeight : 18;
    int kr = s->knobRadius;
    int ky = trackY+(th/2)-(kh/2);
    int kx = s->x+fill-(kw/2);
    if (kx<s->x) kx=s->x;
    if (kx>s->x+tw-kw) kx=s->x+tw-kw;

    // Label
    int lfs = s->labelFontSize>0 ? s->labelFontSize : IX_FONT_DEFAULT_SIZE;
    DrawTextRect(s->x,s->y-1,tw,18,s->label?s->label:"Slider",
                 s->textColor,DT_LEFT|DT_VCENTER|DT_SINGLELINE,lfs,1);

    // Valor
    if (s->showValue)
    {
        int vfs=s->valueFontSize>0?s->valueFontSize:IX_FONT_DEFAULT_SIZE;
        char val[32]; std::snprintf(val,sizeof(val),"%d",s->value);
        DrawTextRect(s->x,s->y-1,s->width,18,val,s->valueColor,
                     DT_RIGHT|DT_VCENTER|DT_SINGLELINE,vfs,1);
    }

    // Track inset (sombra interna)
    if ((s->trackInsetColor>>24)&0xFF)
        GUI_DrawFilledRect(s->x,trackY+1,tw,th,s->trackInsetColor);

    // Track fundo + fill
    int tr=s->trackRadius;
    if (tr>0)
    {
        GUI_DrawChamferRect(s->x,trackY,tw,th,tr,s->trackColor);
        if (fill>0) GUI_DrawChamferRect(s->x,trackY,fill,th,tr,s->fillColor);
    }
    else
    {
        GUI_DrawFilledRect(s->x,trackY,tw,th,s->trackColor);
        if (fill>0) GUI_DrawFilledRect(s->x,trackY,fill,th,s->fillColor);
    }

    // Knob
    if (s->knobTexture)
    {
        GUI_DrawSprite(s->knobTexture,0,0,0,0,kx,ky,kw,kh,0xFFFFFFFF);
    }
    else
    {
        int hasGrad=((s->knobGradTop|s->knobGradBottom)&0xFF000000)!=0;
        if (hasGrad)
        {
            if (kr>0) GUI_DrawChamferGradientRect(kx,ky,kw,kh,kr,s->knobGradTop,s->knobGradBottom);
            else      GUI_DrawGradientRect(kx,ky,kw,kh,s->knobGradTop,s->knobGradBottom);
        }
        else
        {
            if (kr>0) GUI_DrawChamferRect(kx,ky,kw,kh,kr,s->knobColor);
            else      GUI_DrawFilledRect(kx,ky,kw,kh,s->knobColor);
        }
        if ((s->knobStrokeColor>>24)&0xFF)
        {
            if (kr>0) GUI_DrawChamferOutline(kx,ky,kw,kh,kr,1,s->knobStrokeColor);
            else      GUI_DrawOutlinedRect(kx,ky,kw,kh,s->knobStrokeColor);
        }
    }
}
