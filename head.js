
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
//window.webapp_get_title().then(result => { console.log(result.value); });
}


function cons_or_not(res, obj) {
  var hlp_msg="";
  Object.keys(res).forEach(key => { hlp_msg+=key+": "+res[key]+"\n"; });
  if (obj instanceof HTMLElement) obj.innerHTML+=hlp_msg;
  else console.log(hlp_msg);
}

async function help (obj) {
  new Promise((resolve) => {
    window.webapp_help("tab").then(result => { resolve(cons_or_not(result, obj)); });
  });
}


async function ping (p) {
  console.log("ping " +p);
  output.innerHTML+=await new Promise((resolve) => {
    window.echo(p).then(result => { resolve(result+"\n"); });
  });
}

async function unicode (p) {
  console.log("unicode " +p);
  output.innerHTML+=await new Promise((resolve) => {
    window.utf(p).then(result => { resolve(result+"\n"); });
  });
}

function decodeHtml(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

function disp(msg, obj) {
  if (obj instanceof HTMLElement) obj.innerHTML+=msg;
  else console.log(msg);
}

function dir (path=".", obj=output) {
  function grant_in_number (val, sing, plur) {
    ret=val+' ';
    if (val > 1) {
      if (plur === "") ret+=sing+'s';
      else ret+=plur;
    } else ret+=sing;
    return ret;
  }

  function disp_files(root_path, list, obj)  {
    var res_list=[];
    var nfiles=0, ndirs=0, nothers=0;
    var spc=10;
    list.forEach((el, idx) => {
      line=show_status(el)+' ';
      if (el.type == file_type.regular) {
        nfiles++;
        if (el.size.toString().length < spc) {
          rp=spc-el.size.toString().length;
          line+=' '.repeat(rp)+el.size;
        } else line+=' '.repeat(spc);
      } else {
        line+=' '.repeat(spc);
        if (el.type == file_type.directory) ndirs++;
        else nothers++;
      }

      line+=' '+el.last_write+' '+el.file.replace(root_path, "").replace(/^\//, "");
      res_list.push(line);
      if (!obj) console.log(JSON.stringify(decodeHtml(line)));
    });

    if (obj) {
      obj.innerHTML+=root_path+"\n";
      obj.innerHTML+=res_list.join("\n");
      obj.innerHTML+="\n  "+grant_in_number(res_list.length, 'entry', 'entries');
      if (nfiles > 0) obj.innerHTML+=', '+grant_in_number(nfiles, 'file', 'files');
      if (ndirs > 0) obj.innerHTML+=', '+grant_in_number(ndirs, 'folder', 'folders');
      if (nothers > 0) obj.innerHTML+=', '+grant_in_number(nothers, 'of other type', 'other type');
      if (nfiles > 0) obj.innerHTML+="\n";
      obj.scrollTop=obj.scrollHeight;
    }
  }

  var full_list=[];
  return absolute(path).then((root_path) => {
    return ls(root_path).then((files) => {
      files.result.forEach((file, idx, arr) => {
        //file.path=file.path.replace(/\\/g, "\/");
        fstat(file.path).then((file_info) => {
          full_list.push(file_info);
          //console.log("Number of files "+files.result.length+", current file index "+full_list.length+", current file infos "+JSON.stringify(file_info));
          if (full_list.length == files.result.length) disp_files(root_path, full_list, obj);
        });
      });
    });
  });
}

// XMLHttpRequest does not work on thales (TAS) local sites because of CORS restriction on them but works for sites outside because of edge proxy configuration.
// Meanwhile curl request work on local sites but don't on external site because proxy not configured (although it could be done ...)
function curl_promise(func, param) {
  if (typeof param !== 'undefined') {
    this.httpget_res=param;
  } else {
    //console.log(this.httpget_res);
    //console.log(JSON.stringify(this.httpget_res));
    // <= > jq -r '.results[].title'
    this.httpget_res.results.forEach((el) => {
      output.value+=el.title+'\n';
    });
  }
}

function curl() {
  //promise_run(httpget, "https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-10-02&limit=1000", curl_promise);
  promise_run(httpget, "https://api.github.com/users/2nil-web", curl_promise);
}

function process_http_res (jres) {
  //console.log(JSON.stringify(jres));
  //console.log(jres);
  output.value += "login: "+jres.login;
  output.value += '\n';
  output.value += "url: "+jres.url;
}

function httpget_promise(func, param) {
  if (typeof param !== 'undefined') {
    this.httpget_res=param;
  } else {
    process_http_res(this.httget_res);
  }
}

function reqListener() {
  process_http_res(JSON.parse(this.responseText));
}

function http_query() {
  const req = new XMLHttpRequest();

  req.addEventListener("load", reqListener);
  req.open("GET", "https://api.github.com/users/2nil-web");
//  req.setRequestHeader("Access-Control-Allow-Origin", "*");
//  req.setRequestHeader("Authorization", "Basic " + btoa("lalannd2:ocvdBum12$*3"));
  req.send();
}

