module sciter.utils.archive;

import sciter.api;
import std.string;
import std.algorithm;

private HARCHIVE har;

public bool open( ubyte[] data ) { 
  close(); har = SciterOpenArchive(data.ptr,cast(uint)data.length); 
  return har != null; 
}

public void close() { 
  if(har) 
    SciterCloseArchive(har); 
  har = null; 
}

public const(ubyte)[] get(const(wchar)[] path ) {

  if(!har)
    return null;

  LPCBYTE pb = null; UINT blen = 0;

  if( path.startsWith("//"w))
      path = path[2..$];

  SciterGetArchiveItem(har,(path ~ '\0').ptr,&pb,&blen); 
  return pb[0 .. blen];
}
