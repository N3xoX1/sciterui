
# The Fetch() function

`fetch()` in Sciter is modelled after [Fetch API](https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API).

Signature:

```
fetch(url:string | request:Request [, options:object]): Promise(Response) 
```

Where _options_ is an object that may have following properties:

* `method` - string, "GET" | "POST" | etc. 
* `headers` - object ( string->string map ), request headers. 
* `cache` - string, "no-cache" | "reload" | "default".
* `sync` - boolean, Sciter's extras, when set to _true_ fetch performs synchronous data retrieval. Synchronous fetch is useful to get local data - local file or resource content.
* `body` is one of: 
   - instance of a string;
   - instance of an ArrayBuffer or [TypedArray](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray);
   - instance of a File;
   - an object representing form data; such an object may contain fields of the following types:
     
     * string;
     * ArrayBuffer annotated with  `mimeType: string` and `fileName: string` properties; 
     * instance of File object optionally annotated with  `mimeType: string` and `fileName: string` properties; 
     
     If such form data object contains an ArrayBuffer and/or a File then the engine sends request encoded as `multipart/form-data` otherwise it will be sent as `application/x-www-form-urlencoded`.

## Request object

The request object has following methods and properties:

* `request.cache` - see `options.cache` above;
* `request.context` - requested data type, one of: "html" | "image" | "style" | "cursor" | "script" | "data" | "font" | "audio";
* `request.headers` - read-only, object;
* `request.method` - read-only, string;
* `request.url` - read-only, string;

Sciter extras:

* `request.abort()` - method, on call will try to abort current request; Response of aborted request will have `response.aborted` property set to _true_.
* `request.progress(function(bytesLoaded, totalBytes) {...})` the method allows to setup callback on downloading progress.
* `request.fulfill(data:ArrayBuffer, mime: string [,status:integer])` - method, on call will set data to the request as if server responded with that data/mime-type.
* `request.reject(status:integer)` - method, on call will reject the request as if server responded with that status code.

> Note: fetch API uses either HTTP client to get remote files or OS API for local files. In the first case return values are HTTP response codes, while in the second case error codes are coming from OS.
  
## Response object

The response object is what the `fetch()` will promise to return. It has the following properties:

* `request` - Request object this response is associated with;
* `headers` - object, response headers;
* `url` - string, final url. Note: it can differ from request.url if redirections happen;
* `ok` - boolean, if _true_ then the request was fetched successfully;
* `redirected` - boolean;
* `status` - integer, 200, 404, etc.;
* `mimeType` - string, MIME type of received data;
* `arrayBuffer():Promise(arrayBuffer)` - get data as an arrayBuffer;
* `formData():Promise(object)` ;
* `json():Promise(any)` - get response as ready to use data (parsed JSON response);
* `text():Promise(string)` - get response data as string;

Sciter extras:
* `aborted` - boolean, if _true_ then the request was aborted by `request.abort()` call.  


## Cancelable requests in Sciter

Here is sample code that implements request cancellable by the user:
```js

// creating request:
let request = new Request("url", options);

// wiring click eevent handler that will abort the request:
document.on("click", "button#abort", function(){
   request.abort();  
})

// issuing the request:

try {
  let response = await fetch(request);

  if(response.ok)
    //... handle response data here ...
  else if(response.status == 404)
    //... wrong url ...
  else 
    //... etc. ...

} catch(response) {
  if(response.aborted)
     //... ended by ,abort() call ...
  else
     //... some other problems, see response.status ...
}
```
  
  
  
