
function def_promise_end (func, result) {
  var pmsg= "promesse";
  if (typeof func.name !== 'undefined') this.func_name=func.name;
  else this.func_name=""

  pmsg+=arguments.callee.name;
  pmsg+=' ';

  if (typeof result !== 'undefined') {
    this.result=result;
    pmsg+="résolue";
    if (this.result !== "") pmsg+=" avec le résultat '"+this.result+"'";
  } else {
    pmsg+="terminée";
    this.result="";
  }

  pmsg=gettime()+": "+pmsg+'.';
  console.log(pmsg);
}

function promise_run (func, param, promise_end=def_promise_end) {
  console.log(func.name);
  let promise;
  if (typeof param !== 'undefined') promise=func(param);
  else promise=func();


  if (typeof promise_end !== 'undefined') {
    promise.then(result => promise_end(func, result));
    promise.finally(() => promise_end(func));
  }
}

