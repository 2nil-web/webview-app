performance = performance || {};
performance.now = (function() {
  return performance.now       ||
         performance.mozNow    ||
         performance.msNow     ||
         performance.oNow      ||
         performance.webkitNow ||
         function() { return new Date().getTime(); };
})();

navStart=performance.now();

function gettime(subSec=10) {
  dt=new Date();
  res=dt.toLocaleTimeString();
  if (subSec) {
//    navStartMod=navStart%60000;
    perfNow=performance.now();
    perf=(perfNow-navStart);
    perfI=Math.trunc(perfNow-navStart);
    perf-=perfI;
    dtMi=dt.getMilliseconds();
    dtMa=dtMi+perf;

    //console.log("navStart "+navStart.toFixed(subSec)+", perfNow "+perfNow.toFixed(subSec)+", perf "+perf.toFixed(subSec)+", dtMi "+dtMi.toFixed(subSec)+", dtMa "+dtMa.toFixed(subSec));
    res+=':'+dtMa.toFixed(subSec);
  }
  return res;
}

timeNavStart=gettime();
function navDurationSec() {
  return (performance.now()-navStart)/1000;
}
var timer_init=0;      
function timer() {
  if (timer_init === 0) {
    timer_init=navDurationSec();
    return -1;          
  }
  var tdur=navDurationSec()-timer_init;
  timer_init=0;
  return tdur;
}

function gettimeMilli(milli=true) {
  dt=new Date();
  res=dt.toLocaleTimeString();
  if (milli) res+='.'+dt.getMilliseconds();
  return res;
}



function start_clock() {
  time.textContent=gettime(false);
  setInterval(() => { time.textContent=gettime(false); } , 400);
}

