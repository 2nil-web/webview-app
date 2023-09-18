
if (typeof webapp_title === "function") {
  webapp_size(640, 384, 0);
//window.webapp_get_title().then(result => { console.log(result.value); });
}
  
const file_type = {
  not_found: -1,      none: 0, regular: 1, directory: 2, symlink: 3,
      block:  4, character: 5,    fifo: 6,    socket: 7, unknown: 8
};

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

function show_status(fsta) {
  function prm(op, p) { return fsta.perms & p ? op : '-'; }
  function typ(op, p) { return fsta.perms & p ? op : '-'; }

  ret="";
  if (fsta.type === file_type.not_found) ret="          ";
  else {
    ret+=fsta.type === file_type.directory ? 'd':'-';
    ret+=prm('r', perms.owner_read);
    ret+=prm('w', perms.owner_write);
    ret+=prm('x', perms.owner_exec);
    ret+=prm('r', perms.group_read);
    ret+=prm('w', perms.group_write);
    ret+=prm('x', perms.group_exec);
    ret+=prm('r', perms.others_read);
    ret+=prm('w', perms.others_write);
    ret+=prm('x', perms.others_exec);
  }

  return ret;
}

function file_status(f, obj) {
  fstat(f).then(r => {
    f=f.replace(/^\.\//, "");
    res=show_status(r)+' '+f;
    obj.value+=res+"\n";
    r1=27-f.length;
    if (r1 < 1) r1=1;
    r2=7-+r.perms.toString(8).length;
    if (r2 < 1) r2=1;
    console.log(res);
  });
}

// dir()
function dir(fld=".", ta, rec=false) {
  if (rec) l=lsr(fld);
  else l=ls(fld);
  var nfiles=0, ndir=0, nothers=0;

  l.then(r=>{
     r.forEach(f=> {
      file_status(f, ta);
    });

    ta.value+=r.length+" file";
    if (r.length > 1) ta.value+='s';
    ta.value+="\n";
    end_cmd();
  });
}



