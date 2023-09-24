
function def_pthen (func, result) {
  var pmsg= "Promesse ";
  if (typeof func !== 'undefined') pmsg+=func.name+' ';
  pmsg+=" résolue à "+gettime();
  if (typeof result !== 'undefined') pmsg+=" avec le message '"+result;
  pmsg+=".";

  console.log(pmsg);
}

function def_pfinal (func) {
  var pmsg= "Promesse ";
  if (typeof func !== 'undefined') pmsg+=func.name+' ';

  pmsg+="terminée à "+gettime()+".";
  console.log(pmsg);
}

function promise_run (func, param="", pthen=def_pthen, pfinal=def_pfinal) {
  let promise;
  if (typeof param !== 'undefined') promise=func(param);
  else promise=func();
  if (typeof pthen !== 'undefined') promise.then(result => pthen(func, result));
  if (typeof pfinal !== 'undefined') promise.finally(() => pfinal(func));
}

