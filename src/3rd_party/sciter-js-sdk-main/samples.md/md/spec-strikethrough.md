
# Strike-Through

With the flag `options.strikethrough`, the engine enables extension for recognition
of strike-through spans.

Strike-through text is any text wrapped in two tildes (`~`). Strike-through text gets rendered as `<del>` span:

~~Hi~~ Hello, world!

```````````````````````````````` example
~~Hi~~ Hello, world!
.
````````````````````````````````

Too long tilde sequence won't be recognized:

foo ~~~bar~~~

```````````````````````````````` example
foo ~~~bar~~~
.
````````````````````````````````

Also note the markers cannot open a strike-through span if they are followed
with a whitespace; and similarly, then cannot close the span if they are
preceded with a whitespace:

~~foo ~~bar

```````````````````````````````` example
~~foo ~~bar
.
````````````````````````````````

As with regular emphasis delimiters, a new paragraph will cause the cessation
of parsing a strike-through:

This ~~has a

new paragraph~~.


```````````````````````````````` example
This ~~has a

new paragraph~~.
.
````````````````````````````````
