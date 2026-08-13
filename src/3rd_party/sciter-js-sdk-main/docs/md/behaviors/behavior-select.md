# behavior: select

Standard behavior of `<select>` lists. In principle it can be applied to any DOM element.

## Elements

These elements have behavior:select applied by default:

* `<select size="2...N"></select>` - single select;
* `<select|list></select>` - single select;
* `<select|list multiple></select>` - multy select;
* `<select|list multiple="checkmarks"></select>`  - multy select with checkmarks;
* `<select|tree></select>` - hierarchical single select;
* `<select|tree multiple="checkmarks"></select>`  - hierarchical multy select with checkmarks;

## Model

The `<select>` may contain `<option>` elements contained in arbitrary markup.

The behavior treats `<option>'s` or any other DOM element with attribute `role="option"` as a selectable entity.

Selected option will have `:checked` state flag set.

Examples, simple select:

```XML
<select>
  <option value="#ff0000" selected>Red</option>
  <option value="#00ff00">Green</option>
  <option value="#0000ff">Blue</option>
</select>
```

and `<table>` behaving as a select:

```XML
<style>
  table.select > tbody { behavior:select; }
  table.select > tbody > tr:current { color:white; background:blue; }
</style>

<table.select>
  <tbody>
    <tr role="option" value="#ff0000"><td>Red</td><td>#FF0000<td></tr>
    <tr role="option" value="#00ff00"><td>Green</td><td>#00FF00<td></tr>
    <tr role="option" value="#0000ff"><td>Blue</td><td>#0000FF<td></tr>
  </tbody>
</table>
```

`<option>`s in the `<select|tree>` may contain other sub `<option>`s:

```XML
<select>
  <option expanded>
    <caption>Group A</caption>
    <option value="#ff0000">Red</option>
    <option value="#00ff00">Green</option>
    <option value="#0000ff">Blue</option>
  </option>
  <option>
    <caption>Group B</caption>
    ...
  </option>
</select>
```

## Attributes

This behavior knows about:

* `size=integer` - number of visible elements in the list. Note: height of the select element can be overridden by CSS.
* `name="name"` - standard attribute *name* - name of the input element on a form.
* `novalue="text"` - if select has no `<option selected>` initially it will have this text rendered.
* `as="type"` - defines `<option value="...">` interpretation rule, accepts:
  * `as="auto"`, tries to parse option's value as integer, float, boolean or length value. If parsing fails it returns value as a string.
  * `as="integer"`, tries to parse option's value as integer. If parsing fails it returns value as a string.
  * `as="float"`, tries to parse option's value as float. If parsing fails it returns value as a string.
  * `as="numeric"`, tries to parse option's value as either integer or float. If parsing fails it returns value as a string.
  * `as="string"`, default value, does not parse option's value. Returns value as a string.
* `multiple`, select supports multiple selected options at the same time. 
* `multiple="checkmarks"`, select supports multiple selected options at the same time with dedicated checkmarks for options selection.
* `treelines"`, `<select|tree>` only, hierarchical list draws "tree lines" for better grouping visualization.  

## Attributes of `<option>` elements inside the select:

* `value="..."` - value of the option. If ommited then innerText of the option used as a value (string);
* `selected` - defines initial value of `:current`/`:checked` state flag of the option. Selected option[s] define initial value of the select.
* `expanded` - defines initial `:expanded` state of group `<option>` element in a `<select|tree>`.

## Events

Other than standard set of events (mouse, keyboard, focus) behavior:select generates:

* `"change"` event, generated when user changes selection of the select (click on one of options). Posted event.
* `"expanded"` event, group option was expanded by the user;
* `"collapsed"` event, group option was collapsed by the user;

## Properties

Other than standard properties of DOM elements the select also supports:

* `options` - reference to DOM element that holds `<option>` list, in this case this is the element itself.
* `select.currentOption` - Element, read/write, reference to DOM element that represents current option.

## Methods

* `select.optionByValue(val): Element` returns DOM element that represents option with associated _val_ value. Returns null if no such option found.

## Commands

Note: commands are invoked by calling `element.execCommand("cmd-name"[,params])`

* **"set-current"** - when the command is sent to an `<option>` inside the select it causes the option to be current;


## Value

* for single select, scalar value of type determined by `as` attribute, read/write. value of selected option. Value of the option is its value attribute or option's text if there is no value defined.
* for multy select - array of values of selected options. Type of elements of the array is determined by by `as` attribute.

## Selection change handling in script

### raw `onchange` handler

```js
var sel = document.$("select#some");
sel.onchange = function() { var v = this.value; ... }
```

### `on()` subscription

```js
var sel = document.$("select#some");
sel.on("change", function() { ... event handling code ... });
document.on("change", "select#some", function() { ... event handling code ... });
```
