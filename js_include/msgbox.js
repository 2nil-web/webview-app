
const mb = {
  ok: 0x000000, okcancel: 0x000001,
  abortretryignore: 0x000002,
  yesnocancel: 0x000003, yesno: 0x000004,
  retrycancel: 0x000005, canceltrycontinue: 0x000006,
  iconhand: 0x000010, iconquestion: 0x000020, iconexclamation: 0x000030, iconasterisk: 0x000040,
  iconstop: 0x000010, iconerror: 0x000010, iconwarning: 0x000030, iconinformation: 0x000040,
  defbutton1: 0x000000, defbutton2: 0x000100, defbutton3: 0x000200, defbutton4: 0x000300,
  applmodal: 0x000000, systemmodal: 0x001000, taskmodal: 0x002000,
  help: 0x004000,
  nofocus: 0x008000, setforeground: 0x010000,
  default_desktop_only: 0x020000,
  topmost: 0x040000, right: 0x080000,
  typemask: 0x00000F, iconmask: 0x0000F0, defmask: 0x000F00, modemask: 0x003000, miscmask: 0x00C000
}

const id = {
  ok: 1, cancel: 2, abort: 3, retry: 4, ignore: 5,
  yes: 6, no: 7, tryagain: 10, cont: 11
}

function infobox (msg) {
  if (is_webview() === true) {
    window.webapp_get_title().then(wtitle => { msgbox(msg, stem(wtitle), mb.ok); });
  } else {
    alert(msg);
  }
}

// Tester retour msgbox ...
function confirmbox (msg) {
  if (is_webview())
    window.webapp_get_title().then(wtitle => { msgbox(msg, document.title, mb.okcancel); });
  else
    confirm(msg);
}

