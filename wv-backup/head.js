
var appName="Rsync backup";

// To avoid the webapp to load multiple times
const channel = new BroadcastChannel(appName);
channel.postMessage('another-'+appName);
channel.addEventListener('message', (msg) => {
  if (msg.data === 'another-'+appName) {
    // Exitting from 2nd instance of appName
    webapp_exit();
  }
});

function getItemOrDefault (itemId, defVal, msg="") {
  itemVal=localStorage.getItem(itemId);

  if (itemVal === null || itemVal === "") {
    itemVal=defVal;
    localStorage.setItem(itemId, itemVal);
  }

  return itemVal;
};

function getBoolItemOrDefault (itemId, defVal) {
  return (getItemOrDefault (itemId, defVal, "boolean ") === "true");
};

function save_pos() {
  webapp_get_pos().then(pos => {
    localStorage.setItem(`${appName}.x`, pos.x);
    localStorage.setItem(`${appName}.y`, pos.y);
  });
}

function save_dim() {
  localStorage.setItem(`${appName}.outerWidth`, window.outerWidth);
  localStorage.setItem(`${appName}.outerHeight`, window.outerHeight);
}

var winX, winY, winW, winH;
// Load geometry
winX=getItemOrDefault(`${appName}.x`, 2);
winY=getItemOrDefault(`${appName}.y`, 2);
winW=getItemOrDefault(`${appName}.outerWidth`, 1900);
winH=getItemOrDefault(`${appName}.outerHeight`, 1060);//-3;


webapp.set_title(appName);
// To be called with -m option
webapp.restore();
webapp.set_icon("app.ico");
// Define minimum bounds
webapp.set_size(560, 290, 1);
webapp.set_size(winW, winH);
webapp.set_pos(winX, winY);
webapp.on_move('save_pos()');

function about () {
  info=appName+" 1.0.0";
  if (is_webview()) {
  info+=", includes:\n";
  app_info().then((ainf) => {
    info+=ainf.replace(/,/g,"\n")+"\n Which in turn, includes:\n";

    webview_ver().then((wvver) => {
      info+=" WebView version: "+wvver+"\n";
      infobox(info);
    });
  });
  } else alert(info);
}

function conf_update(elt) {
  elt=document.getElementById(elt.id);
  localStorage.setItem(appName+'.'+elt.id, elt.value);
}

function adjustToWindowHeight(elemId) {
  var elem=document.getElementById(elemId);
  var elemRect=elem.getBoundingClientRect();
  eleT=Math.trunc(elemRect.top);
  elem.style.height=(window.innerHeight-eleT-20)+"px";
}

var lastDocCliW=-1, lastDocCliH=-1;
function windowSize() {
  adjustToWindowHeight("backup-list");
  if (lastDocCliW !== document.documentElement.clientWidth) {
    lastDocCliW=document.documentElement.clientWidth;
    localStorage.setItem(`${appName}.outerWidth`, lastDocCliW);
  }

  if (lastDocCliH !== document.documentElement.clientHeight) {
    lastDocCliH=document.documentElement.clientHeight;
    localStorage.setItem(`${appName}.outerHeight`, lastDocCliH);
  }
}

function save_conf() {
  save_pos();
  save_dim();
}

function save_conf_and_exit() {
  save_conf();
  webapp_exit();
}


class BackupElement extends HTMLElement  {
  static observedAttributes = ["src", "dst"];

  constructor() {
    super();
  }

  create=function() {
    // Create checkbox
    var cbox = document.createElement("input");
    cbox.type = "checkbox";
    // Create label
    var label = document.createElement("label");
    // Wrap checkbox within label
    label.style.border="solid 1px";
    label.appendChild(cbox);
    //label.innerHTML=cbox+this.getAttribute("src")+"==>"+this.getAttribute("dst");
    var txt=document.createTextNode(this.getAttribute("src")+"\t\t"+this.getAttribute("dst"));
    txt.fontFamily="Lucida Console, Courier New, monospace";
    label.appendChild(txt);
    // Add label to backup element
    this.appendChild(label);
  }

  connectedCallback() {
    console.log(`backup-element added with src: "${this.getAttribute("src")}" and dst: "${this.getAttribute("dst")}".`);
    this.create();
  }
}

customElements.define("backup-element", BackupElement);

function list_available_backup () {
  const allBak = document.getElementsByTagName("backup-element");
  for (i=0; i < allBak.length; i++) {
    console.log("SRC: "+allBak.item(i).getAttribute("src")+", DST: "+allBak.item(i).getAttribute("dst"));
  }

}

window.addEventListener('load', () => {
  //localStorage.clear(); // Clear all localStorage values
  // List all localStorage
  //for (const key of Object.keys(localStorage)) { console.log("Onloaded "+key, localStorage.getItem(key)); } console.log("");


  document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });
  // For webview or web browser
  if (!is_webview()) {
    backup_menu.style.display="none";
    quit_menu.style.display="none";
    about_menu.style.display="none";
  }

  //list_available_backup ();
  window.onresize = windowSize;
  windowSize();
});

