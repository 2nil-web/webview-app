
var STRIP_COMMENTS = /((\/\/.*$)|(\/\*[\s\S]*?\*\/))/mg;
var ARGUMENT_NAMES = /([^\s,]+)/g;
function getParamNames(prom_func) {
  var fnStr = prom_func.toString().replace(STRIP_COMMENTS, '');
  var result = fnStr.slice(fnStr.indexOf('(')+1, fnStr.indexOf(')')).match(ARGUMENT_NAMES);
  if(result === null)
     result = [];
  return result;
}

function getParams(prom_func) {
 
    // String representation of the function code
    let str = prom_func.toString();
 
    // Remove comments of the form /* ... */
    // Removing comments of the form //
    // Remove body of the function { ... }
    // removing '=>' if prom_func is arrow function
    str = str.replace(/\/\*[\s\S]*?\*\//g, '')
        .replace(/\/\/(.)*/g, '')
        .replace(/{[\s\S]*}/, '')
        .replace(/=>/g, '')
        .trim();
 
    // Start parameter names after first '('
    let start = str.indexOf("(") + 1;
 
    // End parameter names is just before last ')'
    let end = str.length - 1;
 
    let result = str.substring(start, end).split(", ");
 
    let params = [];
 
    result.forEach(element => {
 
        // Removing any default value
        element = element.replace(/=[\s\S]*/g, '').trim();
 
        if (element.length > 0)
            params.push(element);
    });
 
    return params;
}

function basename(path) {
   return path.split('/').reverse()[0];
}

function del_ext(path) {
  return path.replace(/\.[^/.]+$/, "")
}

function stem (path) {
  return del_ext(basename(path));
}

// Polyfills
String.prototype.insert=function (str, pos) {
  return this.substring(0, pos)+str+this.substring(pos)
};


