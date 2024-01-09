
loadreg();
var d=new Date(); // current date
d.setDate(1); // going to 1st of the month
d.setHours(-1); // going to the end of previous month
end_date.value=d.toISOString().split('T')[0];
d.setDate(1); // going to 1st of previous month
start_date.value=d.toISOString().split('T')[0];


