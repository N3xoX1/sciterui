---
sidebar_position: 5
title: Window
toc_min_heading_level: 2
toc_max_heading_level: 5
---

# class Window

Instances of the Window class represent desktop windows.

`Window.this` is a reference to current window object, that is the instance of the `Window` class where the HTML document is loaded.

NOTE: the _window_ below is an instance of Sciter's Window class - e.g. `Window.this` but not that strange "window" thing of browsers. 

## constructor:

```js 
new Window({params:object})
```

where `params` is an object with properties:

+ `type` - optional, is one of:
  
  + `Window.POPUP_WINDOW`
  + `Window.TOOL_WINDOW`
  + `Window.CHILD_WINDOW`
  + `Window.FRAME_WINDOW` - default window type
  + `Window.DIALOG_WINDOW`

+ `parent` : Window - optional, instance of parent (owner window). When owner is closed or minimized then this window will be closed/minimized too. _parent_ defines z-order of this new window. Window with the defined _parent_ will always float on top its parent.
+ `caption` : string - optional, window caption (or title).
+ `x` : integer - optional, screen pixels, horizontal position of the window on screen;
+ `y` : integer - optional, screen pixels, vertical position of the window from the top of the screen;
+ `width` : integer - screen pixels, window width;
+ `height` : integer - screen pixels, window height;
+ `client` : true | false - if `true` then x,y,w,h are coordinates of desired window client box on the screen;
+ `alignment` : integer - optional, \[1 to 9\] alignment of the window on monitor, if \[-1 to -9\] and parent is provided then it aligns the window against parent window. (1 bottom left corner, 2 bottom middle, 3 bottom right corner, 4 middle left, 5 center, 6 middle right, 7 top left corner, 8 top middle, 9 top right corner)
+ `screen` : integer - optional, number of monitor on multi-home systems.
+ `state` : - optional - window state, is one of:

  + `Window.WINDOW_SHOWN` - default state
  + `Window.WINDOW_MINIMIZED`
  + `Window.WINDOW_MAXIMIZED`
  + `Window.WINDOW_HIDDEN`
  + `Window.WINDOW_FULL_SCREEN`

+ `url` : string - optional, window html source code file.
+ `parameters` : array | string | object, ... - optional, extra parameters to pass to the new window.

## properties:

### focus

read/write property, either _null_ or an [element](Element) in focus;

### state
 
`window.state` - read/write, integer, one of:

- 1 `Window.WINDOW_SHOWN` - window is shown normally;
- 2 `Window.WINDOW_MINIMIZED` - window is collapsed to icon/tray, also known as "iconified" state; 
- 3 `Window.WINDOW_MAXIMIZED` - window spans whole working area  (excludes taskbar, etc) of the desktop;
- 4 `Window.WINDOW_HIDDEN` - window is hidden (not visible);
- 5 `Window.WINDOW_FULL_SCREEN` - window spans whole desktop area, so appears on top of all other windows including taskbar, etc;
- 0 `Window.WINDOW_SHOWN_NA` - write only value, use it to show window without activating it (setting focus on it);

### frameType

`window.frameType` - read/write, string, frame type of the window, one of:

- `"standard"` - default frame type;
- `"transparent"` - transparent (a.k.a. layered) window;
- `"solid"` - non-transparent, rectangular window without any decorations;
- `"solid-with-shadow"` - non-transparent window without caption and buttons, may have rounded corners and shadow if OS supports that.
- `"extended"` - standard window shape but without caption bar. Caption and window buttons(exception:MacOS) shall be defined in markup/css. 


### caption

read-write, string, window's caption (title).

### icon

read/write, [Graphics.Image](../Graphics/Image), window icon.

### screen

read-only, integer. Reports screen (monitor) index this window is on at the moment. Integer in the range [0 ... Window.screens).

### graphicsBackend

read-only, string. Reports current graphics backend used: "direct2d", "Skia/OpenGL", etc. 

### minSize, maxSize

+ `window.minSize = [w,h]` - get/set minimal size of resizable window. 
+ `window.maxSize = [w,h]` - get/set maximum size of resizable window.

### blurBehind

`window.blurBehind = "..."`  - read/write, string, blur-behind effect configuration string, either: 
  
- `"none"` - no blur behind effect;
- `"dark|light|auto [ultra] [source-auto|source-windows|source-desktop]"` - flags composition string, where:
    - `"source-windows"` - blur of all windows behind this one;  
    - `"source-desktop"` - blur of desktop background image - so called _Mica_ effect;
    - `"source-auto"` - default, blur source is chosen according to window type;  

Example: W11 Mica light effect window: `window.blurBehind = "light source-desktop"`; 

### isActive

read-only, boolean, reports if window has input focus.

### isAlive

read-only, boolean, it is _true_ if the window is alive - has valid HWINDOW. The property is false when the window was closed and destroyed.

### isOnActiveSpace

read-only, _true_ if window is on active space now. The property is _undefined_ if host system does not support spaces (virtual desktops).

### isResizable

read/write, boolean, true if window can be resized by the user.

### isMaximizable

read/write, boolean, true if window can be maximized by the user.

### isMinimizable

read/write, boolean, true if window can be minimized by the user.

### isTopmost

read/write, boolean, true if window is topmost at z-order.

### isEnabled

read/write, boolean, true if the window is allowed to accept user's input.

### aspectRatio

read/write, float, width to height ratio to keep on window resizes.

### eventRoot
 
```js
 Window.this.eventRoot = element | null
 ```
if set by element, short circuits all UI events to that element and its children as if the window contains only that element. Used in lightbox dialog scenarios (see: samples.sciter/lightbox-dialog).


### focus

read/write, DOM element in focus.

### parent

read-only, Window | null - parent window of this one.

### document

read-only, Document - root document of the window.

### parameters

read-only, any - parameters provided by constructor, available inside the window as they are.

## methods:

### box()

```js
window.box(boxPart,boxOf[,relTo[, asPPX : bool ]]):[...]
```

Reports geometry of the window.

parameters:
  
* _boxPart_ defines what part of the box to return, is one of:
  
  * `"xywh"` or `"rectw"`  - [x,y,w,h], array, position and dimension of the rectangle.
  * `"rect"` - [x1,y1,x2,y2], array, positions of origin and corner of the rectangle.
  * `"position"` - [x,y], array, position of the rectangle.
  * `"dimension"` - [w,h], array, dimension of the rectangle.
  * `"left"`,`"top"`,`"right"`,`"bottom"`,`"width"`,`"height"` - individual integers.

* _boxOf_ is one of:

  + `"border"` - border area of the window that includes window caption and borders around client area;
  + `"client"` - client (content) area of the window;
  + `"cursor"` - mouse cursor position, like `let [cx,cy] = Window.this.box("position","cursor","desktop");`
  + `"caret"` - caret position, relative to client area of the window;

* _relTo_ is one of:

  + `"desktop"` - coordinates are relative to desktop (outline of all monitors in the system);
  + `"monitor"` - coordinates are relative to the monitor this window is replaced on;
  + `"self"` - coordinates are relative to the origin of window's client area;
 
* _asPPX_ optional, is a boolean:

  + `true` - coordinates are in physical device pixels; 
  + `false` - coordinates are in CSS pixels - 1/96 of inch;

### screenBox()

```js
window.screenBox(what [, boxPart [,asPPX]])
```

Reports geometry of monitor this window is on.

parameters:

* _what_ defines what information to return, is one of:

  + `"frame"` - physical position and size of the monitor in screen pixels projected on desktop.
  + `"workarea"` - physical position and size of work area on the monitor ( frame minus taskbar )
  + `"device"` - string, name of the monitor.
  + `"isPrimary"` - boolean, true is that is primary monitor.
  + `"snapshot"` - Graphics.Image, returns snapshot (screenshot) of the monitor.

* _boxPart_ defines what part of the box to return, is one of:

  + `"xywh"` or `"rectw"`  - [x,y,w,h], array, position and dimension of the rectangle.
  + `"rect"` - [x1,y1,x2,y2], array, positions of origin and corner of the rectangle.
  + `"position"` - [x,y], array, position of the rectangle.
  + `"dimension"` - [w,h], array, dimension of the rectangle.
  + `"left"`,`"top"`,`"right"`,`"bottom"`,`"width"`,`"height"` - individual integers.
  
* _asPPX_ is a boolean:

  + `true` - coordinates are in physical device pixels; 
  + `false` - coordinates are in CSS pixels - 1/96 of inch;

### cursorPos()

```js
const [x,y] = window.cursorPos();
```

Reports current position of cursor relative to client area of the window.

---

### move()

```js
window.move(x, y [,width, height [, "client" ]])
```

move/size window.  

parameters:
_x_, _y_, _width_, _height_ are in PPX (physical screen pixels).

If _"client"_ is provided then _x_, _y_, _width_, _height_ are treated as window client area coordinates.

### moveTo()

```js
window.moveTo(monitor, x, y [,width, height [, "client" ]])
```
  
move/size window to particular monitor;

_x_, _y_, _width_, _height_ are in DIPs - device independent pixels (a.k.a. CSS pixels).

### shape()

```js
window.shape( Graphics.Path | Element )
```

Defines shape of the window. Points out of the path are visually transparent and click-through.

If _element_ is provided as a parameter then the path is constructed from non-transparent areas of the element and/or sub-elements.

---

### selectFile()

```js
window.selectFile({params:object})
```
   
file open/save dialog, params is an object with the fields:

+ `mode` : "save"|"open"|"open-multiple"
+ `filter` : "title|ext1;ext2", `"HTML File (*.htm,*.html)|*.html;*.htm|All Files (*.*)|*.*"`
+ `extension` : default file extension, "html"
+ `caption` : title of dialog, "Save As"
+ `path` : initial directory

returns:
- string, file URL of selected file in "open" and "save" modes;
- array(string), list of selected files;
- _null_ if selection was cancelled by the user. 

### selectFolder()

```js
window.selectFolder({params:object})
```

folder open dialog, params is an object with the fields:

+ `caption` : title of dialog
+ `path` : initial directory

returns string - folder URL;

---

### mediaVar()

```js
window.mediaVar(varname[,value])
```

gets/sets media variable that can be used in CSS as `@media varname {...}`

### mediaVars()
   
```js
window.mediaVars([values:object])
```

gets/sets multiple media variables at once. 

---

### addEventHandler()
### on()
```js
window.on("eventname", handler)
```
subscribe to [window related events](#events).

:::tip
_eventname_ may contain namespace suffix: "move.mycomponent"
:::

### off()

```js
window.off("eventname" | handler)
```
  
unsubscribe event handler either by name, namespace or handler reference

### dispatchEvent()

```js
window.dispatchEvent(event):boolean
```
  
Send the event to the window synchronously. Returns _true_ if any of event handlers consumes the event.

### postEvent()

```js
window.postEvent(event)
```
  
Post the event to the window asynchronously. The function returns immediately - does not wait for the event consumption.

---
### load()

```js
Window.this.load(url:string)
```
Loads new document into the window. 

:::tip
Almost always it is better to do not use this method but to use `<frame>` inside the window and load needed document there:
```js
document.$("frame.content").src = url;
```
:::

---

### xcall()

```js
window.xcall(name:string [, arg0, ..., argN]): any
```

Interaction with native behaviors attached to the window. `window.xcall("foo")` will end up in [`handle_scripting_call()`](https://gitlab.com/c-smile/sciter-js-sdk/blob/main/include/sciter-x-behavior.h#L749) of native behavior attached to the window using [SciterWindowAttachEventHandler](https://gitlab.com/c-smile/sciter-js-sdk/blob/main/include/sciter-x-behavior.h#L898) API.

---

### doEvent()

```js
window.doEvent(mode) : any
```
  
Performs system event(s) in application message queue, _mode_ is one of:

* "wait" - waits for the next event and executes it;
* "noWait" - if next event is available executes it otherwise returns immediately;
* "untilMouseUp" - executes events until _mouseup_ event arrives, used for various drag cases;
* "untilQuit" - performs run loop - executes all events until application quit message arrives;
* "I/O" - performs events associated with I/O;

### modal()

```js
window.modal(JSX) : any
```
shows predefined message boxes: 
- `<info>..</info>`, 
- `<alert>..</alert>`, 
- `<error>..</error>`, 
- `<question>..</question>`.
  
```js
window.modal({params}) : any
```
shows new window as dialog, for params see [`new Window({params})`](#constructor). The function returns window close value of `window.close(valToReturn)` call inside the window. 

```js
window.modal(new Window(...)) : any
```
shows already constructed window modally.

---

### performDrag()

```js
window.performDrag(data:object, mode, dragIcon[, dragIconXoff, dragIconYoff] ): null | string
```  
Performs drag-and-drop using system D&D mechanism.

parameters:

- _data_ - object, may contain one or several fields: 
  + `text: string` - plain text data;
  + `html: string` - HTML data; 
  + `file : [path1,path2,...] | path0` - single or multiple file names;
  + `json`: any - any data that can be JSON.stringify'ed;
- _mode_, string, either "copy" or "move";
- _dragIcon_, either [Image](../Graphics/Image) or [Element](Element);
- _dragIconXoff_, _dragIconXoff_ - integer, icon anchor point offset.

returns:
- _null_ - user has canncelled D&D operation by pressing Escape.
- _"copy"_ - user completes copy operation (D&D + CTRL);
- _"move"_ - user completes move operation;

---

### focusable()

The function allows to enumerate elements in TAB order.

```js
window.focusable(dir [,reference:element]): element
```

parameters:

- _dir_, string, one of:
 
  * "next" - next focusable element after the _reference_;
  + "prior" - previous focusable element after the _reference_;
  + "first" - first focusable DOM element on the window;
  + "last" - last focusable DOM element on the window;

 - _reference_ - optional, must be a focusable element with either: 
   + tabindex >= 0 or 
   + `element.state.focusable = true` or 
   + `STATE_FOCUSABLE` in native code.

:::tip
You can assign the found element to `window.focus = element` set the focus on it.
:::

### close()

```js
window.close([value]): bool
```
  
request to close the window, the value is a return value used in modal dialogs;

### update()

```js
window.update()
```
request calculation of layout of elements inside the window. Positions of elements should be known after the call.

### activate()

```js
window.activate(bringToFront: bool)
```
set input focus on the window.

---

### trayIcon()

```js
window.trayIcon({image: Graphics.Image, text: string})
```
   
Show tray icon with the image and tooltip text.

Tray icon will generate ["trayiconclick"](#trayiconclick) event on user clicks.

```js
window.trayIcon("remove")
```
removes tray icon.

```js
const [x,y,w,h] = window.trayIcon("place");
```  
reports location of the icon on desktop, coordinates are in screen pixels.

---

### requestAttention()

```js
window.requestAttention("info" | "alert" | "stop")
```

requests user attention by flashing or bouncing window icon in task/dock bar. 

---

### addHotKeyHandler()

```js
window.addHotKeyHandler(keyCombination:string, callback:function):id
```
  
Registers global system hotkey hook. hotkey handlers allows to handle key combinations when the window is not in focus or even hidden.

The function throws an error if the system rejects the hotkey, for example if that hotkey is already assigned to other application.

_keyCombination_ is a `+`  combination of optional modifiers and key name (event.code names) like: `"F5"` and `"Control+KeyN"`. Supported modifiers:

+  `Control` or `Ctrl`; 
+  `Command` or `Cmd` - CMD key on MacOS and Win key on Windows;
+  `Alt`;
+  `AltGr`;
+  `Shortcut` - is CMD key on MacOS and Ctrl on other OSes;
+  `Shift`;
  
The function returns an id that can be used to unregister hot key.

:::warning
this function is supported only on Windows for a while as only this OS has official hotkey API.
:::


### removeHotKeyHandler()
   
```js
removeHotKeyHandler(id)
```
Removes hotkey handler set by `addHotKeyHandler`.

### isOnActiveSpace

_true_ is the window is on active virtual desktop now.

## class methods and properties:

### this

```js
Window.this
```

property, instance of Window class - reference of the window where this code is running;

### all

```js
Window.all: Array<Window>
```
List of all Sciter windows in current process (including Window.this). Used for enumeration of windows:

```js
for(let wnd of Window.all)
   ...
```

### share

```js
Window.share: Object
```

property, is an object shared between all documents and windows in the application.

:::warning
Use it responsibly. If temporary window or document populates shared object then it should clean the stuff there in _beforeunload_.
:::    

---

### screens

```js
Window.screens: integer
```

returns number of screens (monitors) in the system;

### screenBox() {#nth-screenBox}

```js
Window.screenBox(screen:integer, what[, boxPart])
```

reports geometry and information of the given _screen_ (monitor). For _what_ and _boxPart_ parameters see [window.screenBox()](#screenbox) method above.

Additionally _what_ supports "devicePixelRatio" value, in this case the function returns the ratio of the resolution in physical pixels to the resolution in CSS pixels for the given monitor.

---

### elementAt()

```js
Window.elementAt(screenX,screenY):Element
```
  
returns DOM element under screenX/screenY position. 

:::note
This method may return DOM element belonging to any Sciter window in current process. 
:::

---

### ticks()

```js
Window.ticks():milliseconds
```

returns value of internal timer in milliseconds. 

---

### post() {#Window-post}

```js
Window.post( ge: Event )
```

posts global event *ge* to all windows in current process.

### send() {#Window-send}

```js
Window.send( ge: Event ) : boolean
```

Synchronously sends global event *ge* to all windows in current process.

Sending stops on first window that will consume the event by returning _true_ from event handler of this event.

Returns _true_ if some event handler consumes the event.

## events

Use `window.on("eventname", handler)` to subscribe to these events.

### "statechange"

`window.state` flag has changed.

### "closerequest"

Window closure requested.

JS code can prevent window closure by the user by calling `event.preventDefault()`: 
```js
const CLOSE_BY_CHROME = 0; // user clicked `🗙` on window chrome
const CLOSE_BY_CODE = 1;   // window.close() issued
const CLOSE_BY_LOAD = 2;   // document unload-old/load-new

Window.this.on("closerequest", event => {
  if(event.reason == CLOSE_BY_CHROME) {
    // instead of closing we just minimize it:
    Window.this.state = Window.WINDOW_MINIMIZED;
    event.preventDefault(); // prevent closing the window
  }
});
```

### "resolutionchange"

after window moved to another monitor with different resolution, or user have changed screen resolution.

### "mediachange"

one or several CSS media variables have changed.

### "activate"

the window was deactivated (evt.reason == 0) or got focus (evt.reason > 0).

### "replacementstart"
### "replacementend"

the user has started/ended moving or resizing window frame.

### "move"

user have moved the window.

### "size"

user have changed size of the window.

### "trayiconclick"

the user has clicked on tray icon.

### "spacechange"

Space (a.k.a. virtual desktop) has changed. Use `Window.this.isOnActiveSpace` property to determine if window is on active space now. 

### "system-suspend"

The system is about to enter a suspended state (e.g., sleep mode).

### "system-resume"

The system has resumed operation after being suspended.

### "system-lock"

The desktop session has been locked.

### "system-unlock"

The desktop session has been unlocked.

### "system-logon"

A user logs on to a desktop session.

### "system-logout"

A user has logged off the session.

### "system-shutdown"

A user chooses to end the session.

