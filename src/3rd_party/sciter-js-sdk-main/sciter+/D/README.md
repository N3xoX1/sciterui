
# Sciter SDK for [D language](https://dlang.org/)

The SDK is implemented in pure D - it does not use C/C++ files for operation. It loads Sciter API explicitly from sciter.dll(so,dylib), see /api.d file.

## List of entities in sciter.*** namespace

### Application

Used inside `main()` function to manage life-cycle of the app: initialization, shutdown and "message pump" loop.

### Window

Class that represents desktop window with loaded HTML/CSS/JS. Inherits from dom.events.EventHandler.

### dom.Element

DOM Element, same entity as [W3C Element](https://developer.mozilla.org/en-US/docs/Web/API/Element). Sciter UI is just a DOM tree of Elements and Nodes.

### dom.Node

DOM Node, represents Text, Comment and Element nodes, same entity as [W3C Node](https://developer.mozilla.org/en-US/docs/Web/API/Node).  

### dom.ElementController

class-controller of DOM elements, inherits from dom.events.EventHandler. 

Allows to define custom DOM elements.

### dom.events.EventHandler

class and associated event descriptions. DOM events handling infrastructure. 

Note that events in Sciter are using [sinking/bubbling mechanism](https://javascript.info/bubbling-and-capturing) - events are "walking" from parent to child (sinking) and then backward (bubbling).

### util.archive

Module that supports mechanism of storing and accessing resources inside application binary. SDK contains packfolder.exe utility that allows to pack content of a folder on disk into an embeddable blob. `util.archive.get(path)` allows to fetch individual items from the archive.

### om.VALUE

A.k.a. discriminated union. Wraps data of various types: int, long, string, arrays, hashmaps, etc.

It also may hold : 

* references to JS objects to access them by D code;
* references to D delegates to be callable from JS side.

Essentially this the "lingua franca" of D `<->` JS interaction.

### graphics.Graphics

Represents GPU accelerated 2D graphic primitives. This struct allows to:

* implement [immediate mode painting](https://en.wikipedia.org/wiki/Immediate_mode_(computer_graphics)) - drawing at the surface used by DOM rendering and synchronously to it.

  This gets best of two worlds: declarative and immediate mode UIs;

* draw on an Image surface. Such image can be used in CSS or be drawn by other Graphics insatnce later.

### graphics.Image

Represents jpeg, png, webp, svg, etc. Images can be loaded from ubyte[]'s and saved (serialized) to supported formats.

An image can be constructed from a DOM element making runtime snapshot of it. Can be used for implementing caching and the like.

### graphics.Text

In Sciter it represents block of text that can be used for drawing on graphics. Text instances can be styled by CSS by inline style declarations and class based CSS rules. 

### graphics.Path

Represents 2D paths - vector graphics constructs. Paths can be stroked, filled or used as a clips.


## dsciter.d "Hello World" demo

Demonstrates: 

* Basic app strucutre: `main()` function with Application use;
* Use of resources by binding them to archive instance;
* Window creation and loading content to it.
* Custom `<clock>` element controller (sdk/sciter+/D/samples/drawingcontroller.d).
* Exposing D objects and functions to JS.
* Handling of events originated in DOM or sent/posted from JS explicitly.

### "Hello World" demo compilation

Use one of `build-***` scripts in sdk/sciter+/D/demos/hellod for your platform (Win|Mac|Lnx).

DMD or LDC needs to be [installed](https://dlang.org/download.html) on the machine.







