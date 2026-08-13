---
sidebar_position: 5
---

# module `@markdown`

The module provides [Markdown](https://www.markdownlang.com/) related services.

## functions:

### toHTML()

```js
const html = markdown.toHTML(markdown, options);
```

Returns html string converted from the *markdown* source.

Parameters:

* _markdown_ - either _string_ or _ArrayBuffer_, MD text;
* _options_ - optional object, [see](#options);

Returns string - HTML text.

### toElement()

```js
markdown.toElement(container, markdown, options);
```

Parses *markdown* text directly into DOM element bypassing HTML parsing phase.

Parameters:

* _container_ - DOM [Element](../DOM/Element) that will contain parsed content;
* _markdown_ - either _string_ or _ArrayBuffer_, MD text;
* _options_ - optional object, [see](#options);

Returns _true_ in case of success.

### toFragment()

```js
markdown.toFragment(markdown, options);
```

Parses *markdown* text into VDOM fragment that is ready to be used in Reactor components, for example as:

```js
function Markdown({md}) {
   return <section styleset="markdown.css#markdown">
      { MD.toFragment(md) }
   </section>;
}
```

Parameters:

* _markdown_ - either _string_ or _ArrayBuffer_, MD text;
* _options_ - optional object, [see](#options);

Returns VDOM fragment : `<>...md content here...</>`.


## options

An object that defines parsing flags:

* _indentedCodeBlocks_ - bool, _true_ (default) - supports indented code blocks; 
* _tables_ - bool, _true_ (default) - supports [ Table extension](https://www.markdownlang.com/extended/tables.html); 
* _taskLists_ - bool, _true_ (default) - supports [Task Lists extension](https://www.markdownlang.com/extended/task-lists.html); 
* _superscripts_ - bool, _true_ (default) - supports [superscript extension](https://www.markdownlang.com/advanced/math.html#superscripts-and-subscripts); 
* _subscripts_ - bool, _true_ (default) - supports [subscript extension](https://www.markdownlang.com/advanced/math.html#superscripts-and-subscripts); 
* _admonitions_, bool, _true_ (default) - supports [GitHub admonition] (https://www.markdownlang.com/advanced/github.html#alerts) and [Docusaurus admonitions](https://www.markdownlang.com/advanced/plugins.html#containers-admonitions); 
* _footnotes_ - not yet;
* _highlight_ - bool, _true_ (default) - supports [highlight extension](https://www.markdownlang.com/extended/highlight.html); 
* _strikethrough_ - bool, _true_ (default) - supports [strikethrough extension](https://www.markdownlang.com/extended/strikethrough.html); 
* _blockIdClass_ - bool, _true_ (default) - supports [Heading ID extension](https://www.markdownlang.com/extended/heading-ids.htm) with optional class name as `{#myid .myclass}` 
* _spanIdClass_ - bool, _true_ (default) - supports Span ID and Classes extension as `[span text]{#myid .myclass}`. 
* _spoilers_ - not yet;
* _defintionLists_ - bool, _true_ (default) - supports [defintion lists](https://www.markdownlang.com/extended/definition-lists.html).
* _embeddedHTML_ - bool, _true_ (default) - supports embedded HTML blocks and spans;
* _URL_ - string, URL of the markdown document. The URL is used to resolve relative links and image URLs.

