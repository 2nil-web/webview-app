
document.addEventListener("keyup", (event) => { if (event.keyCode === 27) { webapp_exit(); } });

if (typeof webapp_title === "function") {
  // Default and minimal size
  //webapp_size(640, 400, 0);
  webapp_size(640, 400, 1);
//window.webapp_get_title().then(result => { console.log(result.value); });
}
  
function dir(path=".", rec=false, dst_textarea=output_text) {
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
      //console.log(param.file);
      this.elt=param;
    } else {
      //console.log("LS "+list_size+", L "+full_list.length+', v '+elt.file);
      full_list.push(elt);

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

        dst_textarea.value+=target_path+"\n";
        dst_textarea.value+=res_list.join("\n");
        dst_textarea.value+="\n  "+grant_in_number(res_list.length, 'entry', 'entries');
        if (nfiles > 0) dst_textarea.value+=', '+grant_in_number(nfiles, 'file', 'files');
        if (ndirs > 0) dst_textarea.value+=', '+grant_in_number(ndirs, 'folder', 'folders');
        if (nothers > 0) dst_textarea.value+=', '+grant_in_number(nothers, 'of other type', 'other type');
        if (nfiles > 0) dst_textarea.value+="\n";

        //console.log(dst_textarea.value);
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
    } else {
      list_size=ls_res.length;
      this.ls_res.forEach((elt, idx, arr) => {

        if (elt.hasOwnProperty('path_base64')) {
          //console.log("PB "+elt.path);
          promise_run(fstat, elt.path_base64, fstat_promise);
        } else {
          //console.log("PS "+elt.path);
          promise_run(fstat, elt.path, fstat_promise);
        }
      });
      this.ls_res=[];
    }
  }

  function abs_promise (func, param) {
    if (typeof param !== 'undefined') {
      this.abs_res=param;
    } else {
      target_path=this.abs_res;
      if (rec) promise_run(lsr, this.abs_res, ls_promise);
      else promise_run(ls, this.abs_res, ls_promise);
    }
  }

  promise_run(absolute, path, abs_promise);
}

