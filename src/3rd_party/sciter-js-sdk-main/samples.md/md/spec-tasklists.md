
# Tasklists

Basic task list may look as follows:

* [x] foo
* [X] bar
* [ ] baz

```````````````````````````````` example
.
* [x] foo
* [X] bar
* [ ] baz
.
````````````````````````````````

Task lists can also be in ordered lists:

 1. [x] foo
 2. [X] bar
 3. [ ] baz

```````````````````````````````` example
.
 1. [x] foo
 2. [X] bar
 3. [ ] baz
.
````````````````````````````````
Task lists can also be nested in ordinary lists:

 * xxx:
   * [x] foo
   * [x] bar
   * [ ] baz
 * yyy:
   * [ ] qux
   * [x] quux
   * [ ] quuz

```````````````````````````````` example
.
 * xxx:
   * [x] foo
   * [x] bar
   * [ ] baz
 * yyy:
   * [ ] qux
   * [x] quux
   * [ ] quuz
.
````````````````````````````````

Or in a parent task list:

 1. [-] xxx:
    * [x] foo
    * [x] bar
    * [ ] baz
 2. [x] yyy:
    * [x] qux
    * [x] quux
    * [x] quuz

```````````````````````````````` example
.
1. [-] xxx:
    * [x] foo
    * [x] bar
    * [ ] baz
 2. [x] yyy:
    * [x] qux
    * [x] quux
    * [x] quuz
.
````````````````````````````````

Also, ordinary lists can be nested in the task lists:

 * [x] xxx:
   * foo
   * bar
   * baz
 * [ ] yyy:
   * qux
   * quux
   * quuz

```````````````````````````````` example
.
 * [x] xxx:
   * foo
   * bar
   * baz
 * [ ] yyy:
   * qux
   * quux
   * quuz
.
````````````````````````````````
