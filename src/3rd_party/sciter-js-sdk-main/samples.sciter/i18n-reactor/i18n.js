
// translation table
let translation = {};

// these tags will be automatically put into translation table
JSX_translateTags = {
   caption: true,
   p: true,
   label: true,
}

JSX_translateText = function(text) {
  return translation[text] || text;           
}

JSX_translateNode = function(node, translationId) {
  const handler = translation[translationId];
  if(typeof handler != "function") return node; // as it is
  let translatedText = handler(...node[2]); // pass list of kids as arguments
  if(!translatedText) return node;
  return JSX(node[0],node[1],[translatedText]); // synthesize new node
}

export function loadTranslation(lang) {
  document.attributes["lang"] = lang;
  let table = fetch(__DIR__ + "langs/" + lang + ".js", {sync:true}).text();
  translation = eval("(" + table + ")");
}
