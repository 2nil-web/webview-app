
if (typeof webapp_title === "function") {
  // To be called with -m option
  window.webapp_get_title().then(wtitle => { window.webapp_title(stem(wtitle)); });
  webapp_restore();
  webapp_size(640, 300);
  webapp_pos(640, 390);
  // Faire une méthode webap_move(x, y, w, h); ...
  //webapp_hints(3);
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

function getItemOrDefault (itemName, defVal) {
  itemVal=localStorage.getItem(itemName);
  if (itemVal === null || itemVal === "") {
    itemVal=defVal;
    localStorage.setItem(itemName, itemVal);
  }
  return itemVal;
};

function getBoolItemOrDefault (itemId, defVal) {
  itemVal=localStorage.getItem(itemId);
  //console.log("getBoolItemOrDefault for "+itemId+'='+itemVal);
  if (itemVal === null) {
    itemVal=defVal;
    localStorage.setItem(itemId, itemVal);
  }
  return (itemVal === "true");
};

function adjustToWindowHeight(elemId) {
  var elem=document.getElementById(elemId);
  var elemRect=elem.getBoundingClientRect();
  eleT=Math.trunc(elemRect.top);
  //console.log("window.innerHeight "+window.innerHeight+"-userlist.top "+eleT+"="+(window.innerHeight-eleT-10));
  elem.style.height=(window.innerHeight-eleT-10)+"px";
}

function windowSize() {
  adjustToWindowHeight("userlist");
}

function conf_load() {
  document.getElementById('url').value=getItemOrDefault('url', "https://wiki.space.thales");
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

document.addEventListener('DOMContentLoaded', function() {
  //localStorage.clear(); // Clear all localStorage values
  // List all localStorage
  for (const key of Object.keys(localStorage)) { console.log("Onloaded "+key, localStorage.getItem(key)); }
  console.log("");

  conf_load();

  document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });
  document.querySelectorAll('.togglable').forEach((elt) => {
    if (elt.hasAttribute('id')) {
      if (getBoolItemOrDefault(elt.id+".expanded", true)) {
        elt.innerHTML=elt.innerHTML.replace("<br>", "");
        elt.classList.add("expanded");
      } else {
        elt.innerHTML='<br>'+elt.innerHTML;
        elt.classList.remove("expanded");
      }
    }

    elt.querySelector('legend').addEventListener('click', (event) => {
      console.log("Onclick legend for element "+elt.id);
      if (elt.classList.contains('expanded')) {
        elt.innerHTML='<br>'+elt.innerHTML;
      } else {
        elt.innerHTML=elt.innerHTML.replace(/<br>/,'');
      }

      elt.classList.toggle('expanded');

      if (elt.hasAttribute('id')) {
        if (elt.classList.contains('expanded')) {
          localStorage.setItem(elt.id+".expanded", true);
        } else {
          localStorage.setItem(elt.id+".expanded", false);
        }
      }
      windowSize();
    });

    elt.addEventListener('resize', (event) => {
      windowSize();
    });
  });
});

window.onresize = windowSize;

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
  //console.log(today_contrib);
  for (i=0; i < today_contrib.length; i++) {
    contrib=today_contrib[i];
    console.log(`Retrieving for ${curr_date}`);
    table+=`${curr_date}${sep}${contrib.type}${sep}"${contrib.title}"\n`;
  }
}

function setLoader(on) {
  var cl="", hide="hidden";

  if (on) {
    cl='loader'+(1+Math.floor(Math.random() * (5)));
    console.log("Class "+cl);
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

  for (var d = d1; d <= d2; d.setDate(d.getDate() + 1)) {
    await wiki_rest(d);
  }

  fwrite("wiki_contrib.csv", "Wiki documenting contributions\n");
  fappend("wiki_contrib.csv", `Period${sep}${d1s}${sep}${d2s}\n`);
  fappend("wiki_contrib.csv", `Date${sep}Type${sep}Title\n`);
  fappend("wiki_contrib.csv", table);
  current_path().then(currdir => {
    scriptName=`${currdir}/runImportWikiStats.js`;
    xlFileName=`${currdir}/Wiki contributions from ${d1s} until ${d2s}.xlsx`;
    console.log(`${scriptName}\n${xlFileName}`);

    webapp_shell(scriptName).then( () => {
        exists(xlFileName).then(res => {
          console.log("RES "+res);
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

