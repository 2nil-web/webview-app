
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
//window.webapp_get_title().then(result => { console.log(result.value); });
}
  
function grant_in_number (val, sing, plur) {
    ret=val+' ';
    if (val > 1) {
      if (plur === "") ret+=sing+'s';
      else ret+=plur;
    } else ret+=sing;
    return ret;
}

// dir()
function dir(fld=".", stxta="output_text", rec=false) {
  var dirlist="[]\n", dos, finalRes="";

  fpwd=absolute(fld);

  fpwd.then((cwd) => {
    dos=cwd;
//    console.log("DOS "+dos);
    //chdir(cwd);
    dirlist=cwd+"\n";

  fpwd.finally(() => {
    console.log("DOS "+dos);
    dirlist=dos+"\n";
  });

  if (rec) lstdir=lsr(dos);
  else lstdir=ls(dos);
  var nfiles=0, ndirs=0, nothers=0;
  spc=10;
  txta=document.getElementById(stxta);

  var res="", log_res="";
  lstdir.then(r => {
    //console.log(dos);
    r.forEach((f) =>  {
      sta=fstat(f);
      sta.then(r => {
        re=/^(\.*\/)*/;
        f=f.replace(dos, "").replace(/^\//, "");
        res=log=show_status(r);
        res+=' ';

        if (r.type == file_type.regular) {
          nfiles++;
          if (r.size.toString().length < spc) {
            rp=spc-r.size.toString().length;
            res+=' '.repeat(rp)+r.size;
          } else res+=' '.repeat(spc);
        } else {
          res+=' '.repeat(spc);
          if (r.type == file_type.directory) ndirs++;
          else nothers++;
        }

        res+=' '+f;
        finalRes+=res+"\n";

        log+=','+f+','+ftype2s(r.type);
        if (r.type !== file_type.directory) log+=','+r.size;

        console.log(log);
      });
    });

    sta.finally(() => {
      //if (idx === r.length-1) {
      // Nombre d'entrées dans la liste, fichiers, répertoire et autres ...
      txta.value+="\n"+dirlist+finalRes+"  "+grant_in_number(r.length, 'entry', 'entries') +', '+grant_in_number(nfiles, 'file', 'files') +', '+grant_in_number(ndirs, 'folder', 'folders') +' and '+grant_in_number(nothers, 'of other type', 'other type');
      end_cmd();
      //}
    });
    });
  });

  lstdir.finally(() => {
  });
}

function dirRES(_pth=".", stxta="output_text", rec=false) {
  var nfiles=0, ndirs=0, nothers=0;
  var spc=10;
  var txta=document.getElementById(stxta);
  var renoas=/^\//;
  var absdir=absolute(_pth);
  absdir.then(cwd => {
    //console.log("cwd "+cwd);
    if (rec) lstdir=lsr(cwd);
    else lstdir=ls(cwd);

    var lines=[];
    lstdir.then(ld => {
      ld.forEach((elt, ie) =>  {
        _fstat=fstat(elt);
        _fstat.then(stat => {
          var line="";

          if (stat.type == file_type.regular) {
            nfiles++;
            if (stat.size.toString().length < spc) {
              rp=spc-stat.size.toString().length;
              line+=' '.repeat(rp)+stat.size;
            } else line+=' '.repeat(spc);
          } else {
            line+=' '.repeat(spc);
            if (stat.type == file_type.directory) ndirs++;
            else nothers++;
          }

          line+=' '+elt.replace(cwd, "").replace(renoas, "")+' '+show_status(stat);
          lines.push(line);
          //console.log("LINE"+ie+':'+line);
        });
        return Promise.all(lines);
      });
    });

    lstdir.finally(() => {
      lines.forEach((e, i) => { console.log(e); });
      txta.value+=cwd+"\n";
      txta.value+=lines.join("\n");
      txta.value+="  "+grant_in_number(lines.length, 'entry', 'entries') +', '+grant_in_number(nfiles, 'file', 'files') +', '+grant_in_number(ndirs, 'folder', 'folders') +' and '+grant_in_number(nothers, 'of other type', 'other type');
      end_cmd();
    });
  });
}

