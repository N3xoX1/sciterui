import * as env from "@env";
import * as sys from "@sys";
import * as sciter from "@sciter";

import { LogRunner } from "logrunner.js";

export function checkForImageMagic(resultCb) {
  let signatureFound = false;

  if (env.PLATFORM != "Windows")
    return; // we need it only on Windows so far

  async function readPipe(pipe) {
    try {
      while (pipe) {
        const text = sciter.decode(await pipe.read());
        signatureFound = signatureFound || text.indexOf("ImageMagic") >= 0;
      }
    }
    catch (e) { }
    resultCb(signatureFound);
  }

  const proc = sys.spawn(["magick", "-version"], { stdout: "pipe", stderr: "pipe" });

  if (!proc)
    document.post(() => resultCb(false));
  else
    readPipe(proc.stdout);
}

function checkFolder(folderPath, forWriting = false) {
  const S_IFDIR = 0x4000;
  const S_IWRITE = 0x0080;
  const S_IREAD = 0x0100;

  const modes = forWriting ? (S_IFDIR | S_IWRITE) : (S_IFDIR | S_IREAD);

  const stat = sys.fs.$stat(folderPath);
  if (stat)
    return (stat.st_mode & modes) == modes;

  return false;
}

async function convertSvgToIco(inp, outp) {
  const args = ["magick", "-density", "256x256", "-background", "transparent", inp, "-define", "icon:auto-resize=16,24,32,40,48,64,96,128,16,32,48,128", "-colors", "256", outp];
  const r = await LogRunner.run(args);
  if (r != 0) throw "convertSvgToIco: failed to produce .ICO file";
}

function checkFile(path) {
  if (sys.fs.$stat(path)) return path;
  return null;
}

function getPackfolderPath() {
  if (env.PLATFORM == "Windows") {
    return checkFile(env.home("../packfolder.exe")) ||
      checkFile(env.home("../../bin/windows/packfolder.exe"));
  }
  else if (env.PLATFORM == "OSX") {
    return checkFile(env.home("packfolder")) ||
      checkFile(env.home("../../bin/macosx/packfolder"));
  }
  else if (env.PLATFORM == "Linux") {
    return checkFile(env.home("../packfolder")) ||
      checkFile(env.home("../../bin/linux/packfolder"));
  }
}

function getScappPath(target) {
  switch (target) {
    case "winX32": return checkFile(env.home("../x32/scapp.exe")) || checkFile(env.home("../../bin/windows/x32/scapp.exe"));
    case "winX64": return checkFile(env.home("../x64/scapp.exe")) || checkFile(env.home("../../bin/windows/x64/scapp.exe"));
    case "winARM64": return checkFile(env.home("../arm64/scapp.exe")) || checkFile(env.home("../../bin/windows/arm64/scapp.exe"));
    case "macX64": return checkFile(env.home("scapp")) || checkFile(env.home("../../bin/macosx/scapp"));
    case "linuxX64": return checkFile(env.home("../x64/scapp")) || checkFile(env.home("../../bin/linux/x64/scapp"));
    case "linuxARM32": return checkFile(env.home("../arm32/scapp")) || checkFile(env.home("../../bin/linux/arm32/scapp"));
  }
}

async function packageResources(folder, datfile) {
  const packfolder = getPackfolderPath();
  if (!packfolder)
    throw "packageResources: no packfolder executable found";
  const args = [packfolder, folder, datfile, "-binary"];
  const r = await LogRunner.run(args);
  if (r != 0) throw `packfolder: failed to produce ${datfile} file, status=${r}`;
}

function makePath(dir, subdirs, nameext) {
  let path = dir;
  for (const sub of subdirs) {
    path += "/";
    path += sub;
    if (checkFile(path)) continue;
    if (!sys.fs.$mkdir(path)) throw "makePath: cannot create dir:" + path;
  }
  return path + "/" + nameext;
}

// Use imagemagick to convert svg into pngs, then iconutil to convert into icons
// Notice here inf means: input file (svg), outp means: out path, the result is: ${outp}/icon.icns
// imagick alwasy draw black border on path,  please install inkscape to avoid this(imagick will use inkscape if detected)
async function convertSvgToIcns(inf, outp) {

  async function svg2png(pathin,pathout,w,h) {
     w = Length.ppx(w);
     h = Length.ppx(h);
     let svg = await Graphics.Image.load(pathin);
     console.assert(svg,`cannot read ${svg}`);
     let out = new Graphics.Image(w,h,gfx => { gfx.draw(svg, {x:0,y:0,width:w,height:h}) });
     let bytes = out.toBytes();
     let file = await sys.fs.open(pathout,"w");
     await file.write(bytes);
     await file.close(); 
  }

  const iconset = [
    { size: 16, name: "16x16" }, 
    { size: 32, name: "16x16@2x" }, 
    { size: 32, name: "32x32" }, 
    { size: 64, name: "32x32@2x" },
    { size: 128, name: "128x128" }, 
    { size: 256, name: "128x128@2x" }, 
    { size: 256, name: "256x256" }, 
    { size: 512, name: "256x256@2x" },
    { size: 512, name: "512x512" }, 
    { size: 1024, name: "512x512@2x" }];

  let setfolder = makePath(outp, ["icon.iconset"], "")
  for(let icon of iconset) {
    let outf = setfolder + "icon_" + icon.name + ".png";
    await svg2png(inf,outf,icon.size,icon.size);
    //let args = ["convert", "-resize", `${icon.size}x${icon.size}`, "-background", "none", inf, dst]
    //let r = await LogRunner.run(args);
    //if (r != 0) throw "convertSvgToIcns: failed to covert svg into png file";
  }
  let r = await LogRunner.run(["iconutil", "--convert", "icns", setfolder]);
  if (r != 0) throw "convertSvgToIcns: failed to produce icon.icns file";
}

function mkAppleBundle(exefile, params) {
  var plistInfo = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
  <!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
  <plist version=\"1.0\">\n\
  <dict>\n\
    <key>CFBundleDevelopmentRegion</key>\n\
    <string>en</string> \n\
    <key>CFBundleIVersion</key>\n\
    <string>{version}</string>\n\
    <key>CFBundleShortVersionString</key>\n\
    <string>{version}</string>\n\
    <key>CFBundleIdentifier</key>\n\
    <string>{name}</string>\n\
    <key>CFBundlePackageType</key>\n\
    <string>APPL</string>\n\
    <key>CFBundleSignature</key>\n\
    <string>MOOS</string>\n\
    <key>LSMinimumSystemVersion</key>\n\
    <string>10.9</string>\n\
    <key>NSMainNibFile</key>\n\
    <string>MainMenu</string>\n\
    <key>NSPrincipalClass</key>\n\
    <string>NSApplication</string>\n\
    <key>CFBundleName</key>\n\
    <string>{name}</string>\n\
    <key>CFBundleExecutable</key>\n\
    <string>{name}</string>\n\
    <key>CFBundleIconFile</key>\n\
    <string>{name}.icns</string>\n\
  </dict>\n\
  </plist>";

  function copyFileForce(fnSrc, fnDes) {
    if (sys.fs.$stat(fnDes))
      sys.fs.$unlink(fnDes);
    sys.fs.$copyfile(fnSrc, fnDes);
  }
  
  makePath(params.out, ["macos", `${params.exe}.app`, "Contents", "MacOS"], "");
  makePath(params.out, ["macos", `${params.exe}.app`, "Contents", "Resources"], "");
  
  let appPath = `${params.out}/macos/${params.exe}.app`;
  sys.fs.$chmod(appPath, 0o755);
  appPath += "/Contents"

  let fn = sys.fs.$open(appPath + "/Info.plist", "w");
  if (!fn) throw "Can't open plist.info to write";
  let plist = plistInfo.replace(/{name}/g, params.exe).replace(/{version}/g, params.productVersion);
  fn.$write(plist);
  fn.$close();
  copyFileForce(exefile, appPath + "/MacOS/" + params.exe);
  sys.fs.$chmod(appPath + "/MacOS/" + params.exe, 0o755);
  copyFileForce(params.out+"/icon.icns", `${appPath}/Resources/${params.exe}.icns`);
}


function assembleExe(target, scapp, datfile, exefile, params = null) {
  const r = Window.this.scapp.assembleExe(scapp, datfile, exefile, params);
  let msg;
  switch (r) {
    case 0: LogRunner.add(`${target} Done!`, "result"); break;
    case 1: LogRunner.add(`${target} Done, but no metadata update`, "result"); break;
    case -1: LogRunner.add(`${target} FAILURE, no .dat file`, "stderr"); break;
    case -2: LogRunner.add(`${target} FAILURE opening output file`, "stderr"); break;
    case -3: LogRunner.add(`${target} FAILURE writing output file`, "stderr"); break;
  }
  if (target === "mac") mkAppleBundle(exefile, params)
}

export async function assemble(params) {
  LogRunner.clear();

  if (!checkFolder(params.resources, false)) {
    LogRunner.add(`Error: ${params.resources} is not a readable folder`, "stderr");
    return;
  }

  if (!checkFolder(params.out, true)) {
    LogRunner.add(`Error: ${params.out} is not a writeable folder`, "stderr");
    return;
  }


  const datfile = params.out + "/" + params.exe + ".dat";
  const icofile = params.out + "/" + params.exe + ".ico";

  try {
    await packageResources(params.resources, datfile);
    for (const target of params.targets) {
      switch (target) {
        case "winX32": {
          await convertSvgToIco(params.logo, icofile);
          const scapp = checkFile(env.home("../x32/scapp.exe")) || checkFile(env.home("../../bin.win/x32/scapp.exe"));
          const exefile = makePath(params.out, ["windows", "x32"], params.exe + ".exe");
          var p = Object.assign({}, params, { icofile: icofile });
          assembleExe(target, scapp, datfile, exefile, p);
        } break;
        case "winX64": {
          await convertSvgToIco(params.logo, icofile);
          const scapp = checkFile(env.home("../x64/scapp.exe")) || checkFile(env.home("../../bin.win/x64/scapp.exe"));
          const exefile = makePath(params.out, ["windows", "x64"], params.exe + ".exe");
          var p = Object.assign({}, params, { icofile: icofile });
          assembleExe(target, scapp, datfile, exefile, p);
        } break;
        case "winARM64": {
          await convertSvgToIco(params.logo, icofile);
          const scapp = checkFile(env.home("../arm64/scapp.exe")) || checkFile(env.home("../../bin.win/arm64/scapp.exe"));
          const exefile = makePath(params.out, ["windows", "arm64"], params.exe + ".exe");
          var p = Object.assign({}, params, { icofile: icofile });
          assembleExe(target, scapp, datfile, exefile, p);
        } break;
        case "mac": { // TODO: build proper .app bundle folder here
          await convertSvgToIcns(params.logo, params.out)
          const scapp = checkFile(env.home("scapp")) || checkFile(env.home("../../bin.osx/scapp"));
          const exefile = makePath(params.out, ["macos"], params.exe);
          assembleExe(target, scapp, datfile, exefile, params);
        } break;
        case "linuxX64": {
          const scapp = checkFile(env.home("../x64/scapp")) || checkFile(env.home("../../bin.lnx/x64/scapp"));
          const exefile = makePath(params.out, ["linux", "x64"], params.exe);
          assembleExe(target, scapp, datfile, exefile);
        } break;
        case "linuxARM32": {
          const scapp = checkFile(env.home("../arm32/scapp")) || checkFile(env.home("../../bin.lnx/arm32/scapp"));
          const exefile = makePath(params.out, ["linux", "arm32"], params.exe);
          assembleExe(target, scapp, datfile, exefile);
        } break;
      }
    }
  }
  catch (e) {
    LogRunner.add(e, "stderr");
  }
}

