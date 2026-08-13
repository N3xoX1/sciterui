# Definition lists

Markdown definition lists are an extended feature used to create structured lists of terms and their corresponding definitions. They are particularly useful for glossaries, technical documentation, and parameter descriptions. 

## Basic Syntax

A definition list consists of a term followed by its definition, which starts with a colon (`:`) on the next line. For example:

Term 1
: Definition for term 1
Term 2
: Definition for term 2
: Alternative definition for term 2


```````````````````````````````` example
.
Term 1
: Definition for term 1
Term 2
: Definition for term 2
: Alternative definition for term 2
.
````````````````````````````````

## Advanced Features

Definitions can span multiple lines or include other Markdown elements like bold, italic, links, or code. For example:

Markdown
: Markdown is a **lightweight** markup language:
  - Supports *italic* and **bold** text
  - Allows [links](https://example.com)
  - Includes `inline code`

```````````````````````````````` example
.
Markdown
: Markdown is a **lightweight** markup language:
  - Supports *italic* and **bold** text
  - Allows [links](https://example.com)
  - Includes `inline code`
.
````````````````````````````````

Nested definitions are also possible in some Markdown implementations:

Outer Term 1
: Outer Definition
  Inner Term 1.1
  : Inner Definition
Outer Term 2
: Outer Definition
  Inner Term 2.1
  : Inner Definition  

```````````````````````````````` example
.
Outer Term 1
: Outer Definition
  Inner Term 1.1
  : Inner Definition
Outer Term 2
: Outer Definition
  Inner Term 2.1
  : Inner Definition  
.
````````````````````````````````