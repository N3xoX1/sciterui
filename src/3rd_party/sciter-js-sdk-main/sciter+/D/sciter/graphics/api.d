module sciter.graphics.api;

import sciter.types;
import sciter.dom.types;
import sciter.graphics.types;
import sciter.om.value;

enum GRAPHIN_RESULT
{
  PANIC = -1, // e.g. not enough memory, etc.
  OK = 0,
  BAD_PARAM = 1,  // bad parameter
  FAILURE = 2,    // operation failed, e.g. restore() without save()
  NOTSUPPORTED = 3 // the platform does not support requested feature
}

// ImageSave callback:
alias ImageWriterF = extern(System) SBOOL function(LPVOID prm, LPCBYTE data, UINT dataLength); 
// ImagePaint callback:
alias ImagePainterF = extern(System) SBOOL function(LPVOID prm, HGFX hgfx, UINT width, UINT height); 

// create COLOR value
alias RGBAF = extern(System) COLOR function(UINT red, UINT Graphicsreen, UINT blue, UINT alpha /*= 255*/);

alias GraphicsCreateF = extern(System) GRAPHIN_RESULT function(HIMG img, HGFX* pout_gfx );
alias GraphicsAddRefF = extern(System) GRAPHIN_RESULT function(HGFX Graphicsfx);
alias GraphicsReleaseF = extern(System) GRAPHIN_RESULT function(HGFX Graphicsfx);
// Draws line from x1,y1 to x2,y2 using current lineColor and lineGradient.
alias GraphicsLineF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2 );
// Draws rectangle using current lineColor/lineGradient and fillColor/fillGradient with (optional) rounded corners.
alias GraphicsRectangleF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2 );
// Draws rounded rectangle using current lineColor/lineGradient and fillColor/fillGradient with (optional) rounded corners.
alias GraphicsRoundedRectangleF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2, const(DIM)* radii8 /*DIM[8] - four rx/ry pairs */);
// Draws circle or ellipse using current lineColor/lineGradient and fillColor/fillGradient.
alias GraphicsEllipseF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x, POS y, DIM rx, DIM ry );
// Draws closed arc using current lineColor/lineGradient and fillColor/fillGradient.
alias GraphicsArcF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x, POS y, POS rx, POS ry, ANGLE start, ANGLE sweep );
// Draws star.
alias GraphicsStarF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x, POS y, DIM r1, DIM r2, ANGLE start, UINT rays );
// Closed polygon.
alias GraphicsPolygonF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, const POS* xy, UINT num_points );
// Polyline.
alias GraphicsPolylineF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, const POS* xy, UINT num_points );

// SECTION: Path operations
alias PathCreateF = extern(System) GRAPHIN_RESULT function( HPATH* Path );
alias PathAddRefF = extern(System) GRAPHIN_RESULT function( HPATH Path );
alias PathReleaseF = extern(System) GRAPHIN_RESULT function( HPATH Path );
alias PathMoveToF = extern(System) GRAPHIN_RESULT function( HPATH Path, POS x, POS y, SBOOL relative );
alias PathLineToF = extern(System) GRAPHIN_RESULT function( HPATH Path, POS x, POS y, SBOOL relative );
alias PathArcToF = extern(System) GRAPHIN_RESULT function( HPATH Path, POS x, POS y, ANGLE angle, DIM rx, DIM ry, SBOOL is_large_arc, SBOOL clockwise, SBOOL relative );
alias PathQuadraticCurveToF = extern(System) GRAPHIN_RESULT function( HPATH Path, POS xc, POS yc, POS x, POS y, SBOOL relative );
alias PathBezierCurveToF = extern(System) GRAPHIN_RESULT function( HPATH Path, POS xc1, POS yc1, POS xc2, POS yc2, POS x, POS y, SBOOL relative );
alias PathClosePathF = extern(System) GRAPHIN_RESULT function( HPATH Path );
alias GraphicsDrawPathF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, HPATH Path, DRAW_PATH_MODE dpm );

// SECTION: affine tranformations:
alias GraphicsRotateF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, ANGLE radians, const POS* cx /*= 0*/, const POS* cy /*= 0*/ );
alias GraphicsTranslateF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS cx, POS cy );
alias GraphicsScaleF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, DIM x, DIM y );
alias GraphicsSkewF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, DIM dx, DIM dy );
// all above in one shot
alias GraphicsTransformF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS m11, POS m12, POS m21, POS m22, POS dx, POS dy );

// SECTION: state save/restore

alias GraphicsStateSaveF = extern(System) GRAPHIN_RESULT function( HGFX hgfx );
alias GraphicsStateRestoreF = extern(System) GRAPHIN_RESULT function( HGFX hgfx );

// SECTION: drawing attributes

// set line width for subsequent drawings.
alias GraphicsLineWidthF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, DIM width );
alias GraphicsLineJoinF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, LINE_JOIN type );
alias GraphicsLineCapF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, LINE_CAP type);

// COLOR for solid lines/strokes
alias GraphicsLineColorF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, COLOR c);
// COLOR for solid fills
alias GraphicsFillColorF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, COLOR color );

// setup parameters of linear Graphicsradient of lines.
alias GraphicsLineGradientLinearF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2, const COLOR_STOP* stops, UINT nstops );
// setup parameters of linear Graphicsradient of fills.
alias GraphicsFillGradientLinearF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2, const COLOR_STOP* stops, UINT nstops );
// setup parameters of line Graphicsradient radial fills.
alias GraphicsLineGradientRadialF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x, POS y, DIM rx, DIM ry, const COLOR_STOP* stops, UINT nstops );
// setup parameters of Graphicsradient radial fills.
alias GraphicsFillGradientRadialF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x, POS y, DIM rx, DIM ry, const COLOR_STOP* stops, UINT nstops );

alias GraphicsFillModeF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, SBOOL even_odd /* false - fill_non_zero */ );

// create Text layout for host element
alias TextCreateForElementF = extern(System) GRAPHIN_RESULT function(HTEXT* ptext, LPCWSTR Text, UINT TextLength, HELEMENT he, LPCWSTR classNameOrNull );
// create Text layout using explicit style declaration
alias TextCreateForElementAndStyleF = extern(System) GRAPHIN_RESULT function(HTEXT* ptext, LPCWSTR Text, UINT TextLength, HELEMENT he, LPCWSTR style, UINT styleLength); 

alias TextAddRefF = extern(System) GRAPHIN_RESULT function(HTEXT Path);
alias TextReleaseF = extern(System) GRAPHIN_RESULT function(HTEXT Path);
alias TextGetMetricsF = extern(System) GRAPHIN_RESULT function(HTEXT Text, DIM* minWidth, DIM* maxWidth, DIM* height, DIM* ascent, DIM* descent, UINT* nLines);
alias TextSetBoxF = extern(System) GRAPHIN_RESULT function(HTEXT Text, DIM width, DIM height);

// draw Text with position (1..9 on MUMPAD) at px,py
// Ex: GraphicsDrawText( 100,100,5) will draw Text box with its center at 100,100 px
alias GraphicsDrawTextF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, HTEXT Text, POS px, POS py, UINT position );

// SECTION: Image rendering
// draws img onto the Graphicsraphics surface with current transformation applied (scale, rotation).
alias GraphicsDrawImageF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, HIMG himg, POS x, POS y,
                         const DIM* w /*= 0*/, const DIM* h /*= 0*/, 
                         const UINT* ix /*= 0*/, const UINT* iy /*= 0*/, const UINT* iw /*= 0*/, const UINT* ih, /*= 0*/
                         const float* opacity /*= 0, if provided is in 0.0 .. 1.0*/ );

// SECTION: coordinate space
alias GraphicsWorldToScreenF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS* inout_x, POS* inout_y);
alias GraphicsScreenToWorldF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS* inout_x, POS* inout_y);

// SECTION: clipping
alias GraphicsPushClipBoxF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, POS x1, POS y1, POS x2, POS y2, float opacity /*=1.f*/);
alias GraphicsPushClipPathF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, HPATH hpath, float opacity /*=1.f*/);

// pop clip layer previously set by GraphicsPushClipBox or GraphicsPushClipPath
alias GraphicsPopClipF = extern(System) GRAPHIN_RESULT function( HGFX hgfx);
  
// Image painter

alias ImagePaintF = extern(System) GRAPHIN_RESULT function( HIMG himg, ImagePainterF pPainter, void* prm ); // paint on Image using Graphicsraphics

// VALUE interface
alias vWrapGfxF = extern(System) GRAPHIN_RESULT function( HGFX hgfx, VALUE* toValue);
alias vWrapImageF = extern(System) GRAPHIN_RESULT function( HIMG himg, VALUE* toValue);
alias vWrapPathF = extern(System) GRAPHIN_RESULT function( HPATH hpath, VALUE* toValue);
alias vWrapTextF = extern(System) GRAPHIN_RESULT function( HTEXT htext, VALUE* toValue);

alias vUnWrapGfxF = extern(System) GRAPHIN_RESULT function( const VALUE* fromValue, HGFX *phgfx);
alias vUnWrapImageF = extern(System) GRAPHIN_RESULT function( const VALUE* fromValue, HIMG *phimg );
alias vUnWrapPathF = extern(System) GRAPHIN_RESULT function( const VALUE* fromValue, HPATH *phpath);
alias vUnWrapTextF = extern(System) GRAPHIN_RESULT function( const VALUE* fromValue, HTEXT *phtext);

alias GraphicsFlushF = extern(System) GRAPHIN_RESULT function(HGFX hgfx);

// Graphicsrab DOM element snapshot into Image
alias ImageCreateFromElementF = extern(System) GRAPHIN_RESULT function(HIMG* poutImg, HELEMENT domElement);
alias GraphicsGetNativeDCF = extern(System) GRAPHIN_RESULT function(HGFX hgfx, UINT nativeDcType, VOID** pDC); // 

// Image primitives
alias ImageCreateF = extern(System) GRAPHIN_RESULT function( HIMG* poutImg, UINT width, UINT height, SBOOL withAlpha );

// construct Image from B[n+0],G[n+1],R[n+2],A[n+3] data. 
// size of pixmap data is pixmapWidth*pixmapHeight*4,
// pixmap_format is SCITER_PIXMAP_FORMAT above.
alias ImageCreateFromPixmapF = extern(System) GRAPHIN_RESULT function( HIMG* poutImg, UINT pixmapWidth, UINT pixmapHeight, UINT pixmap_format, LPCBYTE pixmap );

alias ImageAddRefF = extern(System) GRAPHIN_RESULT function( HIMG himg );
alias ImageReleaseF = extern(System) GRAPHIN_RESULT function( HIMG himg );
alias ImageGetInfoF = extern(System) GRAPHIN_RESULT function( HIMG himg, UINT* width, UINT* height, SBOOL* usesAlpha );
alias ImageClearF = extern(System) GRAPHIN_RESULT function( HIMG himg, COLOR byColor );
alias ImageLoadF = extern(System) GRAPHIN_RESULT function( LPCBYTE bytes, UINT num_bytes, HIMG* pout_img ); // load png/jpeg/etc. Image from stream of bytes

// save png/jpeg/etc. Image to stream of bytes
alias ImageSaveF  = extern(System) GRAPHIN_RESULT function ( HIMG himg,
        ImageWriterF pfn, void* prm, /* function and its param passed "as is" */
        UINT encoding, // IMAGE_ENCODING
        UINT quality /* if webp or jpeg:, 10 - 100 */ );


RGBAF RGBA;
GraphicsCreateF GraphicsCreate;
GraphicsAddRefF GraphicsAddRef;
GraphicsReleaseF GraphicsRelease;
GraphicsLineF GraphicsLine;
GraphicsRectangleF GraphicsRectangle;
GraphicsRoundedRectangleF GraphicsRoundedRectangle;
GraphicsEllipseF GraphicsEllipse;
GraphicsArcF GraphicsArc;
GraphicsStarF GraphicsStar;
GraphicsPolygonF GraphicsPolygon;
GraphicsPolylineF GraphicsPolyline;
PathCreateF PathCreate;
PathAddRefF PathAddRef;
PathReleaseF PathRelease;
PathMoveToF PathMoveTo;
PathLineToF PathLineTo;
PathArcToF PathArcTo;
PathQuadraticCurveToF PathQuadraticCurveTo;
PathBezierCurveToF PathBezierCurveTo;
PathClosePathF PathClosePath;
GraphicsDrawPathF GraphicsDrawPath;
GraphicsRotateF GraphicsRotate;
GraphicsTranslateF GraphicsTranslate;
GraphicsScaleF GraphicsScale;
GraphicsSkewF GraphicsSkew;
GraphicsTransformF GraphicsTransform;
GraphicsStateSaveF GraphicsStateSave;
GraphicsStateRestoreF GraphicsStateRestore;
GraphicsLineWidthF GraphicsLineWidth;
GraphicsLineJoinF GraphicsLineJoin;
GraphicsLineCapF GraphicsLineCap;
GraphicsLineColorF GraphicsLineColor;
GraphicsFillColorF GraphicsFillColor;
GraphicsLineGradientLinearF GraphicsLineGradientLinear;
GraphicsFillGradientLinearF GraphicsFillGradientLinear;
GraphicsLineGradientRadialF GraphicsLineGradientRadial;
GraphicsFillGradientRadialF GraphicsFillGradientRadial;
GraphicsFillModeF GraphicsFillMode;
GraphicsDrawTextF GraphicsDrawText;
GraphicsDrawImageF GraphicsDrawImage;
GraphicsWorldToScreenF GraphicsWorldToScreen;
GraphicsScreenToWorldF GraphicsScreenToWorld;
GraphicsPushClipBoxF GraphicsPushClipBox;
GraphicsPushClipPathF GraphicsPushClipPath;
GraphicsPopClipF GraphicsPopClip;
GraphicsFlushF GraphicsFlush;
//GraphicsGetNativeDCF GraphicsGetNativeDC; // 
TextCreateForElementF TextCreateForElement;
TextCreateForElementAndStyleF TextCreateForElementAndStyle; 
TextAddRefF TextAddRef;
TextReleaseF TextRelease;
TextGetMetricsF TextGetMetrics;
TextSetBoxF TextSetBox;
ImagePaintF ImagePaint; // paint on Image using Graphicsraphics
ImageCreateFromElementF ImageCreateFromElement;
ImageCreateF ImageCreate;
ImageCreateFromPixmapF ImageCreateFromPixmap;
ImageAddRefF ImageAddRef;
ImageReleaseF ImageRelease;
ImageGetInfoF ImageGetInfo;
ImageClearF ImageClear;
ImageLoadF ImageLoad; // load png/jpeg/etc. Image from stream of bytes
ImageSaveF ImageSave;
vWrapGfxF vWrapGfx;
vWrapImageF vWrapImage;
vWrapPathF vWrapPath;
vWrapTextF vWrapText;
vUnWrapGfxF vUnWrapGfx;
vUnWrapImageF vUnWrapImage;
vUnWrapPathF vUnWrapPath;
vUnWrapTextF vUnWrapText;
