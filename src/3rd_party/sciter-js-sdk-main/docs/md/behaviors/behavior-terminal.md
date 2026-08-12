# behavior: terminal

This helper behavior implements ANSI-terminal viewer.

The behavior maintains and renders 2D array of characters (screen buffer) of terminal.rows and terminal.columns size.

## Elements

By default the behavior is applied to `<terminal>` element (Sciter specific)

```html
  <terminal />
```

## Attributes

* `rows` - integer, number of rows in terminal buffer  (lines in scrollable buffer) ;
* `columns` - integer, number of columns in terminal buffer (like characters in screen width);
* `selectable` - boolean, if defined then the terminal will support selection by mouse;

If `columns` attribute is not defined then number of columns is calculated according current width of the element. 

Maximum value of `rows` and `columns` is set to 3000. 

## Events

Together with the standard set of events (mouse, keyboard, focus) *behavior: terminal* generates:

* `"change"` - generated when content of terminal buffer changes, e.g. after `terminal.write(text)` call or change of number of columns or rows.
* `"statechange"` - generated on caret position change.
* `"bell"` - generated when `terminal.write(text)` encounters the bell ASCII character (`\a` - code 7).  


## Value

N/A

## Methods

### `element.terminal.write(text: string)`
  
  Puts the text at current caret position. 

  Updates current position to be at the end of the text.

  The text may contain ASCII control codes. 

### `element.terminal.read([row: int, row: column[, maxLength:int]])`
  
  Reads content of buffer at current caret position or row/column. 

### `element.terminal.resize(rows: int, columns: int)`
  
  Resizes internal terminal buffer to be of _rows_ and _columns_ size.

  _rows_ and _columns_ are internally limited by 3000. 


### `element.terminal.clear()`
  
  Clears content of internal terminal buffer.

## Properties

### `element.terminal.rows: int`
  
  read-only, actual rows in terminal buffer.

### `element.terminal.columns: int`
  
  read-only, actual columns in terminal buffer.

### `element.terminal.caretRow: int`
  
  read-only, caret location inside the buffer, row.

### `element.terminal.caretColumn: int`
  
  read-only, caret location inside the buffer, column.
