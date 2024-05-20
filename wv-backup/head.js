
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

const allBak = document.getElementsByTagName("bak-ln");
for (i=0; i < allBak.length; i++) {
  chkd=getBoolItemOrDefault (`${appName}.bak-ln.${i}.checked`, false);
  console.log(`${appName}.bak-ln.${i}.checked: ${chkd}`);
}


webapp.set_title(appName);
// To be called with -m option
webapp.restore();
webapp.set_icon("app.ico");
webapp.set_size(640, 200, 1); // Define minimum bounds (third option to 1)
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

  static bakLnCbCount=0;

  create=function() {
    var backupTable;

    // Créé une nouvelle table si le 1er enfant du père n'en est pas une.
    if (this.parentNode.firstChild.tagName === undefined || this.parentNode.firstChild.tagName.toLowerCase() !== 'table') {
      //console.log("Creating a new table");
      backupTable=document.createElement("table");
      backupTable.style="border-collapse:separate; border-spacing: 4px 4px;";
      this.parentNode.prepend(backupTable);
    } else backupTable=this.parentNode.firstChild;

    // Add a row
    //console.log(Date.now().toString(36)+", "+Date.now().valueOf()+", "+new Date().valueOf());
    var tr=this.parentNode.firstChild.insertRow(-1);

    tr.style="outline:thin solid; user-select: none";
    //tr.style="user-select: none;";

    function addTdObj(obj) {
      var td = document.createElement("td");
      obj.style="display: block;"; // Pour étirer l'objet sur toute la largeur de la cellule
      td.appendChild(obj);
      tr.appendChild(td);
    }

    function addTdLabel(txt) {
      var lab = document.createElement("label");
      lab.htmlFor=BakLn.bakLnCbCount;
      lab.innerHTML=txt;
      addTdObj(lab);
    }

    // Create a checkbox as the first cell of the row
    var cbox = document.createElement("input");
    cbox.type = "checkbox";
    cbox.id = BakLn.bakLnCbCount;
    cbox.checked=getBoolItemOrDefault (`${appName}.bak-ln.${cbox.id}.checked`, false);
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

    cbox.addEventListener('change', function() {
      //console.log(`${appName}.bak-ln.${this.id}.checked: ${this.checked}`);
      localStorage.setItem(`${appName}.bak-ln.${this.id}.checked`, this.checked);
    });

    BakLn.bakLnCbCount++;
    //console.log(`bakLnCbCount:${BakLn.bakLnCbCount}`);
  }

  connectedCallback() {
    addEventListener("load", (event) => {
      this.create();
    });
  }
}

customElements.define("bak-ln", BakLn);

const delay = ms => new Promise(res => setTimeout(res, 400));

function readfile (filename, v) {
  window.fread(filename).then(res => {
    if (res.charAt(0) === '\n') res=res.slice(1);
    if (res.slice(-1) !== '\n') res+='\n';//res.slice(0, -1);
    v.txt=res.replace(/(.*)\n/g, "--exclude='$1' ").slice(0, -1);
  });
}

function tcap_center () {
  for (i=0; i < arguments.length; i++) {
    console.log("Arg"+i+": "+arguments[i])
  }
}

async function run_backup(real_run=true) {
//  tcap_center(1, "toto");

  bakLst=document.getElementById("backup-list");
  const rows = bakLst.getElementsByTagName("table").item(0).rows;
  shell_cmds="";
  currPath=window.location.pathname;
  currPath=currPath.substring(1, currPath.lastIndexOf("/"));
  cyg_currPath="/"+currPath.replace(/\\/g, "\/").replace(/:/, "");

  /*
  let fam={txt:null}, usr={txt:null};
  readfile("famille.exclude_list", fam);
  readfile("user.exclude_list", usr);
  await delay();
  fam=fam.txt;
  usr=usr.txt;
  console.log("Fam: "+fam);
  console.log("Usr: "+usr);
*/

  for (i=0; i < rows.length; i++) {
    cells=rows[i].cells;
    cbx=cells[0].firstChild;
    src=cells[1].firstChild;
    dst=cells[3].firstChild;
    typ=rows[i].dataset.type;

    if (cbx.checked === true) {

      if (shell_cmds !== "") shell_cmds+="; ";

      if (typ.startsWith("win-")) {
        typ=typ.slice(4);
        opt_param="--no-links";
      } else {
        opt_param="--chmod=755 --chown=nobody:nogroup";
      }

      if (typ == 'null' || typ == 'usr' || typ == 'user') {
        file_title=cyg_currPath+"/user.exclude_list";
      } else {
        file_title=cyg_currPath+"/famille.exclude_list";
      }

      rsync_cmd="/usr/bin/rsync --progress -au "+opt_param+" --delete --delete-excluded --exclude-from="+file_title;
      cygsrc="/"+src.innerText.replace(/\\/g, "\/").replace(/:/, "");
      rsync_cmd+=` "${cygsrc}" "${dst.textContent}"`;
      //-e "ssh -i $HOME/.ssh/id_rsa" ';

      echo_cmd=rsync_cmd.replace(/\/usr\/bin\/rsync(.*)/, "echo rsync $1");
      shell_cmds+="echo; tput smso; echo 'Sauvegarde "+src.innerText+" dans "+dst.innerText+"'; tput rmso; ";
      shell_cmds+=echo_cmd;
      if (real_run) shell_cmds+="; "+rsync_cmd;
    }
  }

  term_width=266;
  cmd_env=`D:\\UnixTools\\msys64\\usr\\bin\\mintty.exe -o Charset=UTF-8 -i app.ico -p 10,350 -s ${term_width},44 -t "Sauvegarde en cours" -h always -e /bin/bash --login -i -c `;
  //cmd=`${cmd_env} "${shell_cmds} tput smso smul; echo 'Sauvegarde terminée, appuyer sur <Entrée>'"`;

  cmd=cmd_env;
  if (real_run) msg="  B A C K U P   I N   P R O G R E S S  ";
  else          msg="  D R Y   R U N  ";
  spc=' '.repeat(msg.length);
  col=Math.trunc((term_width-msg.length)/2);
  console.log(col);

  cmd+='"';
    // Shows 255 colors in background for TERM=mintty: for ((i=0; i < 255; i++)); do tput setab $i; printf " %03d " "$i"; tput sgr0; done
    // tput cup row col
    cmd+="export TERM=mintty;";
    cmd+=`tput cup 0 ${col} bold;      echo '${spc}';`;
    cmd+=`tput cup 1 ${col} setab 124; echo '${spc}';`;
    cmd+=`tput cup 2 ${col};           echo '${msg}';`;
    cmd+=`tput cup 3 ${col} ;          echo '${spc}';`;
    cmd+="tput sgr0;";
    cmd+="echo;";

    cmd+=` ${shell_cmds}; `;

  msg=" Sauvegarde terminée, appuyer sur <Entrée> ";
  spc=' '.repeat(msg.length);
  col=Math.trunc((term_width-msg.length)/2);

    cmd+="echo; ";
    cmd+="tput u7; IFS=';' read -r -d R -a pos;row=$((${pos[0]:2} - 1)); row=${pos[0]:2}; ";
    cmd+=`tput bold setab 284; `;
    cmd+=`tput cup \${row}        ${col}; echo '${spc}'; `;
    cmd+=`tput cup $((\${row}+1)) ${col}; echo '${msg}';`;
    cmd+=`tput cup $((\${row}+2)) ${col}; echo -n '${spc}'; tput sgr0;`;
  cmd+='"';

  backup_menu.style = "pointer-events:none;";
  console.log("RSYNC CMD: "+cmd);
  window.webapp_exec(cmd);
//  backup_menu.style = "pointer-events:auto;";
}

function double_set_size() {
  webapp.set_size(winW, winH-1);
  webapp.set_size(winW, winH);
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
  double_set_size();
});


//D:\UnixTools\msys64\usr\bin\mintty.exe -o Charset=UTF-8 -i app.ico -p center -s 110,20 -t "Sauvegarde en cours" -h always -e /bin/echo toto tutu tata
