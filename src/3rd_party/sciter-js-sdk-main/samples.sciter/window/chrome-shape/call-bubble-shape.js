
export function callBubblePath(element, arrowPosition)
{
      const rad = 16; // corner radius
      const asz = 16; // arrow size
      let [x1,y1,x2,y2] = element.state.box("ltbr","inner","window");

      const t = new Graphics.Path();
      let c;

      switch(arrowPosition) {
        case "top":
          c = (x1 + x2) / 2;
          t.moveTo(c, y1);
          y1 += asz;
          t.lineTo(c - asz, y1); t.lineTo(c + asz, y1); t.close();
          break; 
        case "bottom":
          c = (x1 + x2) / 2;
          t.moveTo(c, y2);
          y2 -= asz;
          t.lineTo(c - asz, y2); t.lineTo(c + asz, y2); t.close();
          break; 
      }
      t.roundRect(x1,y1,x2-x1,y2-y1,rad);
      return t;
   }
