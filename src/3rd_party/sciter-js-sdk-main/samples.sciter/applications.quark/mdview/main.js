import * as sys from "@sys";
import * as env from "@env";

import {Remarkable} from "remarkable/index.js";
import {HeaderIds} from "remarkable/plugins/header-ids.js";

import * as Settings from "settings.js";

const APP_NAME = "sciter.js.mdview";

const view = Window.this; // current window
const content = document.$("frame#content"); // content frame
const overlay = document.$("frame#overlay"); // print frame
const FolderE = document.$("#folder"); // folder element (files)
const ResultE = document.$("#result"); // result element (search)

const md = new Remarkable({html: true});
md.use(HeaderIds({anchorText: " "}));

export async function load(href = null) {
  try {
    href = href || content.frame.document.url();
    const url = new URL(href);
    if (url.extension !== "md") return;

    for (const f of Search.docs) {
      if (f.path == href) {
        const body = md.render(f.data);
        const html = `<html><body>${body}</body></html>`;
        content.frame.loadHtml(html, href);
        return;
      }
    }

    //let text = sys.fs.$readfile( URL.toPath(href) );
    //let body = sciter.decode(text,"utf-8");
    const r = await fetch(url.href);
    let body = r.text();
    body = md.render(body);
    const html = `<html><body>${body}</body></html>`;
    content.frame.loadHtml(html, href);
    if (url.hash)
      content.frame.document.$(printf("%s, (%s)", url.hash, url.hash.slice(1)))?.scrollIntoView({"behavior": "smooth", "block": "start"});
    setCaption();
  }
  catch (e) {
    console.error(e.message);
  }
}

function loadFolder(path) {
  function File(props) {
    const {name, folder} = props;
    const atts = (name == "index.md" || name == "README.md") ? {"index": true} : {};
    const value = URL.toPath(folder + name);
    Search.add(value);
    return <option .file {atts} value={value}><caption>{name.slice(0, -3)}</caption></option>;
  }

  let folderContent;

  function Folder(props) {
    const {name, folder} = props;
    const content = folderContent(folder + "/" + name);
    if (content.length)
      return <option .folder collapsed=""><caption>{name}</caption>{content}</option>;
    else
      return null;
  }

  const DIR_ENTRY_FILE = 1;
  const DIR_ENTRY_FOLDER = 2;

  function sorter(dirEntryA,dirEntryB) {
    if(dirEntryA.type == DIR_ENTRY_FOLDER &&
       dirEntryB.type != DIR_ENTRY_FOLDER) return -1;
    if(dirEntryB.type == DIR_ENTRY_FOLDER &&
       dirEntryA.type != DIR_ENTRY_FOLDER) return +1;

    if(dirEntryA.name == "README.md" && 
       dirEntryB.name != "README.md") return -1;
    if(dirEntryB.name == "README.md" && 
       dirEntryA.name != "README.md") return +1;

    return dirEntryA.name.toLowerCase().localeCompare(dirEntryB.name.toLowerCase());
  }

  folderContent = function(path) {
    const at = path + "/";
    const files = sys.fs.$readdir(at);
    if (!files) return "";

    files.sort(sorter);

    const content = [];

    for (const file of files) {
      if (file.type === 1) {
        if (file.name.endsWith(".md"))
          content.push(<File name={file.name} folder={at} />);
      }
      else
        content.push(<Folder name={file.name} folder={at} />);
    }
  
    return content;
  };

  FolderE.content(folderContent(path));

  FolderE.on("change", () => {
    load(FolderE.value);
  });

  const index = FolderE.$(":root>option[index]");
  if (index) index.click();
}

document.on("^click", "a[href]", function(evt, a) {
  let href = a.getAttribute("href");
  if (href.startsWith("#"))
    return;

  href = a.ownerDocument.url(href);
  const url = new URL(href);

  if (url.protocol == "file:" && url.extension == "md") {
    evt.stopPropagation();
    load(href);
  }
  else {
    // external URL
    evt.stopPropagation();
    env.launch(href);
  }
});

function getCurrentPath() {
  return URL.toPath(content.frame.document.url());
}

function setCaption() {
  const caption = getCurrentPath();
  view.caption = caption;
  document.$("window-caption").innerText = caption;
}

function onNewDocumentShown() {
  setCaption();
  document.timer(500, updateMonitor); // throttling monitor change
}

document.on("historystatechange", "frame#content", (evt, frame) => {
  document.$("button#home").state.disabled = frame.history.length == 0;
  document.$("button#back").state.disabled = frame.history.length == 0;
  document.$("button#fore").state.disabled = frame.history.forwardLength == 0;
  onNewDocumentShown();
});


document.on("click", "button#daynight", function(evt, button) {
  document.attributes["theme"] = button.value ? "dark" : "light";
  Settings.saveState();
});

document.on("click", "button#home", () => {
  content.history.go(-content.history.length);
});

document.on("click", "button#back", () => {
  content.history.back();
  return true;
});

document.on("click", "button#fore", () => {
  content.history.forward();
  return true;
});

document.on("click", "button#print", (evt, button) => {
  if (button.value) {
    document.attributes["mode"] = "pager";
    overlay.attributes["hidden"] = undefined;
    overlay.frame.loadFile(__DIR__ + "printview/pager.htm");
  }
  else {
    document.attributes["mode"] = undefined;
    overlay.attributes["hidden"] = true;
    overlay.frame.loadEmpty();
  }
  return true;
});

let currentMonitor = null;

function isMonitorNeeded() {
  return document.$("button#watch").value;
}

function updateMonitor() {
  if (currentMonitor) {
    currentMonitor.close();
    currentMonitor = null;
  }
  if (isMonitorNeeded()) {
    currentMonitor = sys.fs.watch(getCurrentPath(), function() {
      content.timer(100, load); // throttling reload
    });
  }
}

document.on("beforeunload", () => {
  if (currentMonitor)
    currentMonitor.close();
});

document.on("click", "button#watch", (evt, button) => {
  updateMonitor();
  Settings.saveState();
  return true;
});

document.on("provide-current-document", function(evt) {
  const doc = content.frame.document;
  evt.data = {
    html: doc.outerHTML,
    href: doc.url(),
  };
  return true;
});

Settings.add({
  store(data) {
    data.daynight = document.$("button#daynight").value;
  },
  restore(data) {
    document.$("button#daynight").value = data.daynight;
    document.attributes["theme"] = data.daynight ? "dark" : "light";
  },
});

function isFolder(path) {
  return sys.fs.$stat(path)?.st_mode & sys.fs.S_IFDIR;
}

document.ready = function() {
  const argv = view.scapp?.argv;
  let href = (__DIR__ + "hello.md");
  if (argv) {
    const path = argv[argv.length - 1];
    if (isFolder(path)) {
      loadFolder(path);
      return;
    }
    else if (path.endsWith(".md"))
      href = path;
  } else {
    const path = Window.this.parameters?.folder;
    if (isFolder(path)) {
      loadFolder(path);
      return;
    }
  }
  document.$("#nav").attributes["hidden"] = true;
  href = URL.fromPath(href);
  load(href);
};


// Search engine
const Search = {docs: []};



Search.add = async (path) => {
  const url = new URL(path);
  Search.docs.push({
    name: url.dir.substring(url.dir.slice(0, -1).lastIndexOf("/")+1) + url.filename,
    path: path,
    data: (await fetch(path)).text(),
  });
};

function escapeQuery(query) {
  return new RegExp(query.replaceAll("(", "\\(")
                          .replaceAll(")", "\\)")
                          .replaceAll("|", "\\|") , "gi");

}

Search.find = (query) => {
  query = escapeQuery(query);

  const result = [];
  for (const doc of Search.docs) {
    const find = {name: doc.name, path: doc.path, cache: []};
    const matches = doc.data.matchAll(query);

    for (const m of matches)
      find.cache.push(doc.data.substring(m.index-15, m.index+25));

    find.cache.length > 0 && result.push(find);
  }

  return result.length > 0 ? result : false;
};

Search.highlight = (query, index = 0) => {
  query = escapeQuery(query);

  let indexes = 0;

  function mark(node) {
    const range = new Range();
    const text = node.textContent;

    range.setStart(node, 0);
    range.setEnd(node, text.length);
    range.clearMark(["focus", "found"]);

    const matches = text.matchAll(query);
    for (const m of matches) {
      range.setStart(node, m.index);
      range. setEnd(node, m.index + m[0].length);
      range.applyMark(indexes == index ? "focus" : "found");

      const dtl = node.parentElement.$p("details");
      if (dtl) dtl.state.expanded = true;
      if (indexes == index) node.parentElement.scrollIntoView(); indexes++;
    }
  }

  function check(node) {
    node = node.firstChild;
    while (node) {
      node.nodeType == 1 ? check(node) : mark(node);
      node = node.nextSibling;
    }
  }

  check(content.frame.document);
};

ResultE.on("change", () => {
  for (const o of ResultE.$$("option")) {
    if (o.checked) {
      load(o.attributes["value"]);
      Search.highlight(
        document.$("#search").value,
        [...o.parentElement.children].indexOf(o) - 1,
      );
      break;
    }
  }
});

document.on("change", "#search", (evt, input) => {
  document.timer(200, () => {
    if (input.value.trim().length < 3) {
      FolderE.attributes["hidden"] = undefined; ResultE.attributes["hidden"] = true; return;
    }
    else {
      ResultE.attributes["hidden"] = undefined; FolderE.attributes["hidden"] = true;
    }

    const found = Search.find(input.value);
    if (!found) {
      ResultE.content(<h2 style="text-align:center; font-weight: normal;">Nothing found</h2>); return false;
    }

    const content = [];
    for (const f of found) {
      const cache = [];
      for (const find of f.cache)
        cache.push(<option value={f.path}><caption>{find}</caption></option>);

      content.push(<option expanded=""><caption>{f.name}</caption>{cache}</option>);
    }
    ResultE.content(content);
  });
});


async function start() {
    try {
      await Settings.init(APP_NAME);
    } finally {
      Window.this.state = Window.WINDOW_SHOWN;    
    }
}

start();
