module textviewer;

import sciter;
import archive = sciter.utils.archive;

import std.stdio;
import std.mmfile;

static auto blob = cast(ubyte[]) import("resources.bin");

struct LineInfo {
    size_t start;
    size_t length;
}

class MemMappedText {
  MmFile     mmf;
  ubyte[]    data;
  LineInfo[] lines;

  // map and collect lines
  void open(string path)
  {
    mmf = new MmFile(path);
    data = cast(ubyte[]) mmf[];

    size_t start = 0;
    size_t i = 0;
    const n = data.length;

    while (i < n)
    {
        if (data[i] == '\n')
        {
            // CRLF?
            size_t end = i;
            if (end > start && data[end - 1] == '\r')
                --end;
            lines ~= LineInfo(start, end - start);
            start = i + 1;
        }
        ++i;
    }

    // last line without newline
    if (start < n)
    {
        size_t end = n;
        if (end > start && data[end - 1] == '\r')
            --end;
        lines ~= LineInfo(start, end - start);
    }
  }

  int    totalLines() const { return cast(int)lines.length; }

  string lineAt(int idx) const { 
    LineInfo line = lines[idx];
    auto lineText = data[line.start .. line.start + line.length];
    return cast(string) lineText;
  }

  VALUE toVALUE() const {
    VALUE map; 
    map["totalLines"] = VALUE(&this.totalLines);
    map["lineAt"] = VALUE(&this.lineAt);
    return map;
  }

}

int main(string[] args)
{
    Application application = args;

    archive.open(blob);
    
    Window window = new Window(Window.AS_MAIN);

    window.load("this://app/index.htm"); // from archive based on resource blob

    try {

      if(args.length < 2)
        throw new Exception("No file name is provided");

      MemMappedText mmt = new MemMappedText();
      mmt.open(args[1]);
      // getting reference to showFile JS func in doc namespace 
      VALUE showFileJS = window.root.eval("showFile");
      showFileJS.call(mmt.toVALUE);
    } 
    catch (Exception e) {
      VALUE reportFileErrorJS = window.root.eval("reportFileError");
      reportFileErrorJS.call(e.msg);
    }

    // show it
    window.state = Window.STATE.SHOWN;

    // run message pump until quit request or main window closure
    return application.run();

}
