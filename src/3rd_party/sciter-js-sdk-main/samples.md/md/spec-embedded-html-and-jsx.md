# Embedded HTML

## HTML blocks

HTML block should start with a valid html element header at first non-whitespace position and end with an empty line:

<div style="border:1px solid">
  This is embedded div element!
</div>

```````````````````````````````` example
<div style="border:1px solid">
  This is embedded div element!
</div>
.
````````````````````````````````

## HTML inlines

HTML inline elements shall start and end in the same line:

This is some text with <kbd style="color:red">KBD</kbd> inline span.

```````````````````````````````` example
This is some text with <kbd style="color:red">KBD</kbd> inline span.
.
````````````````````````````````

## Embeddeded JSX

If markdown is instantiated by using `Markdown.toFragment(markdown, options)` method then the _markdown_ may contain Reactor components with the same rules as block/inline HTML:

<TestComponent />

```````````````````````````````` example
<TestComponent />
.
````````````````````````````````
