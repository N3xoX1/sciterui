# sciter-js-sdk

Build cross platform desktop applications with HTML, CSS and javascript.

### [Tutorials](https://sciter.com/tutorials/), [Documentation](https://docs.sciter.com/) and [Development logfile](logfile.md).

## Script

Sciter.JS uses [QuickJS](https://bellard.org/quickjs/) in particular [QuickJS++](https://gitlab.com/c-smile/quickjspp).

* ES6: async/await, classes, modules, destructuring;
* BigInt, BigFloat, BigDecimal - arbitrary precision IEEE 754 floating point operations and transcendental functions with exact rounding (currency, etc);
* Node.JS runtime (more or less full) is coming; 

## Platforms

* **Windows - i32, i64 and arm64** - published;
* **Linux - i64, arm32 (Raspberry Pi)** - published;
* **MacOS - i64** and arm64 - published;
* Mobiles - pending;

## Demos

### Calc demo (universal: Browser and Sciter)

Windows:

![Browser and Sciter](https://sciter.com/wp-content/uploads/2020/10/Sciter.JS.Calc_-e1602390091709.png)

Linux (Raspbian on Raspberry Pi in particular)

![Linux on Raspberry Pi](https://sciter.com/wp-content/uploads/2020/10/sjs-rpi.png)

MacOS:

<img src="https://sciter.com/wp-content/uploads/2021/11/calc-macos.png" width="628">

Path: samples/calc

Browser and Sciter shows [the same HTML document](https://gitlab.com/c-smile/sciter-js-sdk/blob/main/samples/calc/index.html).

To run demo start run-calculator-browser.bat or run-calculator-sciter.bat. The later will start bin.win/x32/scapp.exe - standalone Sciter engine.

### Mithril demo (universal: Browser and Sciter)

Path: samples/mithril

Sciter.JS runs [mithril](https://mithril.js.org) as it is. Only basic use cases are thested so far though.
