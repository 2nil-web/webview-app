
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

// Polyfills
String.prototype.insert=function (str, pos) {
  return this.substring(0, pos)+str+this.substring(pos)
};


function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
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

    line+=' '+file_info.last_write+' '+file_info.file.replace(rt_path, "").replace(/^\//, "");
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
      if (obj.hasOwnProperty(k)) scan(obj[k], elt, par+'.'+k);
    }
  } else {
    txt=decodeEntities(par+':'+obj+'\n');
    if (elt) elt.value+=txt;
    //else console.log(txt);
  }
}

// https://docs.blague.xyz/
// https://api.github.com/repos/octocat/Spoon-Knife/issues
// https://api.github.com/users/2nil-web") {
function curl(url="https://api.github.com/repos/octocat/Spoon-Knife/issues", par="octocat") {
  httpget(url, false, false).then((res) => {
    console.log(JSON.stringify(res));//.replace(/,/g, '\n').replace(/[\[{}\]]/g, '\n'));
    scan(res, output, par);
  });
}

// https://docs.blague.xyz/
function blague() {
  httpget("https://blague.xyz/api/joke/random").then((res) => {
    output.value+=decodeEntities(res.joke.question+'\n'+res.joke.answer+'\n');
  });
}

function vdm() {
  httpget("https://blague.xyz/api/vdm/random").then((res) => {
    //output.value+=JSON.stringify(res);
    output.value+=decodeEntities(res.vdm.content+'\n');
  });
}

function gh() {
  httpget("https://api.github.com/users/2nil-web", false, false).then((res) => {
    Object.keys(res).forEach(key => {
      output.value+=decodeEntities(key+": "+res[key]+"\n");
    });
  });
}

