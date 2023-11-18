
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
  //window.webapp_get_title().then(result => { console.log(result); });
}

// Polyfills
String.prototype.insert=function (str, pos) {
  return this.substring(0, pos)+str+this.substring(pos)
};


function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

function cons_or_not(res, obj) {
  var hlp_msg="";
  Object.keys(res).forEach(key => { hlp_msg+=key+": "+res[key]+"\n"; });
  if (obj instanceof HTMLElement) obj.value+=decodeEntities(hlp_msg);
  else console.log(hlp_msg);
}

async function help (obj=output, fmt="") {
  window.webapp_help(fmt).then(help_msg => {
    if (fmt === "json") {
      help_msg=decodeEntities(JSON.stringify(help_msg, null, 0).replace(/[{}"]/g, "").replace(/,/g,"\n"));
    } else {
      help_msg=decodeEntities(help_msg);
    }
    if (obj instanceof HTMLElement) obj.value+=help_msg;
    console.log(help_msg);
  });
}

async function readfile (filename, obj=output) {
  window.fread(filename).then(txt => {
    txt=decodeEntities(txt);
    if (obj instanceof HTMLElement) obj.value+=txt;
    console.log(txt);
  });
}


async function raw_echo (p) {
  console.log("InParam " +p);
  output.value+=await new Promise((resolve) => {
    window.echo(p).then(result => {
      console.log("OutResult " +result);
      resolve(result+"\n");
    });
  });
}

async function ht_echo (p) {
  console.log("InParam " +p);
  output.value+=await new Promise((resolve) => {
    window.hecho(p).then(result => {
      ht_res=decodeEntities(result);
      console.log("OutResult " +result);
      resolve(ht_res+"\n");
    });
  });
}

async function unicode (p) {
  console.log("unicode " +p);
  output.value+=await new Promise((resolve) => {
    window.utf(p).then(result => { resolve(result+"\n"); });
  });
}

function dir (path=".", do_sort=false, obj=output) {
  function grant_in_number (val, sing, plur) {
    ret=val+' ';
    if (val > 1) {
      if (plur === "") ret+=sing+'s';
      else ret+=plur;
    } else ret+=sing;
    return ret;
  }

  var spacing=0;
  var nfiles=0, ndirs=0, nothers=0;
  function mkline (rt_path="", file_info) {
    var line;
    line=show_status(file_info)+' ';

    if (file_info.type == file_type.regular) {
      nfiles++;
      if (file_info.size.toString().length < spacing) {
        rp=spacing-file_info.size.toString().length;
        line+=' '.repeat(rp)+file_info.size;
      } else line+=' '.repeat(spacing);
    } else {
      line+=' '.repeat(spacing);
      if (file_info.type == file_type.directory) ndirs++;
      else nothers++;
    }

    line+=' '+file_info.last_write+' '+file_info.file.replace(rt_path, "").replace(/^\//, "").replace(/^\\/, "");
    //line=line.insert('#', 32);

    return line;
  }

  var full_list=[];
  var all_lines=[];
  return absolute(path).then((root_path) => {
    return ls(root_path).then((files) => {
      files.result.forEach((file, idx, arr) => {
        fstat(file.path).then((file_info) => {
          //console.log("nFiles "+files.result.length+", fIdx "+full_list.length+", fInf "+JSON.stringify(file_info));
          all_lines.push(mkline(root_path, file_info));
          if (all_lines.length == files.result.length) {
            if (do_sort) { // case insensitive sort on the file name
              all_lines.sort(function(a, b) {
                a1=a.substring(32).toLowerCase();
                b1=b.substring(32).toLowerCase();
                if (a1 > b1) return 1;
                else if (a1 < b1) return -1;
                return 0;
              });
            }

            var txt=root_path+"\n";
            txt+=all_lines.join("\n");
            txt+="\n  "+grant_in_number(all_lines.length, 'entry', 'entries');
            if (nfiles > 0) txt+=', '+grant_in_number(nfiles, 'file', 'files');
            if (ndirs > 0) txt+=', '+grant_in_number(ndirs, 'folder', 'folders');
            if (nothers > 0) txt+=', '+grant_in_number(nothers, 'of other type', 'other type');
            if (nfiles > 0) txt+="\n";


            if (obj) {
              obj.value+=decodeEntities(txt);
              obj.scrollTop=obj.scrollHeight;
            } else {
              console.log(txt);
            }
          }
        });
      });
    });
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

function curl(url="https://api.github.com/repos/octocat/Spoon-Knife/issues", name="octocat") {
  httpget(url).then((res) => {
    scan(res, output, name);
    console.log(res);
    console.log(JSON.stringify(res, null, 1));
  });
}

function curl_auth(url="https://api.github.com/user/repos", name="MyRepos", id="TestGHPerso 5e72ff515d8878dd456cea28d972521a8b44dfc3", pass="") {
  httpcred(id, pass);
  httpget(url).then((res) => {
    scan(res, output, name);
    console.log(JSON.stringify(res, null, 1));
  });
}

// https://docs.blague.xyz/
function blague() {
  httpget("https://blague.xyz/api/joke/random").then((res) => {
    output.value+=decodeEntities(res["joke"]["question"]+'\n'+res.joke.answer+'\n');
  });
}

function vdm() {
  httpget("https://blague.xyz/api/vdm/random").then((res) => {
    console.log(JSON.stringify(res));
    output.value+=decodeEntities(res.vdm.content+'\n');
  });
}

async function vdm2() {
  const response = await fetch("https://blague.xyz/api/vdm/random");
  const data = await response.json();
  console.log(data);
}

async function mygithub() {
  const response = await fetch("https://api.github.com/users/2nil-web", { mode: "cors" });
  const data = await response.json();
  console.log(data);
}

function github() {
  //curl("https://api.github.com/users/2nil-web")
  curl("https://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@api.github.com/user/repos", "MyRepos")
}

function reqListener() {
  console.log(JSON.parse(this.responseText));
}

function http_query(url="https://api.github.com/users/2nil-web") {
  const req = new XMLHttpRequest();

  req.addEventListener("load", reqListener);
  req.open("GET", url);
//  req.setRequestHeader("Access-Control-Allow-Origin", "*");
//  req.setRequestHeader("Authorization", "Basic " + btoa("lalannd2:ocvdBum12$*3"));
  req.send();
}


//https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-10-31&limit=1000


// Example POST method implementation:
async function postData(url = "", data = {}) {
  // Default options are marked with *
  const response = await fetch(url, {
    method: "GET", // *GET, POST, PUT, DELETE, etc.
    //mode: "*cors", // no-cors, *cors, same-origin
    cache: "no-cache", // *default, no-cache, reload, force-cache, only-if-cached
    //credentials: "same-origin", // include, *same-origin, omit
    /*headers: {
      "Content-Type": "application/json",
      // 'Content-Type': 'application/x-www-form-urlencoded',
    },*/
    redirect: "follow", // manual, *follow, error
    referrerPolicy: "no-referrer", // no-referrer, *no-referrer-when-downgrade, origin, origin-when-cross-origin, same-origin, strict-origin, strict-origin-when-cross-origin, unsafe-url
    //body: JSON.stringify(data), // body data type must match "Content-Type" header
  });
  return response.json(); // parses JSON response into native JavaScript objects
}

/*
postData("https://api.github.com/users/2nil-web", { answer: 42 }).then((data) => {
  console.log(data); // JSON data parsed by `data.json()` call
});
*/

function hdat(data) {
  console.log(data);
}

function jpfetch(url) {
  var scs = document.createElement("script");
  scs.setAttribute("id", "dynUrl");
  scs.setAttribute("src", url);
  scs.setAttribute('defer', '');
  scs.onload = function handleScriptLoaded(data) {
    console.log('script has loaded '+document.getElementById("dynUrl").crossOrigin);
    console.log(JSON.stringify(document.getElementById("dynUrl").value));
    //document.getElementById('box').textContent = 'Script loaded successfully';
  };

  scs.onerror = function handleScriptError() {
    console.log('error loading script');
  };
  document.head.appendChild(scs);
}


jpfetch("https://blague.xyz/api/vdm/random");

/*
const targetUrl = 'https://blague.xyz/api/vdm/random';
const proxyUrl = 'https://cors-anywhere.herokuapp.com/';

fetch(proxyUrl + targetUrl)
  .then(response => response.json())
  .then(data => console.log(data))
  .catch(error => console.error('Error:', error));

  const response = await fetch("https://blague.xyz/api/vdm/random");
  const data = await response.json();
  console.log(data);
*/
