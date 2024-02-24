
function is_webview () {
  if (typeof webapp_set_icon === "function") {
    //console.log("IS WEBVIEW");
    return true;
  } else return false;
}

function not_webapp_func () {
  console.log("Not a webapp function "+not_webapp_func.caller.name);
}

/*
 Reste à ajouter
  webapp_exec
  webapp_get_title
  webapp_get_title_bg
  webapp_hide
  webapp_minimize
  webapp_on_exit
*/

var webapp = {
  set_title: function(s) {
    if (typeof webapp_set_title === "function") webapp_set_title(s);
    else not_webapp_func();
  },
  set_icon: function(ic) {
    if (typeof webapp_set_icon === "function") webapp_set_icon(ic);
    else not_webapp_func();
  },
  restore: function() {
    if (typeof webapp_restore === "function") webapp_restore();
    else not_webapp_func();
  },
  set_size: function(w, h, hint=0) {
    if (typeof webapp_set_size === "function") webapp_set_size(w, h, hint);
    else not_webapp_func();
  },
  get_size: function() {
    if (typeof webapp_get_size === "function") return webapp_get_size();
    else not_webapp_func();
  },
  set_pos: function(x, y) {
    if (typeof webapp_set_pos === "function") webapp_set_pos(x, y);
    else not_webapp_func();
  },
  get_pos: function() {
    if (typeof webapp_get_pos === "function") return webapp_get_pos();
    else not_webapp_func();
  },
  on_move: function(func) {
    if (typeof webapp_on_move === "function") webapp_on_move(func);
    else not_webapp_func();
  },
  hints: function(hnts) {
  /* 0 Width and height are default size
     1 Width and height are minimum bounds
     2 Width and height are maximum bounds
     3 Window size is fixed */
    if (typeof webapp_hints === "function") webapp_hints(hnts);
    else not_webapp_func();
  },
  getenv: function(s) {
    if (typeof getenv === "function") return getenv(s);
    not_webapp_func();
    return "";
  },
  shell: function(s) {
    if (typeof webapp_shell === "function") return webapp_shell(s);
    not_webapp_func();
    return "";
  },
  exit: function() {
    if (typeof webapp_exit === "function") webapp_exit();
    else not_webapp_func();
  },
};

