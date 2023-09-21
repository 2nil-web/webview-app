
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
//window.webapp_get_title().then(result => { console.log(result.value); });
}
  
const file_type = {
  not_found: 0,      none: 1, regular: 2, directory: 3, symlink: 4,
      block: 5, character: 6,    fifo: 7,    socket: 8, unknown: 9
};

// Return file type in a string form
function ftype2s(t) {
  switch (t) {
    case file_type.none:
      return "`not-evaluated-yet`";
    case file_type.not_found:
      return "not found";
    case file_type.regular:
      return "regular file";
    case file_type.directory:
      return "directory";
    case file_type.symlink:
      return "symlink";
    case file_type.block:
      return "block device";
    case file_type.character:
      return "character device";
    case file_type.fifo:
      return "named IPC pipe";
    case file_type.socket:
      return "named IPC socket";
    case file_type.unknown:
      return "`unknown`";
    default:
      return "`implementation-defined`";
  }
}

const perms = {
        none:      0,
 others_exec:     01, others_write:    02, others_read:    04, others_all:    07,
  group_exec:    010,  group_write:   020,  group_read:   040,  group_all:   070,
  owner_exec:   0100,  owner_write:  0200,  owner_read:  0400,  owner_all:  0700,
         all:   0777,
  sticky_bit:  01000,      set_gid: 02000,     set_uid: 04000,       mask: 07777,
     unknown: 0xFFFF
};

// Return file permissions in a string form
function perm2s(p) {
  sp="";
  if (p === perms.unknown)			sp+="unknown,";
  else {
    if (p&perms.all)			        sp+="all,";
    else {
      if (p&perms.none)			      sp+="none,";
      if (p&perms.others_all)			sp+="others_all,";
      else {
        if (p&perms.others_exec)	sp+="others_exec,";
        if (p&perms.others_write)	sp+="others_write,";
        if (p&perms.others_read)	sp+="others_read,";
      }

      if (p&perms.group_all)		 sp+="group_all,";
      else {
        if (p&perms.group_exec)	 sp+="group_exec,";
        if (p&perms.group_write) sp+="group_write,";
        if (p&perms.group_read)	 sp+="group_read,";
      }

      if (p&perms.owner_all)		 sp+="owner_all,";
      else {
        if (p&perms.owner_exec)	 sp+="owner_exec,";
        if (p&perms.owner_write) sp+="owner_write,";
        if (p&perms.owner_read)	 sp+="owner_read,";
      }
    }


    if (p&perms.mask)			        sp+="mask,";
    else {
      if (p&perms.sticky_bit)			sp+="sticky_bit,";
      if (p&perms.set_gid)			  sp+="set_gid,";
      if (p&perms.set_uid)			  sp+="set_uid,";
    }
  }
  
  // Remove last comma
  sp=sp.slice(0, -1)
  return sp;
}

// Return file permissions in the form 'drwxrwxrwx'
function show_status(fsta) {
  function prm(op, p) { return fsta.perms & p ? op : '-'; }
  var ret="";
  ret+=(fsta.type === file_type.directory) ? 'd':'-';
  ret+=prm('r', perms.owner_read);
  ret+=prm('w', perms.owner_write);
  ret+=prm('x', perms.owner_exec);
  ret+=prm('r', perms.group_read);
  ret+=prm('w', perms.group_write);
  ret+=prm('x', perms.group_exec);
  ret+=prm('r', perms.others_read);
  ret+=prm('w', perms.others_write);
  ret+=prm('x', perms.others_exec);
  return ret;
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

  if (rec) l=lsr(dos);
  else l=ls(dos);
  var nfiles=0, ndirs=0, nothers=0;
  spc=10;
  txta=document.getElementById(stxta);

  var res="", log_res="";
  l.then(r => {
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
}

