
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
//window.webapp_get_title().then(result => { console.log(result.value); });
}


function cons_or_notRES(res) {
  hlp_msg=res.replace(/\./g, ".\n");
  console.log(hlp_msg);
  output.innerHTML+=hlp_msg;
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

function dir(path=".", rec=false, dst_textarea=output) {
  //console.log("path "+path+", ta "+dst_textarea);
  function grant_in_number (val, sing, plur) {
      ret=val+' ';
      if (val > 1) {
        if (plur === "") ret+=sing+'s';
        else ret+=plur;
      } else ret+=sing;
      return ret;
  }

  var target_path="";
  var full_list=[];
  var list_size=0;
  function fstat_promise (func, param) {
    if (typeof param !== 'undefined') {
      //console.log("fs_prom1 "+param.file);
      this.elt=param;
    } else {
      console.log("fs_prom2 LS "+list_size+", L "+full_list.length+', perm '+this.elt.perms+', v '+this.elt.file);
      full_list.push(this.elt);

      if (full_list.length === list_size) {
        var res_list=[];
        var nfiles=0, ndirs=0, nothers=0;
        var spc=10;
        full_list.forEach((el, idx) => {
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

          line+=' '+el.last_write+' '+el.file.replace(target_path, "").replace(/^\//, "");
          res_list.push(line);
        });

        dst_textarea.innerHTML+=target_path+"\n";
        dst_textarea.innerHTML+=res_list.join("\n");
        dst_textarea.innerHTML+="\n  "+grant_in_number(res_list.length, 'entry', 'entries');
        if (nfiles > 0) dst_textarea.innerHTML+=', '+grant_in_number(nfiles, 'file', 'files');
        if (ndirs > 0) dst_textarea.innerHTML+=', '+grant_in_number(ndirs, 'folder', 'folders');
        if (nothers > 0) dst_textarea.innerHTML+=', '+grant_in_number(nothers, 'of other type', 'other type');
        if (nfiles > 0) dst_textarea.innerHTML+="\n";

        //console.log(dst_textarea.innerHTML);
        dst_textarea.scrollTop=dst_textarea.scrollHeight;
        target_path="";
        full_list=[];
        res_list=[];
        list_size=0;
      }
    }
  }

  function ls_promise (func, param) {
    if (typeof param !== 'undefined') {
      this.ls_res=param;
      console.log("A) ls_res2="+JSON.stringify(this.ls_res)+", len ls_res2 "+this.ls_res.result.length);
    } else {
      list_size=this.ls_res.result.length;
      console.log("B) TGT "+target_path);
      console.log("B) ls_res2="+JSON.stringify(this.ls_res)+", len ls_res2 "+this.ls_res.result.length);
      this.ls_res.result.forEach((elt, idx, arr) => {
        elt.path=elt.path.replace(/\\/g, "\/");
        //console.log("TXT "+elt.path.replace(target_path+"\/",""));
        promise_run(fstat, elt.path, fstat_promise);
      });
      this.ls_res="";
    }
  }

  function abs_promise (func, param) {
    if (typeof param !== 'undefined') {
      //console.log("target_path1 "+param);
      this.abs_res=param;
    } else {
      //console.log("target_path2 "+this.abs_res);
      target_path=this.abs_res;
      if (rec) promise_run(lsr, this.abs_res, ls_promise);
      else promise_run(ls, this.abs_res, ls_promise);
    }
  }

  promise_run(absolute, path, abs_promise);
}


// XMLHttpRequest does not work on thales (TAS) local sites because of CORS restriction on them but works for sites outside because of edge proxy configuration.
// Meanwhile curl request work on local sites but don't on external site because proxy not configured (although it could be done ...)
function curl_promise(func, param) {
  if (typeof param !== 'undefined') {
    this.httpget_res=param;
  } else {
    //console.log(this.httpget_res);
    console.log(JSON.stringify(this.httpget_res));
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

