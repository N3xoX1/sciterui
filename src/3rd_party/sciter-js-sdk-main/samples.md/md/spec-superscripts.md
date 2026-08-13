
# Superscript Spans

Recognition of inline superscript spans using the `^text^` syntax.  A single caret on each side
wraps the content, which the HTML renderer outputs as `<sup>`.


## Basic recognition

x^2^

```````````````````````````````` example
x^2^
````````````````````````````````

Superscripts may open after regular text characters:

foo^bar^

```````````````````````````````` example
foo^bar^
````````````````````````````````

Superscripts may appear inside emphasis:

_x^2^_

```````````````````````````````` example
_x^2^_
````````````````````````````````

Superscripts may appear inside strong emphasis:

**x^2^**

```````````````````````````````` example
**x^2^**
````````````````````````````````

## Opt-in behavior

With `option.superscripts = false` the `^` sequence is treated as literal text:


## Whitespace rules

A caret cannot open a superscript span when immediately followed by whitespace:

x^ 2^

```````````````````````````````` example
x^ 2^
````````````````````````````````

A caret cannot close a superscript span when immediately preceded by whitespace:

x^2 ^

```````````````````````````````` example
x^2 ^
````````````````````````````````

## Longer caret runs are literal

Double or longer caret runs are not split into superscript delimiters:

x^^y^^

```````````````````````````````` example
x^^y^^
````````````````````````````````

## Paragraph boundary stops resolution

A superscript span cannot cross a paragraph boundary:

x^

2^

```````````````````````````````` example
x^

2^
````````````````````````````````

## Code spans suppress markers

Caret characters inside code spans are treated as literal text:

`x^2^`

```````````````````````````````` example
`x^2^`
````````````````````````````````

A caret at the very start of a line can still open a superscript span:

^2^

```````````````````````````````` example
^2^
````````````````````````````````

A lone caret at the start of a line followed by whitespace can be neither opener
nor closer, so it is treated as literal text:

^ x

```````````````````````````````` example
^ x
````````````````````````````````

## Unmatched delimiters are literal

An opening caret with no matching closer is literal:

x^2

```````````````````````````````` example
x^2
````````````````````````````````

A closing caret with no matching opener is literal:

x2^

```````````````````````````````` example
x2^
````````````````````````````````

## Interaction with other extensions

### Subscripts (requires `options.subscrips=true`)

Superscript and subscript may nest inside each other:

x^a~b~^

```````````````````````````````` example
x^a~b~^
````````````````````````````````

### Spoilers (requires `option.spoiler=true`)

Superscript may appear inside a spoiler span:

||x^2^||

```````````````````````````````` example
||x^2^||
````````````````````````````````
