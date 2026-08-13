
# Highlight Spans

With the flag `options.highlights=true`, the engine enables recognition of inline
highlight spans using the `==text==` syntax. A pair of equals signs on each
side wraps the content, which the HTML renderer outputs as `<mark>`.


## Basic recognition

==Hello, world!==

```````````````````````````````` example
==Hello, world!==
.
--fhighlight
````````````````````````````````

Highlight spans may appear inline alongside normal text:

This is ==very important== text.

```````````````````````````````` example
This is ==very important== text.
.
````````````````````````````````

Multiple independent highlight spans may appear in the same paragraph:

==first== and ==second==

```````````````````````````````` example
==first== and ==second==
.
````````````````````````````````

## Flag required

With the flag disabled as `options.highlights=false` the `==` sequence is treated as literal text:

## Nested inline spans

Emphasis inside a highlight span:

==very *important* text==

```````````````````````````````` example
==very *important* text==
.
````````````````````````````````

Strong emphasis inside a highlight span:

==**bold** and _italic_==

```````````````````````````````` example
==**bold** and _italic_==
.
````````````````````````````````

Inline code inside a highlight span:

==highlighted `code`==

```````````````````````````````` example
==highlighted `code`==
.
````````````````````````````````

A link inside a highlight span:

==highlighted [link](http://example.com)==

```````````````````````````````` example
==highlighted [link](http://example.com)==
.
````````````````````````````````

A highlight span inside a link:

[==highlighted link==](http://example.com)

```````````````````````````````` example
[==highlighted link==](http://example.com)
.
````````````````````````````````

## Whitespace rules

An equals delimiter cannot open a highlight span when immediately followed by
whitespace:

== highlight==

```````````````````````````````` example
== highlight==
.
````````````````````````````````

An equals delimiter cannot close a highlight span when immediately preceded by
whitespace:

==highlight ==

```````````````````````````````` example
==highlight ==
.
````````````````````````````````

## Delimiter length

Single equals signs are not highlight delimiters:

=highlight=

```````````````````````````````` example
=highlight=
.
````````````````````````````````

Longer equals runs are not split into highlight delimiters:

===highlight===

```````````````````````````````` example
===highlight===
.
````````````````````````````````

## Unmatched delimiters

An opening delimiter with no matching closer is literal:

==highlight

```````````````````````````````` example
==highlight
.
````````````````````````````````

A closing delimiter with no matching opener is literal:

highlight==

```````````````````````````````` example
highlight==
.
````````````````````````````````

## Paragraph boundary stops resolution

A highlight span cannot cross a paragraph boundary:

This ==has a

new paragraph==.


```````````````````````````````` example
This ==has a

new paragraph==.
.
````````````````````````````````

## Suppression inside code

Equals signs inside code spans are treated as literal text:

`==code==`

```````````````````````````````` example
`==code==`
.
````````````````````````````````

Equals signs inside fenced code blocks are treated as literal text:

```````````````````````````````` example
```
==code==
```
.
````````````````````````````````

## Interaction with other extensions

Highlight may appear inside a spoiler span:

||==highlighted spoiler==||

```````````````````````````````` example
||==highlighted spoiler==||
.
````````````````````````````````

Highlight may appear inside strikethrough:

~~==highlighted deletion==~~

```````````````````````````````` example
~~==highlighted deletion==~~
.
````````````````````````````````

Highlight may appear inside table cells:

| Feature | Status |
| --- | --- |
| Highlight | ==done== |

```````````````````````````````` example
| Feature | Status |
| --- | --- |
| Highlight | ==done== |
.
````````````````````````````````
