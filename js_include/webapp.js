
function is_webview () {
  if (typeof webapp_set_icon === "function") {
    //console.log("IS WEBVIEW");
    return true;
  } else return false;
}

var webapp = {
  set_title: function(s) {
    if (typeof webapp_set_title === "function") webapp_set_title(s);
    //else console.log("webapp_set_title does not exist");
  },
  set_icon: function(ic) {
    if (typeof webapp_set_icon === "function") webapp_set_icon(ic);
    //else console.log("webapp_set_icon does not exist");
  },
  restore: function() {
    if (typeof webapp_restore === "function") webapp_restore();
    //else console.log("webapp_restore does not exist");
  },
  set_size: function(w, h) {
    if (typeof webapp_set_size === "function") webapp_set_size(w, h);
    //else console.log("webapp_set_size does not exist");
  },
  set_pos: function(x, y) {
    if (typeof webapp_set_pos === "function") webapp_set_pos(x, y);
    //else console.log("webapp_set_pos does not exist");
  },
  on_move: function(func) {
    if (typeof webapp_on_move === "function") webapp_on_move(func);
    //else console.log("webapp_on_move does not exist");
  },
  hints: function(s) {
  /* 0 Width and height are default size
     1 Width and height are minimum bounds
     2 Width and height are maximum bounds
     3 Window size is fixed */
    if (typeof webapp_hints === "function") webapp_hints(hnts);
    //else console.log("webapp_hints does not exist");
  },
  getenv: function(s) {
    if (typeof getenv === "function") return getenv(s);
   // else console.log("getenv does not exist");
    return "";
  },

};

