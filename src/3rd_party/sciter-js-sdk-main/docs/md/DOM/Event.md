---
sidebar_position: 4
title: Event
toc_min_heading_level: 2
toc_max_heading_level: 5
---

# class Event

## properties:

* `event.bubbles`
* `event.cancelable`
* `event.currentTarget`
* `event.defaultPrevented`
* `event.eventPhase`
* `event.target`
* `event.type` - "click", "mousedown", etc.
* `event.namespace` - namespace portion (after the dot) of event name. Like "comp" in "click.comp" event.
* `event.data:any`
* `event.details:any` - alias of `event.data`
* `event.keyCode:integer` - key code, see include/sciter-x-key-codes.h
* `event.platformKeyCode` - platform's native key code, e.g, wParam in WM_KEYDOWN on Windows.
* `event.code` - string representation of keyCode `KeyA`, `F1`, `Enter`...
* `event.altKey` - ALT true/false
* `event.ctrlKey` 
* `event.metaKey` - `command key` on OSX, `win` on Windows
* `event.shiftKey`
* `event.isOnIcon` - mouse events, Sciter specific, _true_ if mouse is on element icon - `foreground-image` area, if it is defined by CSS.
* `event.button`
* `event.buttons`
* currentTarget relative coordinates:
  - `event.x`:number, CSS pixels
  - `event.y`:number, CSS pixels
  - `event.position`:[Point](../Graphics/Point), CSS pixels
  - `event.contentPosition`:[Point](../Graphics/Point), CSS pixels : `event.position + event.currentTarget.scrollPosition` 
* Client relative coordinates, relative to document container (window|frame):  
  - `event.clientX`:number, CSS pixels
  - `event.clientY`:number, CSS pixels
  - `event.clientPoisition`:[Point](../Graphics/Point), CSS pixels
* Window relative coordinates:
  - `event.windowX`:number, CSS pixels
  - `event.windowY`:number, CSS pixels
  - `event.windowPoisition`:[Point](../Graphics/Point), CSS pixels
* Screen relative coordinates, relative to whole desktop:
  - `event.screenX`:number, screen pixels 
  - `event.screenY`:number, screen pixels 
  - `event.screenPosition`:[Point](../Graphics/Point), screen pixels 
* Scroll (e.g. mouse wheel) deltas:
  - `event.deltaX` :number
  - `event.deltaY` :number
  - `event.delta`:[Size](../Graphics/Size)

* `event.deltaMode|int` - 0 - deltaX/Y are pixels coming from touch devices, 1 - deltaX/Y are in `lines` (a.k.a. mouse wheel `ticks`). 
* `event.relatedTarget` - only for blur|focus|focusin|focusout events, see: [relatedTarget](https://developer.mozilla.org/en-US/docs/Web/API/FocusEvent/relatedTarget).

### properties (Sciter specific):

* `event.x` - sciter specific, coordinates are relative
* `event.y` - to `event.currentTarget` - the element this event handler is attached to.
* `event.source` - used in some events to indicate auxiliary `source` element. 
* `event.isOnIcon:Element` - mouse events, it is set to element when mouse is on icon of that element. Element icon is an element's foreground-image (if any) so event.isOnIcon is on when mouse is over area where the image is rendered.
* `event.reason` - integer, event type specific flags.

### properties (Sciter specific, gestures):

- `event.delta:`[Size](../Graphics/Size) pan gesture deltas, in logical (CSS) pixels;
- `event.deltaZoom:float`  zoom gesture delta zoom multiplier: `currentScale *= evt.deltaZoom`;
- `event.deltaRotation:Angle`  zoom gesture delta angle: `currentRotation += evt.deltaRotation`;

## methods:

* `event.preventDefault()`
* `event.stopImmediatePropagation()`
* `event.stopPropagation()`

## static methods:

* `Event.keyState(key:string): true|false|undefined` 

  Returns pressed status of the _key_. Example:

  ```js
    if(Event.keyState(`CapsLock`))
      ... CAPS LOCK is on ...
  ```

* `Event.cursorPos(): [x,y]` 

  Reports current cursor position relative to desktop, example:

  ```js
  let [xpos,ypos] = Event.cursorPos();
  ```


## Known Events

### Mouse

| name | alt name | description |
| ---- | -------- | ----------- |
| **mousemove**   | **mouse-move** |
| **mouseenter**  | **mouse-enter** | 
| **mouseleave**  | **mouse-leave** |
| **mouseover**   | **mouse-over** | for the difference between mosueenter and mouseover see [MDN](https://developer.mozilla.org/en-US/docs/Web/API/Element/mouseenter_event)
| **mouseout**    | **mouse-out** | for the difference between mosueleave and mouseout see [MDN](https://developer.mozilla.org/en-US/docs/Web/API/Element/mouseleave_event#behavior_of_mouseleave_events)
| **mouseidle**   | **mouse-idle** | mouse stays not moving in the element, the event triggers tooltip show.
| **mousetick**   | **mouse-tick** | mouse is pressed for some time in element, periodic "pulse" event.
| **mousedown**   | **mouse-down** | 
| **mouseup**     | **mouse-up** |
| **wheel**       | **mouse-wheel** |
| **mousedragrequest** | **mouse-drag-request** | pressed mouse starts moving after system defined threshold 
| **singleclick** | **single-click** | \ 
| **dblclick**    | **double-click** | - either one of these events generated.
| **tripleclick** | **triple-click** | / 

### Behaviors

| name | alt name | description |
| ---- | -------- | ----------- |
| **click** | | 
| **press** | | mouse down on clickable element  
| **input** | | posted event, arrived after user changes something.
| **change** | | synchronous event, arrived on user's change and before screen update.
| **changing**  | | before change, in text editors
| **submit**  | | in `<form>`
| **reset**  | | in `<form>`  
| **expand**  | | notification that some element was expanded (opened), e.g. `option` in `<select|tree>` 
| **collapse**  | | notification that some element was collapsed (closed), e.g. `option` in `<select|tree>`  
| **statechange** | **state-change** | UI state change, e.g. caret was moved by the user, splitter moved in frameset, etc.
| **currentstatechange** | **current-state-change** | **:current** state change (select,menu).
| **disabledstatechange**  | **disabled-state-change** |
| **readonlystatechange** | **readonly-state-change** |
| **navigation** | The event is generated in response of click on a hyperlink.  `event.data` is an object `{ url:string, target:string }`. Consuming this event in sinking phase will prevent default loading of target document into window or frame.
| **contextmenu** | **context-menu** | context menu request for the element
| **contextmenusetup** | **context-menu-setup** | notification to setup context menu, context menu DO< element is event.source
| **animationend** | **animation-end** |
| **animationstart** | **animation-start** |
| **animationloop** | **animation-loop** |
| **transitionend** | **transition-end** |
| **transitionstart** | **transition-start** |
| **mediachange** | **media-change** | the event is sent to the window only when media variables have changed
| **contentchange** | **content-change** | DOM change notification: elements added or removed, attrributes changed
| **inputlangchange** | **input-lang-change** | user has switched input language
| **pastehtml** | **paste-html** | is sent by [behavior:htmlarea](../behaviors/behavior-htmlarea) on paste of HTML from clipboard
| **pastetext** | **paste-text** | is sent by [behavior:htmlarea](../behaviors/behavior-htmlarea) on paste of plain text from clipboard
| **pasteimage** | **paste-image** | is sent by [behavior:htmlarea](../behaviors/behavior-htmlarea) on paste of image from clipboard
| **popuprequest** | **popup-request** | see [popup life-cycle events](../DOM/out-of-canvas-elements#popuplifecycleevents)  
| **popupready** | **popupready**  | see [popup life-cycle events](../DOM/out-of-canvas-elements#popuplifecycleevents)     
| **popupdismissing** | **popup-dismissing** | see [popup life-cycle events](../DOM/out-of-canvas-elements#popuplifecycleevents)     
| **popupdismissed** | **popup-dismissed** | see [popup life-cycle events](../DOM/out-of-canvas-elements#popuplifecycleevents)     
| **tooltiprequest** | **tooltip-request** | see [popup life-cycle events](../DOM/out-of-canvas-elements#dynamictooltips)     

### Focus

| name | alt name | description |
| ---- | -------- | ----------- |
| **focusin** | **focus-in** | sent to a container when focus moves on a child inside it
| **focusout** | **focus-out** | sent to a container when focus moves outside of it
| **focus** | |
| **blur**  | |

### Keyboard

| name | alt name | description |
| ---- | -------- | ----------- |
| **keydown** | **key-down** | keyboard key pressed
| **keyup**   | **key-up** | keyboard key depressed
| **keypress** | **key-char** | keyboard key pressed and depressed, key code was translated to produce `event.char`
| **compositionstart** | **composition-start** | IME composition started
| **compositionend** | **composition-end** | IME composition ended

### Scroll

| name | alt name | description |
| ---- | -------- | ----------- |
| **scroll** | | is sent to scrollable element after its scroll position changed
| **scrollanimationstart** | **scroll-animation-start**  | is sent to scrollable element on scroll animation start
| **scrollanimationend** | **scroll-animation-end** | is sent to scrollable element on scroll animation end
| **scrollstepplus** | **scroll-step-plus** | click on the part of a scrollbar 
| **scrollstepminus** | **scroll-step-minus** | click on the part of a scrollbar 
| **scrollpageplus** | **scroll-page-plus** | click on the part of a scrollbar 
| **scrollpageminus** | **scroll-page-minus** | click on the part of a scrollbar 
| **scrollsliderpress** | **scroll-slider-press** | scroll slider pressed
| **scrollsliderrelease** | **scroll-slider-release** | scroll slider released

### Gestures

Set of events coming from touch devices - touchpad/trackpad or touchscreen.


| name | alt name | description |
| ---- | -------- | ----------- |
| **gesturestart** | **gesture-start** | see [Gesture Handling Initiation](#GestureHandlingInitiation)
| **gestureend** | **gesture-end** | the event is sent after last touch contact is removed. 
| **gesturepress** | **gesture-press** | the event is sent when the element is touched. For eaxample Android shows expanding circle animation on such event. 
| **gesturepinch** | **gesture-pinch** | click|zoom|rotation gesture event. 
| **gesturepan**   | **gesture-pan**   | pan (scroll) gesture event. 
| **gestureswipe** | **gesture-swipe** | swipe (fast pan) gesture event. 

#### Gesture Handling

For element to receive gesture events it shall handle the **gesture-start** event. The event is sent when first touch contact is made. 

Element that receives this event may call `element.state.wantsGestures(...)` method to define gestures it wants to receive.
  
`element.state.wantsGestures(event1,event2,...)` accepts list of events of gestures the element wants to receive:

+ `"pan-vertical"` - vertical scroll
+ `"pan-horizontal"` - horizontal scroll
+ `"zoom"` 
+ `"rotation"`

:::caution
`element.state.wantsGestures()` method is expected to be called in `gesture-start` event handler only.
:::

:::tip
If "rotation" gesture is requested use event.deltaRotation in _gesture-pinch_ handler to get delta angle.

If "zoom" gesture is requested use event.deltaZoom in _gesture-pinch_ handler to get the delta zoom.
:::


### Document lifecycle events

Loading:

| name | alt name | description |
| ---- | -------- | ----------- |
| **parsed** | **document-parsed** | document just got a DOM structure, scripts are not run yet. This event can be handled by document container only (window or frame). 
| **DOMContentLoaded** | **document-ready** | document loaded, DOM is parsed, scripts are loaded and run.
| **complete** | **document-complete** | document loaded in full, scripts were run, all resources defined in HTML are finished loading (with success or failure).

Closing:

| name | alt name | description |
| ---- | -------- | ----------- |
| **closerequest** | **document-unload-request** | first phase of document closure, it can be rejected at this point by calling `event.preventDefault()`.
| **beforeunload** | **document-before-unload** | document is about to be unloaded, script namespace is still operational.
| **unload** | **document-unload** | document is closed and about to be deleted soon.


### Element's state change

:::note
These are not a bubbling events - delivered only to the element itself.
:::

| name | alt name | description |
| ---- | -------- | ----------- |
| **sizechange** | **size-change** | change of element dimensions detected
| **visualstatechange** | **visibility-state-change** | element become visible or invisible, for example due to **visibility** value change. `event.reason` is truthy if it become visible.

### Image

| name | alt name | description |
| ---- | -------- | ----------- |
| **load** | **image-load** | image has been loaded
| **error** | **image-error** | image load error occured

### Pager (print preview)

`<frame|pager>` (print preview) related events

| name | alt name | description |
| ---- | -------- | ----------- |
| **paginationstart** | **pagination-start** | pagination has been started on the frame/pager
| **paginationpage** | **pagination-page** | pagination of one page complete
| **paginationend** | **pagination-ended** | pagination has been ended

### Drag-n-drop

System drag-n-drop events:

| name | alt name | description |
| ---- | -------- | ----------- |
| **drag** |  | D&D cursor moves over the element 
| **dragenter** | **drag-enter** | D&D cursor enters the element
| **dragleave** | **drag-leave** | D&D cursor leaves the element
| **drop** |  | User drops the data by releasing primary mouse button
| **dragcancel** | **drag-cancel** | User cancels D&D operation by pressing ESC.
| **willacceptdrop** | **will-accept-drop** | see below

#### Accepting Drag-n-Drop

For an element, in order to accept drop from system's D&D, it should handle _will-accept-drop_ event and if _event.detail_ contains acceptable data and consume the event by calling 
`event.stopPropagation()`

In D&D events event.detail is an object with the following fields

- _dataType_ - string, primary format, one of: "text","html","file" or "json". 
- _data_ - object, may contain one or several fields: 
  + `text: string` - plain text data;
  + `html: string` - HTML data; 
  + `file : [path1,path2,...] | path0` - single or multiple file names;
  + `json`: any - any data that can be JSON.stringify'ed;

#### Initiating Drag-n-Drop

Call [window.performDrag(...)](Window#performdrag) to initiate D&D operation.

```js
event.detail = { dataType: ...; data: {} }
```

### Video

`<video>` related events

| name | alt name | description |
| ---- | -------- | ----------- |
| **videoready** | **video-ready** | 
| **videostart** | **video-start** | 
| **videostop** | **video-stop** | 
| **videocoordinate** | **video-coordinate** | 
| **videoframeready** | **video-frame-ready** | 

## MISC

- [Event handling in Sciter](https://sciter.com/event-handling-in-sciter/)
