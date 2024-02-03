#!/bin/bash

mkdir -p tmp_test && cd $_

read -r -d '' ASC_TXT <<-'EOF'
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYz
012356789
EOF

read -r -d '' UTF_TXT <<-'EOF'
Latin abcdefghijklmnopqrstuvwxyzéàèöüïëêûôîâä
Arabe ابتثجحخدذرزسشصضطظعغفقكلمنهوي
Chinois 啊爱安暗按八把爸吧白百拜班般板半办帮包保抱报爆杯北被背备本鼻
比笔避必边便遍辨变标表别病并补不部布步才材采彩菜参草层曾茶察查差产长
常场厂唱车彻称成城承程吃冲虫出初除楚处川穿传船窗床创春词此次从村存错
答达打大带待代单但淡蛋当党导到道的得灯等低底地第弟点典电店掉调丁定冬
东懂动都读独度短断段对队多朵躲饿儿而耳二发乏法反饭范方房防访放非飞费
分坟份风封夫服福府父副复富妇该改概敢感干刚钢高搞告哥歌革隔格个给跟根
更工公功共狗够构姑古骨故顾固瓜刮挂怪关观官馆管惯光广规鬼贵国果过还孩
海害含汉好号喝河和何合黑很恨红后候呼忽乎湖胡虎户互护花华划画化话怀坏
欢环换黄回会婚活火或货获机鸡积基极及集级急几己寄继际记济纪技计季家加
假架价间简见建健件江将讲交饺脚角叫教较接街阶结节解姐介界今金斤仅紧近
进尽京经精睛景静境究九酒久就旧救居局举句具据剧拒觉绝决军开看康考靠科
可课刻客肯空孔口苦哭快筷块况困拉来浪劳老乐了累类冷离李里理礼立丽利历
力例连联脸练凉两辆亮量谅疗料烈林零〇领另留流六龙楼路旅绿虑论落妈马吗
买卖满慢忙毛么没美每门们
EOF

echo $ASC_TXT >content_and_name_ascii.txt
echo $ASC_TXT >content_ascii_and_name_unicode_要らないéهوي调丁.txt
echo $UTF_TXT >content_unicode_and_name_ascii.txt
echo $UTF_TXT >content_and_name_unicode_ابتثجحخدذرزسشصضطظعغفقكلمنهوي.txt

