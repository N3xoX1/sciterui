module sciter.graphics.types;

public alias HGFX = const(void)*;
public alias HIMG = const(void)*;
public alias HPATH = const(void)*;
public alias HTEXT = const(void)*;

alias POS = float; // position
alias DIM = float; // dimension
alias ANGLE = float; // angle (radians)
alias COLOR = uint; // color, RGBA

struct COLOR_STOP
{
  COLOR color;
  float offset; // 0.0 ... 1.0
}

enum DRAW_PATH_MODE : uint
{
  FILL_ONLY = 1,
  STROKE_ONLY = 2,
  FILL_AND_STROKE = 3,
}

enum LINE_JOIN : uint
{
  MITER = 0,
  ROUND = 1,
  BEVEL = 2,
  MITER_OR_BEVEL = 3,
}

enum LINE_CAP : uint
{
  BUTT = 0,
  SQUARE = 1,
  ROUND = 2,
}

enum IMAGE_ENCODING : uint
{
  RAW, // [a,b,g,r,a,b,g,r,...] vector
  PNG,
  JPG,
  WEBP,
}

enum PIXMAP_FORMAT : uint
{
  IGNORE_ALPHA = 0, 
  PREMUL_ALPHA = 1,
  FORMAT_RAW = 2,
}
