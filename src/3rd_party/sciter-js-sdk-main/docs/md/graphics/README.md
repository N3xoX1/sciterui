---
sidebar_position: 6
title: Graphics
toc_min_heading_level: 2
toc_max_heading_level: 5
---

# class Graphics

Class Graphics represents 2D graphics primitives.

In Sciter.JS Graphics is available :

1. at `<canvas>` element where it represents its offscreen buffer.
2. in `element.paintXXXX(graphics)` [methods](../dom/Element#paintXXXX) to support so called immediate mode painting.
3. in `new Image(painter(graphics))` constructor to draw on Image surface.

Class Graphics also is a namespace that contains classes:

* [Graphics.Color](Color.md)
* [Graphics.Image](Image.md)
* [Graphics.Text](Text.md)
* [Graphics.Path](Path.md)
* [Graphics.Brush](Brush.md)
* and geometry utility classes:
  - [Point](Point.md) - geometric point, also known as 2D vector; 
  - [Size](Size.md) - geometric size, pair of width/height values. 
  - [Rect](Rect.md) - rectangle, can be expressed as quad [x1,y1,x2,y2] or as pair of Point (origin) and Size (dimension).

## Properties:

### lineCap

The shape used to draw the end points of lines (strokes).

read-write, string, one of
* "butt" - default,
* "round",
* "square"

### lineJoin

the shape used to join two line segments where they meet.

read-write, string, one of
* "miter" - default,
* "round",
* "bevel"

### strokeStyle

read-write, either 
* [Color](Color) - color to draw stroke / line;
* string - string representation of a color or a gradient; 
* [Brush](Brush) - brush (gradient, tiled image, etc.);

### lineWidth
### strokeWidth

read-write, number, line width (in CSS pixels, a.k.a. dips).

### fillStyle

read-write, either 
* [Color](Color) - color to fill;
* string - string representation of a color or a gradient; 
* [Brush](Brush.md) - brush (gradient, tiled image, etc.);

### font

read-write, string, in [CSS font attribute format](https://developer.mozilla.org/en-US/docs/Web/CSS/font) 

## Methods:

### setLineDash()

sets configuration of dashed line

```js
graphics.setLineDash(n1,n2,...)
```

### beginPath()

starts a new path by emptying the list of sub-paths. 

```js
graphics.beginPath()
```

### moveTo()

```js
graphics.moveTo(x, y)
graphics.moveTo(Point)
```

### lineTo()

```js
graphics.lineTo(x, y)
graphics.lineTo(Point)
```

### quadraticCurveTo()

adds a quadratic Bézier curve to the current sub-path.

```js
graphics.quadraticCurveTo(cpx, cpy, x, y)
graphics.quadraticCurveTo(cp:Point, pt:Point)
```

### bezierCurveTo()

adds a cubic Bézier curve to the current sub-path.

```js
graphics.bezierCurveTo(p1x, cp1y, cp2x, cp2y, x, y)
graphics.bezierCurveTo(cp1:Point, cp2:Point, pt:Point)
```

### arcTo()

adds a circular arc to the current sub-path, using the given control points and radius. The arc is automatically connected to the path's latest point with a straight line, if necessary for the specified parameters.

```js
graphics.arcTo(x1, y1, x2, y2, radius)
graphics.arcTo(cp1:Point, cp2:Point, radius)
```

### arc()

adds a circular arc to the current sub-path.

```js
graphics.arc(x, y, radius, startAngle, endAngle [, anticlockwise])
```

### ellipse()

```js
graphics.ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle [, anticlockwise])
```

### rect()

```js
graphics.rect(x, y, width, height)
graphics.rect(Rect)
```

### closePath()

adds a straight line from the current point to the start of the current sub-path. If the path has already been closed or has only one point, this function does nothing.

```js
graphics.closePath()
```

### stroke()

```js
graphics.stroke()
```
strokes current path.

```js
graphics.stroke(path)
```
strokes given [Path](Path) object.

### fill()

```js
graphics.fill()
```
fills current path.

```js
graphics.fill(path[,fillRule])
```
fills given [Path](Path) object.

_fillRule_ is one of:

- "nonzero" - the [non-zero winding rule](https://en.wikipedia.org/wiki/Nonzero-rule). Default rule.
- "evenodd" - the [even/odd winding rule](https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule).

### strokeRect()

draws a rectangle that is stroked (outlined) according to the current strokeStyle and other Graphics settings.

```js
graphics.strokeRect(x,y,w,h)
graphics.strokeRect(Rect)
```

### fillRect()

draws a rectangle filled according to the current [fillStyle](#fillstyle).

```js
graphics.fillRect(x,y,w,h)
graphics.fillRect(Rect)
```

### clearRect()

erases pixels in a rectangular area by setting them to transparent black.

```js
graphics.clearRect(x,y,w,h);
graphics.clearRect(Rect);
```

### fillText()

draws a text string at the specified coordinates, filling the string's characters with the current [fillStyle](#fillstyle).

```js
graphics.fillText(text,x,y,maxWidth)
```
:::tip
In case of dynamic updates or frequent drawing of the same text it is better to create/reuse [Graphics.Text](Text) and use [graphics.draw()](#draw) method to draw it.
:::

---

### save()

saves the entire state of the graphics by pushing the current state onto a stack.

```js
graphics.save()
```

### restore()

restores the most recently saved canvas state by popping the top entry in the drawing state stack.

```js
graphics.restore()
```
---

### scale()

adds a horizontal and vertical scaling to the current transformation matrix.

```js
graphics.scale(x,y)
```

### translate()

adds a translation to the current transformation matrix.

```js
graphics.translate(x,y)
```

### rotate()

```js
graphics.rotate(angle [,x,y])
```

adds a rotation to the current transformation matrix.

parameters:
- _angle_, either number of radians or [Angle](../JS/units/Angle);
- _x,y_, if given defines center of rotation; 


### transform()

multiplies the current transformation with the matrix described by the arguments of this method. 

```js 
graphics.transform(a,b,c,d,e,f)
```

### setTransform()

resets (overrides) the current transformation to the identity matrix, and then invokes a transformation described by the arguments of this method. 

```js 
graphics.setTransform(a,b,c,d,e,f)
```
### resetTransform()

resets (overrides) the current transformation to the identity matrix. 

```js 
graphics.resetTransform()
```

## Methods (Sciter.JS specific):

### draw()

```js
graphics.draw(path, {params})
```
draws given [path object](Path), params is an object with:

* `x`, `y` - numbers, destination coordinates;
* `fill`, optional, `"evenodd"` | `"nonzero"`, if defined fills the path by current fill brush;
* `stroke`, optional, `true` | `false`, if _true_ - outlines the path by current stroke brush;

```js
graphics.draw(image, {params})
```

draws the [image object](Image), params is an object with:

* `x`, `y` - numbers, destination coordinates;
* `width` ,`height` - optional, numbers, dimension of destination box;
* `srcX`, `srcY` - optional, integer, origin of source box (sprite);
* `srcWidth`, `srcHeight` - optional, integer, dimension of source box (sprite);
* `opacity`, optional, 0.0 ... 1.0 , opacity (blending);

```js
graphics.draw(text, {params})
```
draws the [text object](Text), params is an object with:  

* `x`, `y` - numbers, destination coordinates;
* `alignment`: integer, 1..9, defines meaning of x/y coordinates, see NUMPAD. 5 - center of text, 7 - left/top corner, etc.
* `fill` : Color, optional, if defined text is draw by that color. If ommited then by current fill color.

### pushLayer()

```js
graphics.pushLayer(x,y,w,h[,opacity|filter])
```

Pushes the layer(clip) on layers stack with rectangular clip with optional opacity or filter.

parameters:
- _x_,_y_,_w_,_h_ - rectangle;
- _opacity_ - number, in range 0.0 ... 1.0 - draw content of the layer with the opacity;
- _filter_ - string, filter in [CSS filter property](https://developer.mozilla.org/en-US/docs/Web/CSS/filter) format; 

```js
graphics.pushLayer(clipAreaName[,opacity | filter])
```
Element's area clip with optional opacity. _clipAreaName_ is one of:

+ `"background-area"` - element background area accounting border-radius;
+ `"border-box"`,
+ `"padding-box"`,
+ `"margin-box"`,
+ `"content-box"`

```js
graphics.pushLayer(path [,opacity])
```

push [Path](Path.md) clip layer with optional opacity.

```js
graphics.pushLayer(mask:Image, useAlpha [,opacity] )
```

push [Image](Image.md) mask clip layer with optional opacity.

If _useAlpha_ is _true_ then alpha value is used as a mask value. Otherwise non-black pixels  define the mask.

### popLayer()

Pops layer created by most recent pushLayer().


## Static properties and methods

### fontFamilies()

```js
let listOfFontFamilies = Graphics.fontFamilies();
```
The method returns array of strings: names of font families that are installed on this machine.
