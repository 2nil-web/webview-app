
if (typeof webapp_title === "function") {
  // To be called with -m option
  window.webapp_get_title().then(wtitle => { window.webapp_title(stem(wtitle)); });
  webapp_restore();
  webapp_size(640, 300);
  webapp_pos(640, 390);
  // Faire une méthode webap_move(x, y, w, h); ...
  //webapp_hints(3);
}

document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

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
  if (itemVal === null || itemVal === "") itemVal=defVal;
  return itemVal;
};

function conf_load() {
  url.value=getItemOrDefault('url', "https://wiki.space.thales");
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
    loader.innerText+='.';
  }
}

async function compute_contrib() {
  run.disabled=true;
  d1=new Date(start_date.value);
  d2=new Date(end_date.value);

  for (var d = d1; d <= d2; d.setDate(d.getDate() + 1)) {
    await wiki_rest(d);
  }

  d1s=d1.toISOString().split('T')[0];
  d2s=d2.toISOString().split('T')[0];

  fwrite("wiki_contrib.csv", "Wiki documenting contributions\n");
  fappend("wiki_contrib.csv", `Period${sep}${d1s}${sep}${d2s}\n`);
  fappend("wiki_contrib.csv", `Date${sep}Type${sep}Title\n`);
  fappend("wiki_contrib.csv", table);
  run.disabled=false;
  //console.log(table);
}

