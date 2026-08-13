
# ID and class attributes in block elements

With the `option.blockIdClass=true` the engine recognizes `{#id .class}` attribute definitions **at the end of headers** as ID and class attribute defintions.

## Basic recognition

#### Heading with an ID and/or class {#myID .sample}

```````````````````````````````` example
#### Heading with a class {#myID .sample}
````````````````````````````````

Attributes recognized only at the end of headings


#### {#myID .sample} heading

```````````````````````````````` example
#### {#myID .sample} heading
````````````````````````````````

ID defintion inside `{` and `}` brackets is a name token strarting with `#`. 

Class defintion inside `{` and `}` brackets is an HTML name token strarting with `.` (dot). Multiple class definitions are supported:

#### My fancy heading {.sample .another-sample}

```````````````````````````````` example
#### My fancy heading {.sample .another-sample}
````````````````````````````````

## Opt-in behavior

With `option.blockIdClass = false` sequences like `{ ... }` are treated as a literal text.





# Inline spans with ID and class attributes

With the `option.spanIdClass=true` engine recognizes `[text of my span]{#myid .myclass}` as attribute definitions of inline spans.

## Basic recognition

Some [text of my span]{#myid .sample} inside a paragraph.

```````````````````````````````` example
.
Some [text of my span]{#myid .sample} inside a paragraph. 
.
````````````````````````````````

The engine renders such constructs as `<span>` elements: 
```````````````````````````````` example
<p>Some <span id="myid" class="sample">text of my span</span>span> inside a paragraph.</p>
````````````````````````````````

## Spans may contain other inlines

Here is [span with **bold** sub-span]{.sample} inside a paragraph.

```````````````````````````````` example
.
Here is [span with **bold** sub-span]{.sample} inside a paragraph.
.
````````````````````````````````

## Opt-in behavior

With `option.spanIdClass = false` sequences `[...]{ ... }` are treated as a literal text.



