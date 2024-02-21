
var appName="Citrix publishing";

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
    //console.log(`For item ${itemId} default ${msg}value is '${itemVal}'`);
    localStorage.setItem(itemId, itemVal);
  } else {
    //console.log(`For item ${itemId} localStorage ${msg}value is '${itemVal}'`);
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
    //console.log(`SAVE POS ${pos.x}, ${pos.y}`);
  });
}

function save_dim() {
  localStorage.setItem(`${appName}.outerWidth`, window.outerWidth);
  localStorage.setItem(`${appName}.outerHeight`, window.outerHeight);
  //console.log(`SAVE DIM ${window.outerWidth}, ${window.outerHeight}`);
}

var winX, winY, winW, winH;
// Load geometry
winX=getItemOrDefault(`${appName}.x`, 2);
winY=getItemOrDefault(`${appName}.y`, 2);
//console.log(`GET POS ${winX}, ${winY}`);
winW=getItemOrDefault(`${appName}.outerWidth`, 1900);
winH=getItemOrDefault(`${appName}.outerHeight`, 1060);//-3;
//console.log(`GET DIM ${winW}, ${winH}`);


webapp.set_title(appName);
// To be called with -m option
webapp.restore();
webapp.set_icon("app.ico");
// Define minimum bounds
webapp.set_size(560, 290, 1);
webapp.set_size(winW, winH);
webapp.set_pos(winX, winY);
webapp.on_move('save_pos()');
//webapp.hints(3);

function about () {
  info=appName+" 1.0.0, includes:\n";
  app_info().then((ainf) => {
    //console.log("app info="+ainf);
    info+=ainf.replace(/,/g,"\n")+"\n Which in turn, includes:\n";

    webview_ver().then((wvver) => {
      info+=" WebView version: "+wvver+"\n";
      //console.log("webview_ver="+wvver);
      infobox(info);
    });
  });
}

function conf_update(elt_id) {
  elt=document.getElementById(elt_id);
  //console.log(elt_id+' '+elt.value);
  localStorage.setItem(elt_id, elt.value);
}

function adjustToWindowHeight(elemId) {
  var elem=document.getElementById(elemId);
  var elemRect=elem.getBoundingClientRect();
  eleT=Math.trunc(elemRect.top);
  //console.log("window.innerHeight "+window.innerHeight+"-userlist.top "+eleT+"="+(window.innerHeight-eleT-10));
  elem.style.height=(window.innerHeight-eleT-20)+"px";
}

var lastDocCliW=-1, lastDocCliH=-1;
function windowSize() {
  adjustToWindowHeight("publi_view");
  if (lastDocCliW !== document.documentElement.clientWidth) {
    lastDocCliW=document.documentElement.clientWidth;
    localStorage.setItem(`${appName}.outerWidth`, lastDocCliW);
  }

  if (lastDocCliH !== document.documentElement.clientHeight) {
    lastDocCliH=document.documentElement.clientHeight;
    localStorage.setItem(`${appName}.outerHeight`, lastDocCliH);
  }
  //console.log(`dim ${lastDocCliW}, ${lastDocCliH}`);
}

function save_conf() {
  save_pos();
  save_dim();
}

function save_conf_and_exit() {
  save_conf();
  webapp_exit();
}


function conf_load() {
  login.value=localStorage.getItem('login');
  if (login.value === "") {
    getenv('USERNAME').then(val => {
      if (val === "") {
        getenv('USER').then(val2 => {
          login.value=val2;
        });
      } else login.value=val;
    });
  }

  psswd.value=localStorage.getItem('psswd');
  //console.log("login "+login.value+", password "+psswd.value);
  cred.style.display=localStorage.getItem(`${appName}.cred`, "block");
}

function show_hide_cred () {
  if (cred.style.display == "none")
    cred.style.display = "block";
  else
    cred.style.display = "none";

  adjustToWindowHeight("publi_view");
  localStorage.setItem(`${appName}.cred`, cred.style.display);
}

function show_publi(title, json) {
  var table = new Tabulator("#publi_view", {
    // set height of table (in CSS or here), this enables the Virtual DOM
    // and improves render speed dramatically (can be any valid css height value)
//    height:140,
    data:json, //assign data to table
    layout:"fitColumns", //fit columns to width of table (optional)
    //Define Table Columns
    columns:[
      {title:"DeliveryController", field:"DeliveryController", width:100},
      {title:"Worker", field:"Worker", width:100},
      {title:"AgentVersion", field:"AgentVersion", width:100},
      {title:"Publication", field:"Publication", width:150},
      {title:"DeliveryGroup", field:"DeliveryGroup", width:150},
      {title:"Members", field:"Members", width:500},
      {title:"Day", field:"Day", width:100},
      {title:"Frequency", field:"Frequency", width:60},
      {title:"StartTime", field:"StartTime", width:100},
      {title:"Cpu", field:"NumCpu", width:60},
      {title:"Mem", field:"MemoryGB", width:70},
      {title:"InfosDisk", field:"InfosDisk"},
    ],
  });
}

function show_backends (title, json) {
   //create Tabulator on DOM element with id "example-table"
  var table = new Tabulator("#publi_view", {
    // set height of table (in CSS or here), this enables the Virtual DOM
    // and improves render speed dramatically (can be any valid css height value)
    data:json, //assign data to table
    layout:"fitColumns", //fit columns to width of table (optional)
    //Define Table Columns
    columns:[
      {title:"VM name", field:"VMName"},
      {title:"Guest name", field:"GuestName"},
      {title:"Power", field:"Powerstate"},
      {title:"Cpu", field:"NumCpu"},
      {title:"Mem GB", field:"MemoryGB"},
      {title:"IP address", field:"IPAddress"},
      {title:"OS version", field:"OSVersion", hozAlign:"right"},
      {title:"UpTime", field:"UpTime", sorter:"UpTime", sorter: "number"},
    ],
  });
}


url="http://tlpalcorr01.aes.alcatel.fr:8090/agl/backends-list";

async function load_pub (pubname) {
  endp=url+'/'+pubname+'.json';

  if (login.value !== "" && psswd.value !== "") {
    let response = await fetch(endp, {method:'GET', headers: {'Authorization': 'Basic ' + btoa(`${login.value}:${psswd.value}`)}});
    let txt = await response.text();
    var lines = txt.split("\n");
    title=lines.shift();
    //document.getElementById("publi_title").innerHTML=title;
    var json = lines.join("\n");
    
    if (pubname === 'BEF_ListeBackEndDetails') show_backends(title, json);
    else show_publi(title, json);
  }
}

window.addEventListener('load', () => {
  //localStorage.clear(); // Clear all localStorage values
  // List all localStorage
  //for (const key of Object.keys(localStorage)) { console.log("Onloaded "+key, localStorage.getItem(key)); } console.log("");

  document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });
  conf_load();
  //load_pub();
  window.onresize = windowSize;
  windowSize();
});

function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

async function readfile (filename, obj=output) {
  window.fread(filename).then(txt => {
    txt=decodeEntities(txt);
    if (obj instanceof HTMLElement) obj.value+=txt;
    //console.log(txt);
  });
}

