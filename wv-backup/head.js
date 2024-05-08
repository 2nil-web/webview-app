
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
webapp.set_size(640, 250, 1); // Define minimum bounds (third option to 1)
webapp.set_size(winW, winH);  // Define actual size
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

function adjustHeight(elt, offset=20) {
  //var elt=document.getElementById(elemId);
  var eRc=elt.getBoundingClientRect();
  eleT=Math.trunc(eRc.top);
  elt.style.height=(window.innerHeight-eleT-offset)+"px";
}

var lastDocCliW=-1, lastDocCliH=-1;
function windowSize() {
  adjustHeight(output);

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

class BakLn extends HTMLElement {
  static observedAttributes = ["src", "dst"];

  constructor() {
    super();
  }

  create=function() {
    var backupTable;

    // Créé une nouvelle table si le 1er enfant du père n'en est pas une.
    if (this.parentNode.firstChild.tagName === undefined || this.parentNode.firstChild.tagName.toLowerCase() !== 'table') {
      //console.log("Creating a new table");
      backupTable=document.createElement("table");
      backupTable.style="border-collapse:separate; border-spacing: 4px 4px;";
      backupTable.dataset.cbUniqPfx=Date.now().toString(36);
      backupTable.dataset.cbIdCount=0;
      this.parentNode.prepend(backupTable);
    } else backupTable=this.parentNode.firstChild;

    // Add a row
    //console.log(Date.now().toString(36)+", "+Date.now().valueOf()+", "+new Date().valueOf());
    var tr=this.parentNode.firstChild.insertRow(-1), cbId=Math.random().toString(16).slice(2);
    cbId=backupTable.dataset.cbUniqPfx+'.'+backupTable.dataset.cbIdCount.toString();
    //console.log(`cbId: ${cbId}`);
    backupTable.dataset.cbIdCount++;

    //tr.style="outline:thin solid; user-select: none";
    tr.style="user-select: none;";

    function addTdObj(obj) {
      var td = document.createElement("td");
      obj.style="display: block;"; // Pour étirer l'objet sur toute la largeur de la cellule
      td.appendChild(obj);
      tr.appendChild(td);
    }

    function addTdLabel(txt) {
      var lab = document.createElement("label");
      lab.htmlFor=cbId;
      lab.innerHTML=txt;
      addTdObj(lab);
    }

    // Create a checkbox as the first cell of the row
    var cbox = document.createElement("input");
    cbox.type = "checkbox";
    cbox.id = cbId;
    addTdObj(cbox);
    //console.log(`${this.tagName}[${cbox.id}]:[${this.innerHTML}]`);
    this.innerHTML='';
    this.style="position: absolute;left:0px; top:0px; width:0px; height:0px;";

    // Add a second cell for the src of the backup
    addTdLabel(this.getAttribute("src"));
    // Create a third cell as a decoration
    addTdLabel("&#x1F449;");
    // Add a fourth cell for the dst of the backup
    addTdLabel(this.getAttribute("dst"));
    // Add backup type as a dataset field
    tr.dataset.type=this.getAttribute("type");
    // Add a fifth cell for the type of the backup
    //addTdLabel(this.getAttribute("type"));
  }

  connectedCallback() {
//    this.create();
    addEventListener("load", (event) => {
      this.create();
    });
  }
}
customElements.define("bak-ln", BakLn);


function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

function exec_cmd_no_return(run_cmd, cmd_value, duration) {
  if (run_cmd.disabled === true) {
    console.log('Command ['+cmd_value+'] did not return after '+duration/1000+' seconds.');
    run_cmd.disabled=false;
  }
}

function exec_cmd(run_cmd, cmd_value, output_area) {
    run_cmd.disabled = true;
    cmd_value=cmd_value.trim();
    tmout=3000;
    setTimeout(exec_cmd_no_return, tmout, run_cmd, cmd_value, tmout);
    window.webapp_exec(cmd_value, true).then(result => {
      output_area.value += decodeEntities(result.value);
      output_area.scrollTop=output_area.scrollHeight;
      run_cmd.disabled=false;
    });
}

function run_backupRES() {
  const allBak = document.getElementsByTagName("bak-ln");
  for (i=0; i < allBak.length; i++) {
    console.log("SRC: "+allBak.item(i).getAttribute("src")+", DST: "+allBak.item(i).getAttribute("dst")+", TYPE: "+allBak.item(i).getAttribute("type"));
  }
}

function run_backup() {
  bakLst=document.getElementById("backup-list");
  const rows = bakLst.getElementsByTagName("table").item(0).rows;
  params="";
  for (i=0; i < rows.length; i++) {
    cells=rows[i].cells;
    cbx=cells[0].firstChild;
    src=cells[1].firstChild;
    dst=cells[3].firstChild;
    typ=rows[i].dataset.type;
    if (cbx.checked === true) {
      params+=src.textContent+" "+dst.textContent+" ";
      if (typ == 'null') params+="user";
      else params+=typ;
      params+=" ";
    }

    console.log(`ROW[${i}][0]: ${cbx.checked}`);
    console.log(`ROW[${i}][1]: ${src.textContent}`);
    console.log(`ROW[${i}][3]: ${dst.textContent}`);
    console.log(`ROW[${i}].type: ${rows[i].dataset.type}`);
    console.log("");
  }

  console.log("params: "+params);
//  exec_cmd(backup_menu, "echo OK; read a", 
// D:\UnixTools\msys64\usr\bin\mintty.exe -o Charset=UTF-8 -i app.ico -p 600,640 -s 100,20 -t "Sauvegarde en cours" -e /bin/bash --login -i -c mysync.sh 
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

  window.onresize = windowSize;
  // The following 2 commands to trigger a correct resize
  webapp.set_size(winW, winH-1);
  webapp.set_size(winW, winH);
});


