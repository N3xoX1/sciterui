---
sidebar_position: 5
title: Styles, Events and Mounting points
toc_min_heading_level: 2
toc_max_heading_level: 5
---

# Defining Default Style of a Component

## By external CSS file

Normally you define styles globally in your application. But some components may require some styling (e.g. particular layout) that is critical for their operation.


Reactor (Sciter's internal code in fact) offers simple way to define component styles using *styleset* attribute.

Let's redefine our `Clock` class that we used earlier :


```js
class Clock extends Element 
{
  time = new Date(); // setting initial state 

  componentDidMount() {
    this.timer(1000, function() {
      this.componentUpdate { time:new Date() };
      return true; // to keep the timer ticking
    });
  }

  render() {
    return <clock styleset={__DIR__ + "styles.css#clock"}>
      <div.greeting>Hello, world!</div>
      <div>It is {this.data.time.toLocaleTimeString()} now.</div>
     </clock>;
  } 
}
```

Note: `styleset={__DIR__ + "styles.css#clock"}` attribute above, it resolves to absolute URL using this script file location as a base.

And here is a content of clock.css file:

```CSS
@set clock 
{
  :root {
    display: block;
    flow:vertical;
  }
  span.time { 
    display:inline-block; 
    white-space:nowrap; 
  }
}
```

As you see it defines independent set of CSS rules that define content (and only content) of our clock component and its internals.

General consideration: if component is designed to be used in many applications then its default style set should include only rules needed for layout and those are critical for operation. Each application may add its own styling on top of these default styles.

Note that style sets in Sciter are not polluting global list of style rules and so are very effective - reduce time needed for style resolution of DOM elements.

## By embedded CSS style set declaration

Instead of declaring component styles in separate CSS file we can declare styles in the same JS file using `CSS.set` constructor.

```js
const clockStyles = CSS.set` 
  :root {
    display: block;
    flow:vertical;
  }
  span.time { 
    display:inline-block; 
    white-space:nowrap; 
  }
`;

// rest is from the above sample

class Clock extends Element 
{
  time = new Date(); // setting initial state 

  ...

  render() {
    return <clock styleset={clockStyles}>
      <div.greeting>Hello, world!</div>
      <div>It is {this.data.time.toLocaleTimeString()} now.</div>
     </clock>;
  } 
}
```
Note how style set is assigned to the component by `<clock styleset={clockStyles}>`.  

## Events Handling in Components

In contrast with ReactJS Sciter does not require any special constructs for handling events - we can use normal (for the Sciter) event handling definitions in classes.

Sample of small component that encapsulates search block enclosing `<input>` and `<button>` in one entity:

```js
class Search extends Element {

  render() {
    return <search>
        <input|text />
        <button.do/>
    </search>;
  }

  ["on click at button.do"](evt, button) { 
    this.post(new Event("do-search", {data: this.$("input").value} )); 
  }
  ["on change at input"](evt,input) { 
    this.showSuggestionsFor( input.value ); 
  }

  get value() { return this.$("input").value; }
  set value(nv) { this.$("input").value = nv;

}
```

In this example we are handling two events: click on the button and change of the input.


Word about event handling functions like `["on change at input"](evt,input)` above.

That format is a standard JS/ES2020 way of defining functions with [computable names](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/Method_definitions#computed_property_names) or names that include namespaces. It is just in Sciter event handlers are using this form to describe event handling functions that have the following signatures:    

```js
["on eventname"](event) {}
["on eventname at selector"](event, selectorElement) {}
["on eventname of selector"](event, selectorElement) {}
```

Where:  

* **on**\[space\] part marks the function as event handler;     
* *eventname* is a name of event - either standard HTML's one like click, input, focus, ... or custom event name;
  * the *eventname* may start with `^` symbol like `^click` to handle event in sinking phase (before the event will be dispatched to children).
  * the *eventname* may contain namespace suffix like `click.mynamespace`.
* \[space\]**at**\[space\], if present, signifies that selector will follow;
  * *selector* here is a CSS selector of a child element **inside** this component. When the event handler will be triggered *selectorElement* argument of the function will get reference to the selector matching child and `event.target` will contain reference to the element that generated the event.
* \[space\]**of**\[space\], if present, signifies that selector will follow;
  * *selector* here is a CSS selector of a child element **that generates** the event. When the event handler will be triggered *selectorElement* argument of the function will get reference to that matching child elemt.   

Such class based event handlers are especially effective in cases when there are many elements at the same time on screen. Such event subscription schema does not require addEventHandler calls for each element.

## HTML Resident Mounting Points

Reactor Components are executable entities even if they look as HTML.

Normally in ReactJS you see JSX code only inside `<script>` sections and JS code files like here:

```js
function App() {
  return
    <main>
       <Welcome name="Ivan" />
       <Welcome name="Olga" />
       <Welcome name="Andrew" />
    </main>;
}

document.body.patch( <App /> );
```

While this works in general it may look non-natural or inconvenient to someone.

Alternatively Sciter offers special `<reactor>` HTML element as a mounting point:

```html
<body>
   <p>Test of Tabs component.</p>
      
   <reactor type="Tabs" src="tabs.js">
     <tab name="first" label="First tab">First tab content</tab>
     <tab name="second" label="Second tab">Second tab content</tab>
     <tab name="third" label="Third tab" src="tab-content.htm"></tab>
   </reactor>

</body>
```

The `<reactor>` element expects two attributes:

* *type* - class name of the component and 
* *src* - url of the script file where that component is defined.  

Please note that, while it looks like HTML, content between `<reactor>` and `</reactor>` is parsed by script rules (JSX in this case). Essentially you may think as the whole `<reactor>` section is just `<script>` element (pseudo code):

```html
<body>
   <p>Test of <Tabs> component.</p>
      
   <script component="Tabs" src="tabs.js">
     <tab name="first" label="First tab">First tab content</tab>
     <tab name="second" label="Second tab">Second tab content</tab>
     <tab name="third" label="Third tab" src="tab-content.htm"></tab>
   </script>

</body>
```

The only major difference of the `<reactor>`: it is a placeholder element - as soon as component gets instantiated it **replaces** the `<reactor>` DOM element. Therefore final DOM after `<reactor>`'s *execution* will look like:

```html
<body>
   <p>Test of <tabs> component.</p>
      
   <tabs>
     <tab name="first" label="First tab">First tab content</tab>
     <tab name="second" label="Second tab">Second tab content</tab>
     <tab name="third" label="Third tab" src="tab-content.htm"></tab>
   </tabs>

</body>
```