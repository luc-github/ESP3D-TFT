#!/usr/bin/env python3

import os

#list of symbols to generate and their font file
symbols_array = [
[0xe568,'HEAT_BED','fa-solid-900.ttf'],
[0xf2c9,'EXTRUDER','fa-solid-900.ttf'],
[0xf0ca,'LIST','fa-solid-900.ttf'],
[0xf715,'SLASH','fa-solid-900.ttf'],
[0xf012,'STATION_MODE','fa-solid-900.ttf'],
[0xf519,'ACCESS_POINT','fa-solid-900.ttf'],
[0xf00c,'OK','fa-solid-900.ttf'],
[0xe596,'PROBE_CHECK','fa-solid-900.ttf'],
[0xf00d,'CLOSE','fa-solid-900.ttf'],
[0xf011,'POWER','fa-solid-900.ttf'],
[0xf028,'VOLUME_HIGH','fa-solid-900.ttf'],
[0xf027,'VOLUME_LOW','fa-solid-900.ttf'],
[0xf6a9,'VOLUME_OFF','fa-solid-900.ttf'],
[0xf013,'SETTINGS','fa-solid-900.ttf'],
[0xf2d1,'NO_HEAT_BED','fa-solid-900.ttf'],
[0xe040,'HEAT_EXTRUDER','fa-solid-900.ttf'],
[0xf2ed,'TRASH','fa-solid-900.ttf'],
[0xe3af,'HOME','fa-solid-900.ttf'],
[0xf019,'DOWNLOAD','fa-solid-900.ttf'],
[0xf021,'REFRESH','fa-solid-900.ttf'],
[0xf304,'EDIT','fa-solid-900.ttf'],
[0xf048,'PREVIOUS','fa-solid-900.ttf'],
[0xf051,'NEXT','fa-solid-900.ttf'],
[0xf04b,'PLAY','fa-solid-900.ttf'],
[0xf04c,'PAUSE','fa-solid-900.ttf'],
[0xf0c7,'SAVE','fa-solid-900.ttf'],
[0xf0e0,'MESSAGE','fa-solid-900.ttf'],
[0xf0e7,'LASER','fa-solid-900.ttf'],
[0xf76f,'VACCUM','fa-solid-900.ttf'],
[0xf1f6,'DISABLE_ALERT','fa-solid-900.ttf'],
[0xf023,'LOCK','fa-solid-900.ttf'],
[0xf2dc,'COOLANT','fa-solid-900.ttf'],
[0xf04d,'STOP','fa-solid-900.ttf'],
[0xf1eb,'WIFI','fa-solid-900.ttf'],
[0xf071,'WARNING','fa-solid-900.ttf'],
[0xf07b,'FOLDER','fa-solid-900.ttf'],
[0xf15b,'FILE','fa-solid-900.ttf'],
[0xf11c,'KEYBOARD','fa-solid-900.ttf'],
[0xf55a,'BACKSPACE','fa-solid-900.ttf'],
[0xf7c2,'SD_CARD','fa-solid-900.ttf'],
[0xf0b2,'JOG','fa-solid-900.ttf'],
[0xf077,'UP','fa-solid-900.ttf'],
[0xf078,'DOWN','fa-solid-900.ttf'],
[0xf053,'LEFT','fa-solid-900.ttf'],
[0xf054,'RIGHT','fa-solid-900.ttf'],
[0xf120,'COMMAND','fa-solid-900.ttf'],
[0xf624,'GAUGE','fa-solid-900.ttf'],
[0xf1ab,'LANGUAGE','fa-solid-900.ttf'],
[0xf863,'FAN','fa-solid-900.ttf'],
[0xf48b,'SPEED','fa-solid-900.ttf'],
[0xf72b,'WIZARD','fa-solid-900.ttf'],
[0xf185,'LIGHT','fa-solid-900.ttf'],
[0xf5c1,'ENGINE','fa-solid-900.ttf'],
[0xf5fd,'LAYERS','fa-solid-900.ttf'],
[0xe4b8,'LEVELING','fa-solid-900.ttf'],
[0xf4db,'FILAMENT','fa-solid-900.ttf'],
[0xe4bd,'CENTER','fa-solid-900.ttf'],
[0xf002,'SEARCH','fa-solid-900.ttf'],
[0xf4d7,'FILAMENT_SENSOR','fa-solid-900.ttf'],
[0xf2cc,'MIST','fa-solid-900.ttf'],
[0xf13e,'UNLOCK','fa-solid-900.ttf'],
[0xf192,'LASER_2','fa-solid-900.ttf'],
[0xe4c3,'MILLING','fa-solid-900.ttf'],
[0xf3e5,'NEW_LINE','fa-solid-900.ttf'],
[0xf293,'BLUETOOTH','fa-brands-400.ttf'],
[0xf287,'USB','fa-brands-400.ttf'],
[0xf0a1,'MORE_INFO','fa-solid-900.ttf'],
[0xf055,'PLUS','fa-solid-900.ttf'],
[0xf056,'MINUS','fa-solid-900.ttf']
]

#list of font sizes to generate
fonts_size_list = ["8","10","12","14","16","18","20","22","24","26","28","30","32","34","36","38","40","42","44","46","48"]

extra_symboles =" --symbols （），盗提陽帯鼻画輕ッ冊ェル写父ぁフ結想正四O夫源庭場天續鳥れ講猿苦階給了製守8祝己妳薄泣塩帰ぺ吃変輪那着仍嗯爭熱創味保字宿捨準查達肯ァ薬得査障該降察ね網加昼料等図邪秋コ態品屬久原殊候路願楽確針上被怕悲風份重歡っ附ぷ既4黨價娘朝凍僅際洋止右航よ专角應酸師個比則響健昇豐筆歷適修據細忙跟管長令家ザ期般花越ミ域泳通些油乏ラ。營ス返調農叫樹刊愛間包知把ヤ貧橋拡普聞前ジ建当繰ネ送習渇用補ィ覺體法遊宙ョ酔余利壊語くつ払皆時辺追奇そ們只胸械勝住全沈力光ん深溝二類北面社值試9和五勵ゃ貿幾逐打課ゲて領3鼓辦発評１渉詳暇込计駄供嘛郵頃腦反構絵お容規借身妻国慮剛急乗静必議置克土オ乎荷更肉還混古渡授合主離條値決季晴東大尚央州が嗎験流先医亦林田星晩拿60旅婦量為痛テ孫う環友況玩務其ぼち揺坐一肩腰犯タょ希即果ぶ物練待み高九找やヶ都グ去」サ、气仮雑酒許終企笑録形リ銀切ギ快問滿役単黄集森毎實研喜蘇司鉛洲川条媽ノ才兩話言雖媒出客づ卻現異故り誌逮同訊已視本題ぞを横開音第席費持眾怎選元退限ー賽処喝就残無いガ多ケ沒義遠歌隣錢某雪析嬉採自透き側員予ゼ白婚电へ顯呀始均畫似懸格車騒度わ親店週維億締慣免帳電甚來園浴ゅ愈京と杯各海怒ぜ排敗挙老買7極模実紀ヒ携隻告シ並屋這孩讓質ワブ富賃争康由辞マ火於短樣削弟材注節另室ダ招擁ぃ若套底波行勤關著泊背疲狭作念推ぐ民貸祖介說ビ代温契你我レ入描變再札ソ派頭智遅私聽舉灣山伸放直安ト誕煙付符幅ふ絡她届耳飲忘参革團仕様載ど歩獲嫌息の汚交興魚指資雙與館初学年幸史位柱族走括び考青也共腕Lで販擔理病イ今逃當寺猫邊菓係ム秘示解池影ド文例斷曾事茶寫明科桃藝売便え導禁財飛替而亡到し具空寝辛業ウ府セ國何基菜厳市努張缺雲根外だ断万砂ゴ超使台实ぽ礼最慧算軟界段律像夕丈窓助刻月夏政呼ぴざ擇趣除動従涼方勉名線対存請子氏將5少否諸論美感或西者定食御表は參歳緑命進易性錯房も捕皿判中觀戦ニ緩町ピ番ず金千ろ?不た象治関ャ每看徒卒統じ手範訪押座步号ベ旁以母すほ密減成往歲件緒読歯效院种七謂凝濃嵌震喉繼クュ拭死円2積水欲如ポにさ寒道區精啦姐ア聯能足及停思壓２春且メ裏株官答概黒過氷柿戻厚ぱ党祭織引計け委暗複誘港バ失下村較続神ぇ尤強秀膝兒来績十書済化服破新廠1紹您情半式產系好教暑早め樂地休協良な哪常要揮周かエ麗境働避護ンツ香夜太見設非改広聲他検求危清彼經未在起葉控靴所差內造寄南望尺換向展備眠點完約ぎ裡分説申童優伝島机須塊日立拉,鉄軽單気信很転識支布数紙此迎受心輸坊モ處「訳三曇兄野顔戰增ナ伊列又髪両有取左毛至困吧昔赤狀相夠整別士経頼然簡ホ会發隨営需脱ヨば接永居冬迫圍甘醫誰部充消連弱宇會咲覚姉麼的増首统帶糖朋術商担移景功育庫曲總劃牛程駅犬報ロ學責因パ嚴八世後平負公げ曜陸專午之閉ぬ談ご災昨冷職悪謝對它近射敢意運船臉局難什産頗!球真記ま但蔵究制機案湖臺ひ害券男留内木驗雨施種特復句末濟キ色訴依せ百型る石牠討呢时任執飯歐宅組傳配小活ゆべ暖ズ漸站素らボ束価チ浅回女片独妹英目從認生違策僕楚ペ米こ掛む爸六状落漢プ投カ校做啊洗声探あ割体項履触々訓技ハ低工映是標速善点人デ口次可"
symboles=""

fonts_files_list=[]
#generate font
def generateFont(sizefont):
	mainfont = "SimSun.woff"
	fontname = "lv_font_simsun_"
	rangemain= "0x20-0x7F,0xB0"
	source = "lv_font_conv --no-compress --no-prefilter --bpp 4 --size "+sizefont+" --font " + mainfont + " -r "+rangemain + extra_symboles +" --font FreeSerifBold.ttf -r 0x2022" +symboles+" --format bin -o fonts/"+fontname+sizefont+".bin --force-fast-kern-format"
	print("Generating "+sizefont+" px, "+fontname+sizefont+".c" )
	os.system(source)

#list all font files
for item in symbols_array:
	if item[2] not in fonts_files_list:
		fonts_files_list.append(item[2])
#generate symbols list for each font file
for font_file_name in fonts_files_list:
	symboles+=" --font "+font_file_name+" -r "
	for item in symbols_array:
		if item[2] == font_file_name:
			symboles=symboles+str(item[0])+","
	symboles = symboles[:-1]
#generate fonts for each size
for font_size in fonts_size_list:
	generateFont(font_size)


