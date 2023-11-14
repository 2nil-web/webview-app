
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
  //cmd.value ='readfile("content_and_name_ascii.txt");';
  cmd.value='readfile("content_and_name_unicode_çابتث背备本鼻.txt");';
  //cmd.value='readfile("content_ascii_and_name_unicode_éهوي调丁.txt");';
  //cmd.value='readfile("content_unicode_and_name_ascii.txt");';
  
  //cmd.value='readfile("version.h");';
  //cmd.value='help();';
  //cmd.value='curl();';
  //cmd.value='dir()';
  //cmd.value+='\nping("Latin abcdefghijklmnopqrstuvwxyz\\nArabe ابتثجحخدذرزسشصضطظعغفقكلمنهوي\\nChinois 啊爱安暗按八把爸吧白百拜班般板半办帮包保抱报爆杯北被背备本鼻比笔避必边便遍辨变标表别病并补不部布步才材采彩菜参草层曾茶察查差产长常场厂唱车彻称成城承程吃冲虫出初除楚处川穿传船窗床创春词此次从村存错答达打大带待代单但淡蛋当党导到道的得灯等低底地第弟点典电店掉调丁定冬东懂动都读独度短断段对队多朵躲饿儿而耳二发乏法反饭范方房防访放非飞费分坟份风封夫服福府父副复富妇该改概敢感干刚钢高搞告哥歌革隔格个给跟根更工公功共狗够构姑古骨故顾固瓜刮挂怪关观官馆管惯光广规鬼贵国果过还孩海害含汉好号喝河和何合黑很恨红后候呼忽乎湖胡虎户互护花华划画化话怀坏欢环换黄回会婚活火或货获机鸡积基极及集级急几己寄继际记济纪技计季家加假架价间简见建健件江将讲交饺脚角叫教较接街阶结节解姐介界今金斤仅紧近进尽京经精睛景静境究九酒久就旧救居局举句具据剧拒觉绝决军开看康考靠科可课刻客肯空孔口苦哭快筷块况困拉来浪劳老乐了累类冷离李里理礼立丽利历力例连联脸练凉两辆亮量谅疗料烈林零〇领另留流六龙楼路旅绿虑论落妈马吗买卖满慢忙毛么没美每门们")';
  //cmd.value ='ping("C:/Users/lalannd2/MyApps/home/Sources/8-webview-app/.git")';
}

cmd.focus();

function end_cmd() {
  output.scrollTop=output.scrollHeight;
  run_cmd.disabled=false;
  cmd.focus();
}

function exec_cmd_no_return(cmd_value, duration) {
  if (run_cmd.disabled === true) {
    tout_warn.value='Command ['+cmd_value+'] did not return after '+duration/1000+' seconds.';
    run_cmd.disabled=false;
  } else tout_warn.value='&nbsp;';

  cmd.focus();
}

function exec_cmd(cmd_value) {
    run_cmd.disabled = true;
    cmd_value=cmd_value.trim();
    tout_warn.value='&nbsp;';
    setTimeout(exec_cmd_no_return, tout.value, cmd_value, tout.value);
    window.webapp_exec(cmd_value, true).then(result => {
      output.value += decodeEntities(result.value);//.replace(/(?:\r\n|\r|\n)/g, '<br/>');
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

