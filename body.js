
/*
window.ls("..").then(result => { console.log(result.value); });
file_status("x64");
file_status("webview-app.exe");
file_status("win.cpp");
file_status("NUL");
file_status("NotFound");
file_status("wv-runopt.o");
*/
if (cmd.value === "") {
  //cmd.value='dir()';
  cmd.value='help()';
}

cmd.focus();

function end_cmd() {
  output_text.scrollTop=output_text.scrollHeight;
  run_cmd.disabled=false;
  cmd.focus();
}

function exec_cmd_no_return(cmd_value, duration) {
  if (run_cmd.disabled === true) {
    tout_warn.innerHTML='Command ['+cmd_value+'] did not return after '+duration/1000+' seconds.';
    run_cmd.disabled=false;
  } else tout_warn.innerHTML='&nbsp;';

  cmd.focus();
}

function exec_cmd(cmd_value) {
    run_cmd.disabled = true;
    cmd_value=cmd_value.trim();
    tout_warn.innerHTML='&nbsp;';
    setTimeout(exec_cmd_no_return, tout.value, cmd_value, tout.value);
    window.webapp_exec(cmd_value, true).then(result => {
      output_text.value += result.value;//.replace(/(?:\r\n|\r|\n)/g, '<br/>');
      end_cmd();
    });
}

run_cmd.addEventListener( "click", (event) => { // Sur click "Run" on execute la commande comme shell ou javascript
  if (cmd.value !== "") {
    run_cmd.disabled = true;
    cmd.value=cmd.value.trim();

    if (as_js.checked) {
      eval(cmd.value);
      end_cmd();
    } else exec_cmd(cmd.value);
  }

  cmd.focus();
});


