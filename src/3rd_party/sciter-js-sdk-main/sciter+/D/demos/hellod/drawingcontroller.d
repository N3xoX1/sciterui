module drawingcontroller;

import std.datetime;
import std.datetime.systime;
import std.utf;
import core.stdc.stdio : sprintf;

import sciter;

const float PI = 3.1415926f;

class DrawingController : ElementController
{

  this(HELEMENT h) { super(h); }

  override void didStart() {
    startTimer(500, &this.onTick);
  }

  bool onTick() {
    self.requestPaint(); // will cause handleDraw() to be called
    return true; // keep timer ticking
  }

  override bool handleDraw(HELEMENT he, ref DRAW_EVENT evt) 
  { 
    if(evt.type != DRAW.CONTENT)
      return false; // we are drawing only content layer

    Graphics gfx = evt.gfx; // on this Graphics
    RECT     area = evt.area; // in this area

    auto w = area.width;
    auto h = area.height;

    float scale = w < h? w / 300.0f: h / 300.0f;

    SysTime  now = Clock.currTime(); // this time   

    gfx.stateSave();

      gfx.translate(area.left + w / 2.0f, area.top + h / 2.0f);
      gfx.scale(scale,scale);    
      gfx.rotate(-PI/2);
      gfx.lineColor(0);
      gfx.lineWidth(8.0f);
      gfx.lineCap(LINE_CAP.ROUND);
         
      // Hour marks
      gfx.stateSave();
        gfx.lineColor(rgba(0x32,0x5F,0xA2));
        for (int i = 0; i < 12; ++i) {
          gfx.rotate(PI/6);
          gfx.line(137.0f,0,144.0f,0);
        }
      gfx.stateRestore();

      // Minute marks
      gfx.stateSave();
        gfx.lineWidth(3);
        gfx.lineColor(rgba(0xA5,0x2A, 0x2A));
        for (int i = 0; i < 60; ++i) {
          if ( i % 5 != 0)
            gfx.line(143,0,146,0);
          gfx.rotate(PI/30.0f);
        }
      gfx.stateRestore();

      // show current date
      { 
        gfx.stateSave();
        gfx.rotate(PI/2);
        char[20] buffer;
        int written = sprintf(buffer.ptr, "%04d-%02d-%02d",now.year,now.month,now.day);
        Text text = Text.createForElementAndStyle(toUTF16(buffer[0..written]), self, "font-size:10pt;color:brown"w );
        gfx.draw(text, 0, 60, 5);
        gfx.stateRestore();
      }

      uint sec = now.second;
      uint min = now.minute;
      uint hr  = now.hour;
      hr = hr >= 12 ? hr - 12 : hr;
    
      // draw hours hand
      gfx.stateSave();
        gfx.rotate( hr*(PI/6) + (PI/360)*min + (PI/21600)*sec );
        gfx.lineWidth(14);
        gfx.lineColor(rgba(0x32,0x5F,0xA2));
        gfx.line(-20,0,70,0);
      gfx.stateRestore();

      // draw Minute hand
      gfx.stateSave();
        gfx.rotate( (PI/30)*min + (PI/1800)*sec );
        gfx.lineWidth(10);
        gfx.lineColor(rgba(0x32,0x5F,0xA2));
        gfx.line(-28,0,100,0);
      gfx.stateRestore();
        
      // draw Second hand
      gfx.stateSave();
        gfx.rotate(sec * PI/30);
        gfx.lineColor(rgba(0xD4,0,0));
        gfx.fillColor(rgba(0xD4,0,0));
        gfx.lineWidth(6);
        gfx.line(-30,0,83,0);
        gfx.ellipse(0,0,10,10);
    
        gfx.noFill();
        gfx.ellipse(95,0,10,10);
      gfx.stateRestore();

    gfx.stateRestore();      


    return true; 
  }
}

static this() {
  registerElementController!DrawingController("custom-drawing-controller");
                                           //  ^ for CSS: behavior: custom-drawing-controller;
}
