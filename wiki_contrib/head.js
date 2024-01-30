
var appName="Wiki contributors";

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

var winX, winY, winW, winH;
// Load geometry
winX=getItemOrDefault(`${appName}.x`, 640);
winY=getItemOrDefault(`${appName}.y`, 390);
//console.log(`GET POS ${winX}, ${winY}`);
winW=getItemOrDefault(`${appName}.outerWidth`, 640);
winH=getItemOrDefault(`${appName}.outerHeight`, 360);
//console.log(`GET DIM ${winW}, ${winH}`);

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

if (typeof webapp_restore === "function") {
  window.webapp_set_title(appName);
  // To be called with -m option
  webapp_restore();
  webapp_set_size(winW, winH);
  webapp_set_pos(winX, winY);
  //webapp_hints(3);
  /*
    0 Width and height are default size
    1 Width and height are minimum bounds
    2 Width and height are maximum bounds
    3 Window size is fixed
  */
  webapp_on_move('save_pos()');
}

function about () {
  info="Wiki contrib 1.0.0, includes:\n";
  app_info().then((ainf) => {
    console.log("app info="+ainf);
    info+=ainf.replace(/,/g,"\n")+"\n Which in turn, includes:\n";

    webview_ver().then((wvver) => {
      info+=" WebView version: "+wvver+"\n";
      console.log("webview_ver="+wvver);

      // May not include libcurl
      if (typeof libcurl_ver !== "undefined") { 
        libcurl_ver().then((lcver) => {
          info+=" LibCurl version: "+lcver;
          console.log("libcurl_ver="+lcver);
          infobox(info);
        });
      } else infobox(info);
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
  adjustToWindowHeight("userlist");
  //console.log(`wind dim (${document.documentElement.clientWidth}, ${document.documentElement.clientHeight})`);
  if (lastDocCliW !== document.documentElement.clientWidth) {
    lastDocCliW=document.documentElement.clientWidth;
    localStorage.setItem(`${appName}.outerWidth`, lastDocCliW);
  }

  if (lastDocCliH !== document.documentElement.clientHeight) {
    lastDocCliH=document.documentElement.clientHeight;
    localStorage.setItem(`${appName}.outerHeight`, lastDocCliH);
  }
}

function conf_load() {
  url.value=getItemOrDefault('url', "https://wiki.space.thales");
  if (url.value === "") url.value="toto";

  space.value=getItemOrDefault('space', "orchestra");

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

  userlist.value=getItemOrDefault('userlist', "alkadea,arnones,capous,cavallc,chaumia1,essaydh,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,ropold,thurona,tourel,xsii077,xsii076");

  // preset the period
  var d=new Date(); // current date
  d.setDate(1); // going to 1st of the month
  d.setHours(-1); // going to the end of previous month
  end_date.value=d.toISOString().split('T')[0];
  d.setDate(1); // going to 1st of previous month
  start_date.value=d.toISOString().split('T')[0];
}

window.addEventListener('load', () => {
  //localStorage.clear(); // Clear all localStorage values
  // List all localStorage
  //for (const key of Object.keys(localStorage)) { console.log("Onloaded "+key, localStorage.getItem(key)); } console.log("");

  document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

  Array.from(document.getElementsByClassName('collapser')).forEach((elt) => {
    if (elt.hasAttribute('id')) {
      divs=elt.parentElement.getElementsByClassName("collapsable");
      if (divs.length > 0) divs[0].hidden=getBoolItemOrDefault(elt.id+".collapsed", false);
    }

    //console.log("elt "+elt);
    elt.addEventListener('click', (event) => {
    //console.log("Onclick legend for element "+elt.id);

     if (event) event.stopPropagation();
      obj=event.target.parentElement;
      divs=obj.getElementsByClassName("collapsable");
      if (divs.length > 0) {
        divs[0].hidden=!divs[0].hidden;
        //console.log("setItem "+elt.id+"="+divs[0].hidden);
        localStorage.setItem(elt.id+".collapsed", divs[0].hidden);
        // Cas particulier de la liste des contributeurs
        if (obj.hasAttribute('id') && obj.id == "contributors") {

          if (divs[0].hidden === false) {
          } else {
          }
        }
      }
      windowSize();
    });

    elt.addEventListener('resize', () => {
      windowSize();
    });
  });

  conf_load();
  window.onresize = windowSize;
  windowSize();
});

function check_period () {
  let elt=event.srcElement;
  sd=new Date(start_date.value);
  ed=new Date(end_date.value);

  if (elt.id === "start_date") {
    // Si la date de début modifiée est supérieure à la date de fin alors on met la date de fin à égalité avec la date de début.
    if (sd > ed) end_date.value=sd.toISOString().split('T')[0];
  } else if (elt.id === "end_date") {
    // Si la date de fin modifiée est inférieure à la date de début alors on met la date de début à égalité avec la date de fin.
      if (ed < sd) start_date.value=ed.toISOString().split('T')[0];
  }
}

function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

async function readfile (filename, obj=output) {
  window.fread(filename).then(txt => {
    txt=decodeEntities(txt);
    if (obj instanceof HTMLElement) obj.value+=txt;
    console.log(txt);
  });
}

var table="";
var sep=';';

async function wiki_rest(dt) {
  curr_date=dt.toISOString().split('T')[0];
  const response=await fetch(
    `${url.value}/rest/api/content/search?cql=contributor+in+(${userlist.value})+and+space+=+${space.value}+and+lastmodified+=+${curr_date}&limit=1000`,
    { headers: new Headers({ "Authorization": `${login.value}:${psswd.value}` }),}
  );
  const data = await response.json();
  let today_contrib = data.results;
  //console.log(JSON.stringify(today_contrib));
  fappend("wiki_contrib.min.json", `\n  {\n    "date": "${curr_date}",\n    `);
  fappend("wiki_contrib.min.json", JSON.stringify(data, null, 2).replace(/\n/g, "\n      "));
  fappend("wiki_contrib.min.json", "\n  }");

  //console.log(today_contrib);
  for (i=0; i < today_contrib.length; i++) {
    contrib=today_contrib[i];
    //console.log(`Retrieving for ${curr_date}`);
    table+=`${curr_date}${sep}${contrib.type}${sep}"${contrib.title}"\n`;
    //fappend("wiki_contrib.min.json", JSON.stringify(today_contrib[i]));
  }
}

function setLoader(on) {
  var cl="", hide="hidden";

  if (on) {
    cl='loader'+(1+Math.floor(Math.random() * (5)));
    //console.log("Class "+cl);
    run.disabled=true;
  }
  my_loader.className=cl;
  my_loader.setAttribute("hidden", hide);
  run.disabled=false;
}

async function compute_contrib() {
  setLoader(true);

if (true) {
  run.disabled=true;
  d1=new Date(start_date.value);
  d2=new Date(end_date.value);

  d1s=d1.toISOString().split('T')[0];
  d2s=d2.toISOString().split('T')[0];


  fwrite("wiki_contrib.min.json", "[");
  for (var d = d1; d <= d2; d.setDate(d.getDate() + 1)) {
    await wiki_rest(d);
  }
  fappend("wiki_contrib.min.json", "]\n");

  fwrite("wiki_contrib.csv", "Wiki documenting contributions\n");
  fappend("wiki_contrib.csv", `Period${sep}${d1s}${sep}${d2s}\n`);
  fappend("wiki_contrib.csv", `Date${sep}Type${sep}Title\n`);
  fappend("wiki_contrib.csv", table);
  current_path().then(currdir => {
    scriptName=`${currdir}/runImportWikiStats.js`;
    xlFileName=`${currdir}/Wiki contributions from ${d1s} until ${d2s}.xlsx`;
    //console.log(`${scriptName}\n${xlFileName}`);

    webapp_shell(scriptName).then( () => {
        exists(xlFileName).then(res => {
          //console.log("RES "+res);
          if (res) {
            webapp_shell(xlFileName);//.then(() => { setLoader(false); });
          } else {
            alert(`The expected file '${xlFileName}' does not exist check for any possible troubles`);
          }
            setLoader(false); 
        });
    });
  });
}
}

