
# Subscript Spans

Recognition of inline subscript spans using the `~text~` syntax (single tilde 
on each side).  The HTML renderer outputs these as `<sub>`.

## Basic recognition

H~2~O

```````````````````````````````` example
H~2~O
````````````````````````````````

Subscripts may open after regular text characters:

log~2~x

```````````````````````````````` example
log~2~x
````````````````````````````````

Subscripts may appear inside link text:

```````````````````````````````` example
[H~2~O](http://example.com)
````````````````````````````````

## Opt-in behavior

With `option.subscripts = false` the `~` is treated as literal text:

## Whitespace rules

A tilde cannot open a subscript span when immediately followed by whitespace:

H~ 2~O

```````````````````````````````` example
H~ 2~O
````````````````````````````````

A tilde cannot close a subscript span when immediately preceded by whitespace:

H~2 ~O

```````````````````````````````` example
H~2 ~O
````````````````````````````````

A tilde at the start of a line can open but not close a span:

~2~

```````````````````````````````` example
~2~
````````````````````````````````

## Code spans suppress markers

Tilde characters inside code spans are treated as literal text:

`H~2~O`

```````````````````````````````` example
`H~2~O`
````````````````````````````````

## Double tilde is not subscript

Double tilde `~~text~~` is not a subscript delimiter; it is only meaningful
when `options.strikethrough=true` is also enabled:

~~old~~

```````````````````````````````` example
~~old~~
````````````````````````````````

## Interaction with `options.strikethrough=true`

With only `options.strikethrough=true`, single-tilde `~text~` renders as
strikethrough (GFM compatibility):

~text~

```````````````````````````````` example
~text~
````````````````````````````````

With only `options.strikethrough=true`, double-tilde `~~text~~` also renders as
strikethrough:

~~text~~

```````````````````````````````` example
~~text~~
````````````````````````````````

With both `option.subscripts = true` and `options.strikethrough=true`, single tilde
becomes subscript and double tilde remains strikethrough:

H~2~O and ~~old~~

```````````````````````````````` example
H~2~O and ~~old~~
````````````````````````````````
