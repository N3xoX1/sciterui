
module sciter.graphics;

import std.string;
import std.exception;

import sciter.types;
public import sciter.dom;
public import sciter.graphics.types;
public import sciter.om.value;
import sciter.graphics.api;
import sciter.utils.cvt;

COLOR rgba(ubyte R,ubyte G,ubyte B,ubyte A = 255) { return RGBA(R,G,B,A); }
COLOR rgba(float R,float G,float B,float A = 1.0f) { return RGBA(cast(ubyte)(R*255.0f),cast(ubyte)(G*255.0f),cast(ubyte)(B*255.0f),cast(ubyte)(A*255.0f)); }

struct Graphics {

  HGFX handle;

  this(HGFX h) { handle = h; if(handle) checkRV(GraphicsAddRef(handle)); }
  this(ref return scope const Graphics g) { this(g.handle); }

  ~this() { clear(); }
  void clear() { if(handle) { checkRV(GraphicsRelease(handle)); handle = null; } }

  void opAssign(HGFX h) { clear(); handle = h; if(handle) checkRV(GraphicsAddRef(handle)); }
  void opAssign(Graphics g) { clear(); this = g.handle; }

  static Graphics fromVALUE(ref VALUE v) { HGFX hgfx; checkRV(vUnWrapGfx(&v,&hgfx)); return Graphics(hgfx);}
  VALUE toVALUE() { VALUE v; checkRV(vWrapGfx(handle,&v)); return v; }

  /// Draws line from x1,y1 to x2,y2 using current lineColor and lineGradient.
  void line(float x1, float y1, float x2, float y2) { checkRV(GraphicsLine(handle,x1,y1,x2,y2)); }
  /// Draws rectangle using current lineColor/lineGradient and fillColor/fillGradient with (optional) rounded corners.
  void rect(float x1, float y1, float x2, float y2) { checkRV(GraphicsRectangle(handle,x1,y1,x2,y2)); }
  /// Draws rounded rectangle using current lineColor/lineGradient and fillColor/fillGradient with rounded corners.
  void roundRect(float x1, float y1, float x2, float y2, float r) { 
    float[8] radii = [r,r,r,r,r,r,r,r];
    checkRV(GraphicsRoundedRectangle(handle,x1,y1,x2,y2,radii.ptr)); 
  }
  void roundRect(float x1, float y1, float x2, float y2, float rx, float ry) { 
    float[8] radii = [rx,ry,rx,ry,rx,ry,rx,ry];
    checkRV(GraphicsRoundedRectangle(handle,x1,y1,x2,y2,radii.ptr)); 
  }
  /// Draws circle or ellipse using current lineColor/lineGradient and fillColor/fillGradient.  
  void ellipse(float x, float y, float rx, float ry) { checkRV(GraphicsEllipse(handle,x,y,rx,ry)); }
  /// Draws closed arc using current lineColor/lineGradient and fillColor/fillGradient.
  void arc(float x, float y, float rx, float ry, float startAngle, float sweepAngle) { checkRV(GraphicsArc(handle,x,y,rx,ry,startAngle, sweepAngle)); }
  /// Draws star.  
  void star(float x, float y, float rInner, float rOuter, float startAngle, uint rays) { checkRV(GraphicsStar(handle,x,y,rInner, rOuter, startAngle, rays)); }
  /// Draws closed polygon.
  void polygon(float[] xy) { checkRV(GraphicsPolygon(handle, xy.ptr, cast(uint) xy.length)); }
  /// Draws polyline
  void polyline(float[] xy) { checkRV(GraphicsPolyline(handle, xy.ptr, cast(uint) xy.length)); }

  /// draw text block with gravity (1..9 on MUMPAD) at px,py
  /// E.g.: draw(Text,100,100,5) will draw text box with its center at 100,100 px
  void draw(ref Text text, float x, float y, uint gravity = 7 /*left/top*/) { checkRV(GraphicsDrawText(handle,text.handle,x,y,gravity)); }
  
  /// draw path (fill or stroke or both)
  void draw(ref Path path, DRAW_PATH_MODE dpm = DRAW_PATH_MODE.FILL_AND_STROKE) { checkRV(GraphicsDrawPath(handle,path.handle,dpm)); }

  /// draw the image as it is
  void draw(ref Image img, float x, float y, float opacity = 1.0f) {
    checkRV(GraphicsDrawImage(handle, img.handle,x, y, null, null, null, null, null, null, opacity < 1.0f ? &opacity: null));
  }
  /// draw the image with scaling
  void draw(ref Image img, float x, float y, float w, float h, float opacity = 1.0f) {
    checkRV(GraphicsDrawImage(handle, img.handle,x, y, &w, &h, null, null, null, null, opacity < 1.0f ? &opacity: null));
  }
  /// draw portion of the image (a.k.a. sprite)
  void draw(ref Image img, float x, float y, float w, float h, 
                           uint ix, uint iy, uint iw, uint ih, float opacity = 1.0f) {
    checkRV(GraphicsDrawImage(handle, img.handle,x, y, &w, &h, &ix, &iy, &iw, &ih, opacity < 1.0f ? &opacity: null));
  }

  /// SECTION: affine tranformations:
  void rotate(float angle) { checkRV(GraphicsRotate(handle, angle, null, null)); }
  void rotate(float angle, float cx, float cy) { checkRV(GraphicsRotate(handle, angle, &cx, &cy)); }
  void translate(float dx, float dy) { checkRV(GraphicsTranslate(handle, dx, dy)); }
  void scale(float sx, float sy) { checkRV(GraphicsScale(handle, sx, sy)); }
  void skew(float dx, float dy) { checkRV(GraphicsSkew(handle, dx, dy)); }
  // all above in one shot
  void transform(float m11, float m12, float m21, float m22, float dx, float dy) { checkRV(GraphicsTransform(handle, m11, m12, m21, m22, dx, dy)); }

  /// SECTION: clipping, these functions set clip or clipping layer (if opacity < 1.0) 
  void pushClipBox(float x1, float y1, float x2, float y2, float opacity = 1.0f) { checkRV(GraphicsPushClipBox(handle, x1, y1, x2, y2, opacity)); } 
  void pushClipPath(ref const(Path) path, float opacity = 1.0f) { checkRV(GraphicsPushClipPath(handle, path.handle, opacity)); }   
  void popClip() { checkRV(GraphicsPopClip(handle)); } 

  /// SECTION: state save/restore 
  void stateSave() { checkRV(GraphicsStateSave(handle)); } 
  void stateRestore() { checkRV(GraphicsStateRestore(handle)); } 

  /// SECTION: coordinate space
  void worldToScreen(ref float inoutX, ref float inoutY) { checkRV(GraphicsWorldToScreen(handle, &inoutX, &inoutY)); } 
  void screenToWorld(ref float inoutX, ref float inoutY) { checkRV(GraphicsScreenToWorld(handle, &inoutX, &inoutY)); } 

  /// SECTION: drawing attributes
  void lineWidth(float width) { checkRV(GraphicsLineWidth(handle, width)); }
  void lineJoin(LINE_JOIN type) { checkRV(GraphicsLineJoin(handle, type)); }
  void lineCap(LINE_CAP type) { checkRV(GraphicsLineCap(handle, type)); }

  /// COLOR for solid lines/strokes
  void lineColor(COLOR c) { checkRV(GraphicsLineColor(handle, c)); }
  /// COLOR for solid fills
  void fillColor(COLOR c) { checkRV(GraphicsFillColor(handle, c)); }

  /// disable stroke / fill
  void noLine() { checkRV(GraphicsLineColor(handle, 0)); }
  void noFill() { checkRV(GraphicsFillColor(handle, 0)); }

  /// set linear gradient of lines and fills.
  void lineGradientLinear(float x1, float y1, float x2, float y2, COLOR_STOP[] stops) { checkRV(GraphicsLineGradientLinear(handle, x1, y1, x2, y2, stops.ptr, cast(uint)stops.length)); }
  void fillGradientLinear(float x1, float y1, float x2, float y2, COLOR_STOP[] stops) { checkRV(GraphicsFillGradientLinear(handle, x1, y1, x2, y2, stops.ptr, cast(uint)stops.length)); }

  /// set gradient radial of lines and fills.
  void lineGradientRadial(float x, float y, float rx, float ry, COLOR_STOP[] stops) { checkRV(GraphicsLineGradientRadial(handle, x, y, rx, ry, stops.ptr, cast(uint)stops.length)); }
  void fillGradientRadial(float x, float y, float rx, float ry, COLOR_STOP[] stops) { checkRV(GraphicsFillGradientRadial(handle, x, y, rx, ry, stops.ptr, cast(uint)stops.length)); }  

  void fillEvenOdd() { checkRV(GraphicsFillMode(handle, 1)); }
  void fillNonZero() { checkRV(GraphicsFillMode(handle, 0)); }

  // void flush() { checkRV(GraphicsFlush(handle)); }

}

struct Path {

  HPATH handle;

  this(HPATH h) { handle = h; if(handle) checkRV(PathAddRef(handle)); }
  this(ref return scope const Path g) { this(g.handle); }
  ~this() { clear(); }
  void clear() { if(handle) { checkRV(PathRelease(handle)); handle = null; } }

  void opAssign(HPATH h) { clear(); handle = h; if(handle) checkRV(PathAddRef(handle)); }
  void opAssign(Path g) { clear(); this = g.handle; }

  static Path fromVALUE(ref VALUE v) { HPATH h; checkRV(vUnWrapPath(&v,&h)); return Path(h);}
  VALUE toVALUE() { VALUE v; checkRV(vWrapPath(handle,&v)); return v; }

  ref Path moveTo(float x, float y) { checkRV(PathMoveTo(handle,x,y,0)); return this; }
  ref Path moveToRel(float x, float y) { checkRV(PathMoveTo(handle,x,y,1)); return this; }

  ref Path lineTo(float x, float y) { checkRV(PathLineTo(handle,x,y,0)); return this; }
  ref Path lineToRel(float x, float y) { checkRV(PathLineTo(handle,x,y,1)); return this; }

  ref Path arcTo(float x, float y, ANGLE angle, float rx, float ry, bool isLargeArc, bool isClockwise) 
      { checkRV(PathArcTo(handle,x,y,angle,rx,ry,isLargeArc,isClockwise, 0)); return this; }
  ref Path arcToRel(float x, float y, ANGLE angle, float rx, float ry, bool isLargeArc, bool isClockwise) 
      { checkRV(PathArcTo(handle,x,y,angle,rx,ry,isLargeArc,isClockwise, 1)); return this; }

  ref Path quadraticCurveTo(float xc, float yc, float x, float y) { checkRV(PathQuadraticCurveTo(handle,xc,yc,x,y,0)); return this; }
  ref Path quadraticCurveToRel(float xc, float yc, float x, float y) { checkRV(PathQuadraticCurveTo(handle,xc,yc,x,y,1)); return this; }

  ref Path bezierCurveTo(float xc1, float yc1, float xc2, float yc2, float x, float y) { checkRV(PathBezierCurveTo(handle,xc1,yc1,xc2,yc2,x,y,0)); return this; }
  ref Path bezierCurveToRel(float xc1, float yc1, float xc2, float yc2, float x, float y) { checkRV(PathBezierCurveTo(handle,xc1,yc1,xc2,yc2,x,y,1)); return this; }

  ref Path close() { checkRV(PathClosePath(handle)); return this; }
}

/// Text, a.k.a. TextLayout
struct Text { 

  HTEXT handle;

  this(HTEXT h) { handle = h; if(handle) checkRV(TextAddRef(handle)); }
  this(ref return scope const Text g) { this(g.handle); }
  ~this() { clear(); }
  void clear() { if(handle) { checkRV(TextRelease(handle)); handle = null; } }

  void opAssign(HTEXT h) { clear(); handle = h; if(handle) checkRV(TextAddRef(handle)); }
  void opAssign(Text t) { clear(); this = t.handle; }

  static Text fromVALUE(ref VALUE v) { HTEXT h; checkRV(vUnWrapText(&v,&h)); return Text(h); }
  VALUE toVALUE() const { VALUE v; checkRV(vWrapText(handle,&v)); return v; }

  /// Text toDraw = Text.create("Foo"w, self);

  static Text createForElement(wstring text, Element host) { 
    HTEXT h; 
    //function(HTEXT* ptext, LPCWSTR text, UINT textLength, HELEMENT he, LPCWSTR classNameOrNull )
    checkRV(TextCreateForElement(&h, cast(LPCWSTR)text.ptr, cast(UINT)text.length, host.handle, null )); 
    return Text(h); 
  }
  /// Text toDraw = Text.create("Foo"w, self, "canvas-text"w);
  static Text createForElement(wstring text, Element host, wstring cssClassName) { 
    HTEXT h; 
    checkRV(TextCreateForElement(&h, cast(LPCWSTR)text.ptr, cast(UINT)text.length, host.handle, toWstringz(cssClassName) )); 
    return Text(h); 
  }
  /// Text toDraw = Text.create("Foo"w, self, "font:12pt Arial,Verdana,sans-serif"w);
  static Text createForElementAndStyle(wstring text, Element host, wstring cssStyle) { 
    HTEXT h; 
    checkRV(TextCreateForElementAndStyle(&h, cast(LPCWSTR)text.ptr, cast(UINT)text.length, host.handle, cast(LPCWSTR)cssStyle.ptr, cast(UINT)cssStyle.length )); 
    return Text(h); 
  }

  struct METRICS {
    float minWidth;
    float maxWidth;
    float height;
    float ascent;
    float descent;
    uint  nLines;
  }

  METRICS metrics() const { METRICS r; checkRV(TextGetMetrics(handle, &r.minWidth, &r.maxWidth,&r.height,&r.ascent,&r.descent, &r.nLines)); return r; }

  /// set width/height of this text block, text will be aligned according to css text-align,vertical-align
  void box(float width, float height) { checkRV(TextSetBox(handle, width, height)); }

}


struct Image {
  HIMG handle;

  this(HIMG h) { handle = h; if(handle) checkRV(ImageAddRef(handle)); }
  this(ref return scope const Image g) { this(g.handle); }
  ~this() { clear(); }
  void clear() { if(handle) { checkRV(ImageRelease(handle)); handle = null; } }

  void opAssign(HIMG h) { clear(); handle = h; if(handle) checkRV(ImageAddRef(handle)); }
  void opAssign(Image g) { clear(); this = g.handle; }

  static Image fromVALUE(ref VALUE v) { HIMG h; checkRV(vUnWrapImage(&v,&h)); return Image(h);}
  VALUE toVALUE() { VALUE v; checkRV(vWrapImage(handle,&v)); return v; }

  /// create image of width / height dimensions
  static Image create(uint width, uint height) { HIMG h; checkRV(ImageCreate(&h, width, height, 1)); return Image(h); }
  /// render an element on bitmap and return it
  static Image createSnapshotOf(Element el) { HIMG h; checkRV(ImageCreateFromElement(&h, el.handle)); return Image(h); }

  /// construct image from B[n+0],G[n+1],R[n+2],A[n+3] data. 
  /// size of pixmap data (bytes) is pixmapWidth*pixmapHeight*4,
  /// format is PIXMAP_FORMAT.
  static Image createFromPixmap(uint pixelsWidth, uint pixelsHeight, PIXMAP_FORMAT format, const(ubyte)[] bytes) {
    enforce(pixelsWidth * pixelsHeight * 4 <= bytes.length, "bytes length is invalid");
    HIMG h; checkRV(ImageCreateFromPixmap(&h,pixelsWidth,pixelsHeight,format,bytes.ptr));
    return Image(h);
  }

  /// construct image from data containg supported bitmap serialization formats:
  /// at least PNG, JPG, WEBP. 
  static Image createFromData(const(ubyte)[] data) { HIMG h; checkRV(ImageLoad(data.ptr,cast(UINT)data.length,&h)); return Image(h); }

  /// Image update - draws on image using Graphics primitives
  void paint(void delegate(ref Graphics gfx, uint w, uint h) painter ) {
    PainterHolder ph;
    ph.painter = painter;
    checkRV(ImagePaint( handle, &ImagePainter, cast(void*)&ph));
  }
  /// clears image by setting all its pixels to the color:
  void clear(COLOR color) { checkRV(ImageClear(handle,color)); }

  /// serialize image to png/jpeg/etc. bytes
  /// quality is actual for jpeg and webp and shall be in 10..100 range
  void saveTo(void delegate(const(ubyte)[] bytes) receiver, IMAGE_ENCODING encoding, uint quality = 0) {
     ReceiverHolder ph;
     ph.receiver = receiver;
     checkRV(ImageSave(handle, &ImageWriter, cast(void*)&ph, encoding, quality));
  }

  SIZE dim() const {
    uint w; uint h; int d;
    checkRV(ImageGetInfo(handle, &w, &h, &d));
    return SIZE(cast(int)w,cast(int)h);
  }
}  

struct PainterHolder {
  void delegate(ref Graphics gfx, uint w, uint h) painter;
}

extern(System) SBOOL ImagePainter(LPVOID prm, HGFX hgfx, UINT width, UINT height) {
  PainterHolder* ph = cast(PainterHolder*)prm;
  Graphics gfx = hgfx;
  ph.painter(gfx,width,height);
  return 1;
}

struct ReceiverHolder {
  void delegate(const(ubyte)[] bytes) receiver;
}

extern(System) SBOOL ImageWriter(LPVOID prm, LPCBYTE data, UINT dataLength) {
  ReceiverHolder* ph = cast(ReceiverHolder*)prm;
  ph.receiver(data[0..dataLength]);
  return 1;
}

private void checkRV(GRAPHIN_RESULT rv) {
  if(rv == GRAPHIN_RESULT.OK)
    return;
  else if(rv == GRAPHIN_RESULT.BAD_PARAM)
    throw new Exception("Invalid parameter");
  else if(rv == GRAPHIN_RESULT.FAILURE)
    throw new Exception("Operation failed");
  else if(rv == GRAPHIN_RESULT.NOTSUPPORTED)
    throw new Exception("Operation not supported");
  else if(rv == GRAPHIN_RESULT.PANIC)
    throw new Exception("Runtime panic error");
}

