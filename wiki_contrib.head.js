
if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
  window.webapp_get_title().then(wtitle => { window.webapp_title(stem(wtitle)); });
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

function loadreg() {
  defval="alkadea,arnones,capous,cavallc,chaumia1,essaydh,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,ropold,thurona,tourel,xsii077,xsii076";
  GetReg("Software\\WikiContrib", "userlist", defval).then(regUserlist => {
    userlist.value=regUserlist;
    console.log(userlist.value);
  });
}


function save_and_exit() {
  StoReg("Software\\WikiContrib", "userlist", userlist.value)
  webapp_exit();
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


function scan(obj, elt, par="") {
  if (obj instanceof Object) {
    for (k in obj) {
      if (obj.hasOwnProperty(k)) scan(obj[k], elt, par+'['+k+']');
    }
  } else {
    txt=decodeEntities(par+':'+obj+'\n');
    if (elt) elt.value+=txt;
    //else console.log(txt);
  }
}

function isJson(item) {
  let value = typeof item !== "string" ? JSON.stringify(item) : item;
  try {
    value = JSON.parse(value);
  } catch (e) {
    return false;
  }

  return typeof value === "object" && value !== null;
}


async function mywiki() {
  const response = await fetch("https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-12-07&limit=1000", { headers: new Headers({ "Authorization": 'lalannd2:ocvdBum12$*4' }),});
//    "Authorization": `Basic ${base64.encode(`${login}:${password}`)}`
  const data = await response.json();
  console.log(data);
  output.value += JSON.stringify(data);
}

async function mywikiNOCRED() {
  const response = await fetch("https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-12-07&limit=1000");
  const data = await response.json();
  console.log(data);
  output.value += JSON.stringify(data);
}

async function mywikiRES() {
  const response = await fetch("https://wiki.space.thales/rest/api/content?limit=2", { credentials: 'include', });
  const data = await response.json();
  console.log(data);
}

function compute_contrib() {
  d1=new Date(start_date.value);
  d2=new Date(end_date.value);

  for (var d = d1; d <= d2; d.setDate(d.getDate() + 1)) {
    console.log(d.toISOString().split('T')[0]);
  }
}
