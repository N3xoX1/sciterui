module dsciter;

import sciter;
import archive = sciter.utils.archive;

import std.stdio;
import core.thread;
import std.concurrency;

// custom drawing controller demo
import drawingcontroller;

//string html = "<html><body>Hello D!</body></html>";
static auto blob = cast(ubyte[]) import("resources.bin");

static string DCompilerName() { 
    version (DigitalMars) { return "DMD (Digital Mars D)"; }
    version (LDC) { return "LDC (LLVM-based D Compiler)"; }
    version (GNU) { return "GDC (GNU D Compiler)"; }
}
// demo of native function callable from JS
int adder(int a, int b) { return a + b; }

// demo of native function callable from JS

// function to run in a separate thread
void worker(Promise promise) {
    writeln("Thread started");
    // Simulate work
    Thread.sleep(5.seconds);
    writeln("Thread finished");
    VALUE rv = "5 seconds passed";
    promise.resolve(rv);
}

// async function, exposed to JS
Promise asyncFunction() {

   // create promise
   Promise promise = new Promise();

   // starts worker thread with the promise
   auto t = new Thread({ worker(promise); });
   t.start();

   // return the promise to JS
   return promise; 
}


int main(string[] args)
{
    Application application = args;
    
    archive.open(blob);
    
    struct DCompiler {
       string name = DCompilerName;
       int verMajor = __VERSION__ / 1000;
       int verMinor = __VERSION__ % 1000;
       VALUE   opCast(T : VALUE)() const { return VALUE.from(this); }
    }

    // create global JS variable: let Compiler = { name:..., verMajor:... verMinor:... } 
    application.globalVar("Compiler", cast(VALUE) DCompiler() );
    // add global function adder (for the demo purposes)
    application.globalVar("adder", VALUE(&adder));
    application.globalVar("asyncFunction", VALUE(&asyncFunction));

    // demo function that is used as an event handler  
    bool onClick() {
       writeln("mouse CLICK");
       return false;
    }

    Window window = new Window(Window.AS_MAIN);

    //window.on(MOUSE_EVENT.TYPE.CLICK) = &onClick;
    window.on(MOUSE.CLICK).at("body") = () { writeln("mouse CLICK"); return false;};
    window.on(EVENT.CLICK).at("button#test") = (ref Element button){ writeln("CLICK on ", button.tag); return false;};
   
    // load HTML
    //window.load( cast(ubyte[]) html ); // from literal HTML string
    window.load("this://app/index.htm"); // from archive based on resource blob

    // show it
    window.state = Window.STATE.SHOWN;

    // getting reference to jsFunc in doc namespace 
    VALUE jsFunc = window.root.eval("jsFunc");
    // calling it
    VALUE r = jsFunc.call("From D to JS with love");
    writeln("jsFunc reported ", r.toString);

    // run message pump until quit request or main window closure
    return application.run();
}