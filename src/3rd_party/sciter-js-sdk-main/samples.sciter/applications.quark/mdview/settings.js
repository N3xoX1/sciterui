
import * as env from "@env";
import * as sys from "@sys";

import {encode, decode} from "@sciter";

let path;
const list = []; // list of persistable objects

// persistable settings

export async function store() {
    if (!path)
        return;

    let file;

    try {
        file = await sys.fs.open(path, "w+", 0o666);
        const data = {};
        for (const persistable of list)
            persistable.store(data);
        await file.write(encode(JSON.stringify(data, null, "  "), "utf-8"));
    }
    catch (error) {
        Window.modal(<warning>Cannot open file {path} for writing.<br/>{error}<br/>Settings will not be saved.</warning>);
    }
    finally {
        if (file) file.close();
    }
}

async function restore() {
    let buffer;
    try {
        buffer = await sys.fs.$readFile(path, "r");
    }
    catch (error) {
        return false;
    }

    try {
        const data = JSON.parse(decode(buffer, "utf-8"));
        for (const persistable of list)
            persistable.restore(data);
        return true;
    }
    catch (error) {
        console.error("Restore error:", error);
    }
}

export function add(persistable) {
    list.push(persistable);
}

// window position persistence
add({
    store(data) {
        let [x,y,w,h] = Window.this.box("xywh","border","monitor");
        data.window = {left:x,top:y,width:w,height:h, monitor:Window.this.monitor};
    },
    restore(data) {
        if (data.window) {
            var x = Math.max(data.window.left,0);
            var y = Math.max(data.window.top,0);
            var w = Math.max(data.window.width,800);
            var h = Math.max(data.window.height,600); 
            var monitor = data.window.monitor ?? 0;
            Window.this.moveTo(monitor,x,y,w,h); // replace on monitor
        }
    },
});

export function saveState() {
    if (!document.parentWindow) {
        Window.this.off(saveState); // document is unloaded
        return;
    }

    // throttled request to store the data
    document.timer(1000, store);
}

Window.this.on("move", saveState)
    .on("size", saveState);

export async function init(APP_NAME) {
    path = env.path("USER_APPDATA", APP_NAME + ".json");
    return await restore();
}

