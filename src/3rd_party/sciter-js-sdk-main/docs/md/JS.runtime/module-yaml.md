---
sidebar_position: 6
---

# module `@yaml`

The module provides [YAML](https://yaml.org/) related services.

## functions:

### parse()

```js
let map = yaml.parse(yamlText);
```

Returns a value, usually plain JS object that contains keys/values of parsed document.

Parameters:

* _yamlText_ - either _string_ or _ArrayBuffer_, YAML text;

Returns a value.

Example, when given YAML text like this:

```yaml
What It Is:
  YAML is a human-friendly data serialization
  language for all programming languages.

YAML Specifications:
- YAML 1.2:
  - Revision 1.2.2  # Oct  1, 2021  (About this version)
  - Revision 1.2.1  # Oct  1, 2009
  - Revision 1.2.0  # Jul 21, 2009
- YAML 1.1          # Jan 18, 2005
- YAML 1.0          # Jan 29, 2004
```

It will return JS object having this structure:

```JS
{
   "What It Is": "YAML is a human-friendly data serialization...",
   "YAML Specifications": [
      { 
        "YAML 1.2": [
           "Revision 1.2.2",
           "Revision 1.2.1",
           "Revision 1.2.0" 
        ]
      },
      "YAML 1.1",
      "YAML 1.0"
   ]
}
```

