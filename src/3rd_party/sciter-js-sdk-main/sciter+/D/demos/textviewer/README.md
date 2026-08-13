# Basic TextViewer app

Demonstrates use of behavior:virtual-list to show portion of text file in sliding window.

Uses std.mmfile.MmFile to map file in memory for effective acces.

Loading whole file in DOM/memory is not an option, especially on large files.