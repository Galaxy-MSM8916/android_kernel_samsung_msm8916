/*
   'src_compress_deflate_inftrees.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
*/
#include"cobf.h"
#ifdef _WIN32
#if defined( UNDER_CE) && defined( bb334) || ! defined( bb341)
#define bb357 1
#define bb356 1
#else
#define bb335 bb350
#define bb358 1
#define bb348 1
#endif
#define bb360 1
#include"uncobf.h"
#include<ndis.h>
#include"cobf.h"
#ifdef UNDER_CE
#include"uncobf.h"
#include<ndiswan.h>
#include"cobf.h"
#endif
#include"uncobf.h"
#include<stdio.h>
#include<basetsd.h>
#include"cobf.h"
bba bbt bbl bbf, *bb3;bba bbt bbe bbo, *bb80;bba bb137 bb125, *bb351;
bba bbt bbl bb41, *bb73;bba bbt bb137 bbk, *bb59;bba bbe bbu, *bb134;
bba bbh bbf*bb79;
#ifdef bb307
bba bbd bb61, *bb124;
#endif
#else
#include"uncobf.h"
#include<linux/module.h>
#include<linux/ctype.h>
#include<linux/time.h>
#include<linux/slab.h>
#include"cobf.h"
#ifndef bb118
#define bb118
#ifdef _WIN32
#include"uncobf.h"
#include<wtypes.h>
#include"cobf.h"
#else
#ifdef bb121
#include"uncobf.h"
#include<linux/types.h>
#include"cobf.h"
#else
#include"uncobf.h"
#include<stddef.h>
#include<sys/types.h>
#include"cobf.h"
#endif
#endif
#ifdef _WIN32
#ifdef _MSC_VER
bba bb117 bb224;
#endif
#else
bba bbe bbu, *bb134, *bb216;
#define bb200 1
#define bb202 0
bba bb261 bb249, *bb205, *bb252;bba bbe bb278, *bb255, *bb227;bba bbt
bbo, *bb80, *bb215;bba bb8 bb266, *bb221;bba bbt bb8 bb226, *bb230;
bba bb8 bb119, *bb212;bba bbt bb8 bb63, *bb237;bba bb63 bb228, *bb251
;bba bb63 bb259, *bb220;bba bb119 bb117, *bb217;bba bb244 bb289;bba
bb210 bb125;bba bb262 bb85;bba bb112 bb116;bba bb112 bb235;
#ifdef bb234
bba bb233 bb41, *bb73;bba bb287 bbk, *bb59;bba bb209 bbd, *bb31;bba
bb222 bb57, *bb120;
#else
bba bb231 bb41, *bb73;bba bb253 bbk, *bb59;bba bb245 bbd, *bb31;bba
bb229 bb57, *bb120;
#endif
bba bb41 bbf, *bb3, *bb263;bba bbk bb206, *bb225, *bb286;bba bbk bb282
, *bb246, *bb284;bba bbd bb61, *bb124, *bb269;bba bb85 bb38, *bb241, *
bb223;bba bbd bb239, *bb265, *bb243;bba bb116 bb272, *bb213, *bb281;
bba bb57 bb270, *bb240, *bb208;
#define bb143 bbb
bba bbb*bb247, *bb81;bba bbh bbb*bb271;bba bbl bb218;bba bbl*bb207;
bba bbh bbl*bb62;
#if defined( bb121)
bba bbe bb115;
#endif
bba bb115 bb19;bba bb19*bb273;bba bbh bb19*bb186;
#if defined( bb268) || defined( bb248)
bba bb19 bb37;bba bb19 bb111;
#else
bba bbl bb37;bba bbt bbl bb111;
#endif
bba bbh bb37*bb279;bba bb37*bb277;bba bb61 bb211, *bb219;bba bbb*
bb107;bba bb107*bb257;
#define bb250( bb36) bbj bb36##__ { bbe bb267; }; bba bbj bb36##__  * \
 bb36
bba bbj{bb38 bb190,bb260,bb214,bb285;}bb232, *bb238, *bb283;bba bbj{
bb38 bb10,bb177;}bb254, *bb280, *bb242;bba bbj{bb38 bb264,bb275;}
bb274, *bb288, *bb276;
#endif
bba bbh bbf*bb79;
#endif
bba bbf bb103;
#define IN
#define OUT
#ifdef _DEBUG
#define bb147( bbc) bb27( bbc)
#else
#define bb147( bbc) ( bbb)( bbc)
#endif
bba bbe bb160, *bb172;
#define bb293 0
#define bb313 1
#define bb297 2
#define bb325 3
#define bb354 4
bba bbe bb361;bba bbb*bb123;
#endif
#ifdef _WIN32
#ifndef UNDER_CE
#define bb32 bb346
#define bb43 bb347
bba bbt bb8 bb32;bba bb8 bb43;
#endif
#else
#endif
#ifdef _WIN32
bbb*bb127(bb32 bb48);bbb bb108(bbb* );bbb*bb138(bb32 bb158,bb32 bb48);
#else
#define bb127( bbc) bb146(1, bbc, bb141)
#define bb108( bbc) bb340( bbc)
#define bb138( bbc, bbp) bb146( bbc, bbp, bb141)
#endif
#ifdef _WIN32
#define bb27( bbc) bb339( bbc)
#else
#ifdef _DEBUG
bbe bb145(bbh bbl*bb98,bbh bbl*bb26,bbt bb258);
#define bb27( bbc) ( bbb)(( bbc) || ( bb145(# bbc, __FILE__, __LINE__ \
)))
#else
#define bb27( bbc) (( bbb)0)
#endif
#endif
bb43 bb301(bb43*bb319);
#ifndef _WIN32
bbe bb331(bbh bbl*bbg);bbe bb320(bbh bbl*bb20,...);
#endif
#ifdef _WIN32
bba bb355 bb95;
#define bb142( bbc) bb353( bbc)
#define bb144( bbc) bb336( bbc)
#define bb135( bbc) bb359( bbc)
#define bb133( bbc) bb342( bbc)
#else
bba bb343 bb95;
#define bb142( bbc) ( bbb)(  *  bbc = bb337( bbc))
#define bb144( bbc) (( bbb)0)
#define bb135( bbc) bb352( bbc)
#define bb133( bbc) bb344( bbc)
#endif
#if ( defined( _WIN32) || defined( __WIN32__)) && ! defined( WIN32)
#define WIN32
#endif
#if defined( __GNUC__) || defined( WIN32) || defined( bb1257) ||  \
defined( bb1247)
#ifndef bb407
#define bb407
#endif
#endif
#if defined( __MSDOS__) && ! defined( bb169)
#define bb169
#endif
#if defined( bb169) && ! defined( bb407)
#define bb533
#endif
#ifdef bb169
#define bb1076
#endif
#if ( defined( bb169) || defined( bb1238) || defined( WIN32)) && !  \
defined( bb139)
#define bb139
#endif
#if defined( __STDC__) || defined( __cplusplus) || defined( bb1268)
#ifndef bb139
#define bb139
#endif
#endif
#ifndef bb139
#ifndef bbh
#define bbh
#endif
#endif
#if defined( __BORLANDC__) && ( __BORLANDC__ < 0x500)
#define bb1148
#endif
#ifndef bb292
#ifdef bb533
#define bb292 8
#else
#define bb292 9
#endif
#endif
#ifndef bbq
#ifdef bb139
#define bbq( bb421) bb421
#else
#define bbq( bb421) ()
#endif
#endif
bba bbf bb154;bba bbt bbe bb9;bba bbt bb8 bb25;bba bb154 bb34;bba bbl
bb447;bba bbe bb1171;bba bb9 bb165;bba bb25 bb167;
#ifdef bb139
bba bbb*bb72;bba bbb*bb191;
#else
bba bb154*bb72;bba bb154*bb191;
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bb72( *bb526)bbq((bb72 bb122,bb9 bb510,bb9 bb48));bba bbb( *bb524
)bbq((bb72 bb122,bb72 bb1138));bbj bb390;bba bbj bb1221{bb34*bb128;
bb9 bb149;bb25 bb193;bb34*bb619;bb9 bb397;bb25 bb642;bbl*bb327;bbj
bb390*bb23;bb526 bb414;bb524 bb379;bb72 bb122;bbe bb1000;bb25 bb377;
bb25 bb1189;}bb451;bba bb451*bb16;bbr bbh bbl*bb1196 bbq((bbb));bbr
bbe bb529 bbq((bb16 bb15,bbe bb176));bbr bbe bb971 bbq((bb16 bb15));
bbr bbe bb1086 bbq((bb16 bb15,bbe bb176));bbr bbe bb959 bbq((bb16 bb15
));bbr bbe bb1217 bbq((bb16 bb15,bbh bb34*bb441,bb9 bb449));bbr bbe
bb1188 bbq((bb16 bb132,bb16 bb185));bbr bbe bb1089 bbq((bb16 bb15));
bbr bbe bb1215 bbq((bb16 bb15,bbe bb126,bbe bb303));bbr bbe bb1218 bbq
((bb16 bb15,bbh bb34*bb441,bb9 bb449));bbr bbe bb1197 bbq((bb16 bb15));
bbr bbe bb1045 bbq((bb16 bb15));bbr bbe bb1187 bbq((bb34*bb132,bb167*
bb321,bbh bb34*bb185,bb25 bb332));bbr bbe bb1181 bbq((bb34*bb132,
bb167*bb321,bbh bb34*bb185,bb25 bb332,bbe bb126));bbr bbe bb1202 bbq(
(bb34*bb132,bb167*bb321,bbh bb34*bb185,bb25 bb332));bba bb191 bb39;
bbr bb39 bb1237 bbq((bbh bbl*bb1039,bbh bbl*bb45));bbr bb39 bb1239 bbq
((bbe bb486,bbh bbl*bb45));bbr bbe bb1262 bbq((bb39 bb26,bbe bb126,
bbe bb303));bbr bbe bb1226 bbq((bb39 bb26,bb191 bb42,bbt bb22));bbr
bbe bb1222 bbq((bb39 bb26,bbh bb191 bb42,bbt bb22));bbr bbe bb1248 bbq
((bb39 bb26,bbh bbl*bb1265,...));bbr bbe bb1223 bbq((bb39 bb26,bbh bbl
 *bbg));bbr bbl*bb1270 bbq((bb39 bb26,bbl*bb42,bbe bb22));bbr bbe
bb1242 bbq((bb39 bb26,bbe bbn));bbr bbe bb1271 bbq((bb39 bb26));bbr
bbe bb1259 bbq((bb39 bb26,bbe bb176));bbr bb8 bb1228 bbq((bb39 bb26,
bb8 bb97,bbe bb1233));bbr bbe bb1269 bbq((bb39 bb26));bbr bb8 bb1244
bbq((bb39 bb26));bbr bbe bb1231 bbq((bb39 bb26));bbr bbe bb1234 bbq((
bb39 bb26));bbr bbh bbl*bb1220 bbq((bb39 bb26,bbe*bb1267));bbr bb25
bb1018 bbq((bb25 bb377,bbh bb34*bb42,bb9 bb22));bbr bb25 bb1206 bbq((
bb25 bb391,bbh bb34*bb42,bb9 bb22));bbr bbe bb1149 bbq((bb16 bb15,bbe
bb126,bbh bbl*bb195,bbe bb196));bbr bbe bb1158 bbq((bb16 bb15,bbh bbl
 *bb195,bbe bb196));bbr bbe bb1124 bbq((bb16 bb15,bbe bb126,bbe bb590
,bbe bb466,bbe bb967,bbe bb303,bbh bbl*bb195,bbe bb196));bbr bbe
bb1122 bbq((bb16 bb15,bbe bb466,bbh bbl*bb195,bbe bb196));bbr bbh bbl
 *bb1209 bbq((bbe bb18));bbr bbe bb1214 bbq((bb16 bb0));bbr bbh bb167
 *bb1204 bbq((bbb));
#ifdef __cplusplus
}
#endif
#define bb1019 1
#ifdef bb139
#if defined( bb1773)
#else
#endif
#endif
bba bbt bbl bb156;bba bb156 bb1236;bba bbt bb137 bb130;bba bb130 bb521
;bba bbt bb8 bb410;bbr bbh bbl*bb1101[10 ];
#if bb292 >= 8
#define bb811 8
#else
#define bb811 bb292
#endif
#ifdef bb169
#define bb436 0x00
#if defined( __TURBOC__) || defined( __BORLANDC__)
#if( __STDC__ == 1) && ( defined( bb1831) || defined( bb1809))
bbb bb983 bb1377(bbb*bb105);bbb*bb983 bb1380(bbt bb8 bb1772);
#else
#include"uncobf.h"
#include<alloc.h>
#include"cobf.h"
#endif
#else
#include"uncobf.h"
#include<malloc.h>
#include"cobf.h"
#endif
#endif
#ifdef WIN32
#define bb436 0x0b
#endif
#if ( defined( _MSC_VER) && ( _MSC_VER > 600))
#define bb1786( bb486, bb131) bb1824( bb486, bb131)
#endif
#ifndef bb436
#define bb436 0x03
#endif
#if defined( bb1571) && ! defined( _MSC_VER) && ! defined( bb1811)
#define bb1019
#endif
bba bb25( *bb985)bbq((bb25 bb502,bbh bb34*bb42,bb9 bb22));bb72 bb1210
bbq((bb72 bb122,bbt bb510,bbt bb48));bbb bb1199 bbq((bb72 bb122,bb72
bb935));bba bbj bb1742 bb153;bbj bb1742{bb329{bbj{bb154 bb1212;bb154
bb989;}bb531;bb9 bb1301;}bb532;bb9 bb625;};bbr bbe bb2078 bbq((bb165*
,bb165* ,bb153* * ,bb153* ,bb16));bbr bbe bb2060 bbq((bb9,bb9,bb165* ,
bb165* ,bb165* ,bb153* * ,bb153* * ,bb153* ,bb16));bbr bbe bb2025 bbq
((bb165* ,bb165* ,bb153* * ,bb153* * ,bb16));
#if ! defined( bb2318) && ! defined( bb139)
#define bb2318
#endif
bbj bb390{bbe bb460;};bb40 bbe bb2033 bbq((bb165* ,bb9,bb9,bbh bb165*
,bbh bb165* ,bb153* * ,bb165* ,bb153* ,bb9* ,bb165* ));bb40 bbh bb9
bb2446[31 ]={3 ,4 ,5 ,6 ,7 ,8 ,9 ,10 ,11 ,13 ,15 ,17 ,19 ,23 ,27 ,31 ,35 ,43 ,51 ,59 ,67 ,
83 ,99 ,115 ,131 ,163 ,195 ,227 ,258 ,0 ,0 };bb40 bbh bb9 bb2434[31 ]={0 ,0 ,0 ,0 ,0
,0 ,0 ,0 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,0 ,112 ,112 };bb40 bbh bb9
bb2484[30 ]={1 ,2 ,3 ,4 ,5 ,7 ,9 ,13 ,17 ,25 ,33 ,49 ,65 ,97 ,129 ,193 ,257 ,385 ,513 ,
769 ,1025 ,1537 ,2049 ,3073 ,4097 ,6145 ,8193 ,12289 ,16385 ,24577 };bb40 bbh bb9
bb2485[30 ]={0 ,0 ,0 ,0 ,1 ,1 ,2 ,2 ,3 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,7 ,7 ,8 ,8 ,9 ,9 ,10 ,10 ,11 ,11 ,
12 ,12 ,13 ,13 };bb40 bbe bb2033(bbp,bb11,bbg,bbs,bbv,bb47,bb82,bb1823,
bb1826,bb448)bb165*bbp;bb9 bb11;bb9 bbg;bbh bb165*bbs;bbh bb165*bbv;
bb153* *bb47;bb165*bb82;bb153*bb1823;bb9*bb1826;bb165*bb448;{bb9 bbc;
bb9 bbn[15 +1 ];bb9 bb20;bbe bb55;bbe bb44;bb943 bb9 bbz;bb943 bb9 bb77
;bb943 bbe bb6;bbe bb178;bb9 bb1201;bb943 bb165*bb28;bb153*bb87;bbj
bb1742 bb24={{{0 }},0 };bb153*bb299[15 ];bb943 bbe bbw;bb9 bb10[15 +1 ];
bb165*bb2153;bbe bb177;bb9 bb0;bb28=bbn; *bb28++=0 ; *bb28++=0 ; *bb28
++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=
0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *bb28++=0 ; *
bb28++=0 ;bb28=bbp;bbz=bb11;bb599{bbn[ *bb28++]++;}bb109(--bbz);bbm(
bbn[0 ]==bb11){ *bb47=(bb153* )0 ; *bb82=0 ;bb4 0 ;}bb178= *bb82;bb90(
bb77=1 ;bb77<=15 ;bb77++)bbm(bbn[bb77])bb21;bb6=bb77;bbm((bb9)bb178<
bb77)bb178=bb77;bb90(bbz=15 ;bbz;bbz--)bbm(bbn[bbz])bb21;bb55=bbz;bbm(
(bb9)bb178>bbz)bb178=bbz; *bb82=bb178;bb90(bb177=1 <<bb77;bb77<bbz;
bb77++,bb177<<=1 )bbm((bb177-=bbn[bb77])<0 )bb4(-3 );bbm((bb177-=bbn[bbz
])<0 )bb4(-3 );bbn[bbz]+=bb177;bb10[1 ]=bb77=0 ;bb28=bbn+1 ;bb2153=bb10+2 ;
bb109(--bbz){ *bb2153++=(bb77+= *bb28++);}bb28=bbp;bbz=0 ;bb599{bbm((
bb77= *bb28++)!=0 )bb448[bb10[bb77]++]=bbz;}bb109(++bbz<bb11);bb11=
bb10[bb55];bb10[0 ]=bbz=0 ;bb28=bb448;bb44=-1 ;bbw=-bb178;bb299[0 ]=(
bb153* )0 ;bb87=(bb153* )0 ;bb0=0 ;bb90(;bb6<=bb55;bb6++){bbc=bbn[bb6];
bb109(bbc--){bb109(bb6>bbw+bb178){bb44++;bbw+=bb178;bb0=bb55-bbw;bb0=
bb0>(bb9)bb178?(bb9)bb178:bb0;bbm((bb20=1 <<(bb77=bb6-bbw))>bbc+1 ){
bb20-=bbc+1 ;bb2153=bbn+bb6;bbm(bb77<bb0)bb109(++bb77<bb0){bbm((bb20
<<=1 )<= * ++bb2153)bb21;bb20-= *bb2153;}}bb0=1 <<bb77;bbm( *bb1826+bb0
>1440 )bb4(-4 );bb299[bb44]=bb87=bb1823+ *bb1826; *bb1826+=bb0;bbm(bb44
){bb10[bb44]=bbz;bb24.bb532.bb531.bb989=(bb154)bb178;bb24.bb532.bb531
.bb1212=(bb154)bb77;bb77=bbz>>(bbw-bb178);bb24.bb625=(bb9)(bb87-bb299
[bb44-1 ]-bb77);bb299[bb44-1 ][bb77]=bb24;}bb50*bb47=bb87;}bb24.bb532.
bb531.bb989=(bb154)(bb6-bbw);bbm(bb28>=bb448+bb11)bb24.bb532.bb531.
bb1212=128 +64 ;bb50 bbm( *bb28<bbg){bb24.bb532.bb531.bb1212=(bb154)( *
bb28<256 ?0 :32 +64 );bb24.bb625= *bb28++;}bb50{bb24.bb532.bb531.bb1212=(
bb154)(bbv[ *bb28-bbg]+16 +64 );bb24.bb625=bbs[ *bb28++-bbg];}bb20=1 <<(
bb6-bbw);bb90(bb77=bbz>>bbw;bb77<bb0;bb77+=bb20)bb87[bb77]=bb24;bb90(
bb77=1 <<(bb6-1 );bbz&bb77;bb77>>=1 )bbz^=bb77;bbz^=bb77;bb1201=(1 <<bbw)-
1 ;bb109((bbz&bb1201)!=bb10[bb44]){bb44--;bbw-=bb178;bb1201=(1 <<bbw)-1
;}}}bb4 bb177!=0 &&bb55!=1 ?(-5 ):0 ;}bbe bb2078(bbn,bb1737,bb1808,bb1823
,bb0)bb165*bbn;bb165*bb1737;bb153* *bb1808;bb153*bb1823;bb16 bb0;{bbe
bb24;bb9 bb1826=0 ;bb165*bb448;bbm((bb448=(bb165* )( * ((bb0)->bb414))(
(bb0)->bb122,(19 ),(bb12(bb9))))==0 )bb4(-4 );bb24=bb2033(bbn,19 ,19 ,(
bb165* )0 ,(bb165* )0 ,bb1808,bb1737,bb1823,&bb1826,bb448);bbm(bb24==(-
3 ))bb0->bb327=(bbl* )"";bb50 bbm(bb24==(-5 )|| *bb1737==0 ){bb0->bb327=
(bbl* )"";bb24=(-3 );}( * ((bb0)->bb379))((bb0)->bb122,(bb72)(bb448));
bb4 bb24;}bbe bb2060(bb2252,bb2468,bbn,bb58,bb966,bb1053,bb1036,
bb1823,bb0)bb9 bb2252;bb9 bb2468;bb165*bbn;bb165*bb58;bb165*bb966;
bb153* *bb1053;bb153* *bb1036;bb153*bb1823;bb16 bb0;{bbe bb24;bb9
bb1826=0 ;bb165*bb448;bbm((bb448=(bb165* )( * ((bb0)->bb414))((bb0)->
bb122,(288 ),(bb12(bb9))))==0 )bb4(-4 );bb24=bb2033(bbn,bb2252,257 ,
bb2446,bb2434,bb1053,bb58,bb1823,&bb1826,bb448);bbm(bb24!=0 || *bb58==
0 ){bbm(bb24==(-3 ))bb0->bb327=(bbl* )"";bb50 bbm(bb24!=(-4 )){bb0->
bb327=(bbl* )"";bb24=(-3 );}( * ((bb0)->bb379))((bb0)->bb122,(bb72)(
bb448));bb4 bb24;}bb24=bb2033(bbn+bb2252,bb2468,0 ,bb2484,bb2485,
bb1036,bb966,bb1823,&bb1826,bb448);bbm(bb24!=0 ||( *bb966==0 &&bb2252>
257 )){bbm(bb24==(-3 ))bb0->bb327=(bbl* )"";bb50 bbm(bb24==(-5 )){bb0->
bb327=(bbl* )"";bb24=(-3 );}bb50 bbm(bb24!=(-4 )){bb0->bb327=(bbl* )"";
bb24=(-3 );}( * ((bb0)->bb379))((bb0)->bb122,(bb72)(bb448));bb4 bb24;}
( * ((bb0)->bb379))((bb0)->bb122,(bb72)(bb448));bb4 0 ;}
#ifdef bb2318
bb40 bbe bb2409=0 ;
#define bb2617 544
bb40 bb153 bb2510[bb2617];bb40 bb9 bb2200;bb40 bb9 bb2201;bb40 bb153*
bb2333;bb40 bb153*bb2338;
#else
bb40 bb9 bb2200=9 ;bb40 bb9 bb2201=5 ;bb40 bb153 bb2333[]={{{{96 ,7 }},
256 },{{{0 ,8 }},80 },{{{0 ,8 }},16 },{{{84 ,8 }},115 },{{{82 ,7 }},31 },{{{0 ,8 }},
112 },{{{0 ,8 }},48 },{{{0 ,9 }},192 },{{{80 ,7 }},10 },{{{0 ,8 }},96 },{{{0 ,8 }},
32 },{{{0 ,9 }},160 },{{{0 ,8 }},0 },{{{0 ,8 }},128 },{{{0 ,8 }},64 },{{{0 ,9 }},224
},{{{80 ,7 }},6 },{{{0 ,8 }},88 },{{{0 ,8 }},24 },{{{0 ,9 }},144 },{{{83 ,7 }},59 },
{{{0 ,8 }},120 },{{{0 ,8 }},56 },{{{0 ,9 }},208 },{{{81 ,7 }},17 },{{{0 ,8 }},104 },
{{{0 ,8 }},40 },{{{0 ,9 }},176 },{{{0 ,8 }},8 },{{{0 ,8 }},136 },{{{0 ,8 }},72 },{{{
0 ,9 }},240 },{{{80 ,7 }},4 },{{{0 ,8 }},84 },{{{0 ,8 }},20 },{{{85 ,8 }},227 },{{{
83 ,7 }},43 },{{{0 ,8 }},116 },{{{0 ,8 }},52 },{{{0 ,9 }},200 },{{{81 ,7 }},13 },{{{
0 ,8 }},100 },{{{0 ,8 }},36 },{{{0 ,9 }},168 },{{{0 ,8 }},4 },{{{0 ,8 }},132 },{{{0 ,
8 }},68 },{{{0 ,9 }},232 },{{{80 ,7 }},8 },{{{0 ,8 }},92 },{{{0 ,8 }},28 },{{{0 ,9 }}
,152 },{{{84 ,7 }},83 },{{{0 ,8 }},124 },{{{0 ,8 }},60 },{{{0 ,9 }},216 },{{{82 ,7 }
},23 },{{{0 ,8 }},108 },{{{0 ,8 }},44 },{{{0 ,9 }},184 },{{{0 ,8 }},12 },{{{0 ,8 }},
140 },{{{0 ,8 }},76 },{{{0 ,9 }},248 },{{{80 ,7 }},3 },{{{0 ,8 }},82 },{{{0 ,8 }},18
},{{{85 ,8 }},163 },{{{83 ,7 }},35 },{{{0 ,8 }},114 },{{{0 ,8 }},50 },{{{0 ,9 }},
196 },{{{81 ,7 }},11 },{{{0 ,8 }},98 },{{{0 ,8 }},34 },{{{0 ,9 }},164 },{{{0 ,8 }},2
},{{{0 ,8 }},130 },{{{0 ,8 }},66 },{{{0 ,9 }},228 },{{{80 ,7 }},7 },{{{0 ,8 }},90 },
{{{0 ,8 }},26 },{{{0 ,9 }},148 },{{{84 ,7 }},67 },{{{0 ,8 }},122 },{{{0 ,8 }},58 },{
{{0 ,9 }},212 },{{{82 ,7 }},19 },{{{0 ,8 }},106 },{{{0 ,8 }},42 },{{{0 ,9 }},180 },{
{{0 ,8 }},10 },{{{0 ,8 }},138 },{{{0 ,8 }},74 },{{{0 ,9 }},244 },{{{80 ,7 }},5 },{{{
0 ,8 }},86 },{{{0 ,8 }},22 },{{{192 ,8 }},0 },{{{83 ,7 }},51 },{{{0 ,8 }},118 },{{{0
,8 }},54 },{{{0 ,9 }},204 },{{{81 ,7 }},15 },{{{0 ,8 }},102 },{{{0 ,8 }},38 },{{{0 ,
9 }},172 },{{{0 ,8 }},6 },{{{0 ,8 }},134 },{{{0 ,8 }},70 },{{{0 ,9 }},236 },{{{80 ,7
}},9 },{{{0 ,8 }},94 },{{{0 ,8 }},30 },{{{0 ,9 }},156 },{{{84 ,7 }},99 },{{{0 ,8 }},
126 },{{{0 ,8 }},62 },{{{0 ,9 }},220 },{{{82 ,7 }},27 },{{{0 ,8 }},110 },{{{0 ,8 }},
46 },{{{0 ,9 }},188 },{{{0 ,8 }},14 },{{{0 ,8 }},142 },{{{0 ,8 }},78 },{{{0 ,9 }},
252 },{{{96 ,7 }},256 },{{{0 ,8 }},81 },{{{0 ,8 }},17 },{{{85 ,8 }},131 },{{{82 ,7 }
},31 },{{{0 ,8 }},113 },{{{0 ,8 }},49 },{{{0 ,9 }},194 },{{{80 ,7 }},10 },{{{0 ,8 }}
,97 },{{{0 ,8 }},33 },{{{0 ,9 }},162 },{{{0 ,8 }},1 },{{{0 ,8 }},129 },{{{0 ,8 }},65
},{{{0 ,9 }},226 },{{{80 ,7 }},6 },{{{0 ,8 }},89 },{{{0 ,8 }},25 },{{{0 ,9 }},146 },
{{{83 ,7 }},59 },{{{0 ,8 }},121 },{{{0 ,8 }},57 },{{{0 ,9 }},210 },{{{81 ,7 }},17 },
{{{0 ,8 }},105 },{{{0 ,8 }},41 },{{{0 ,9 }},178 },{{{0 ,8 }},9 },{{{0 ,8 }},137 },{{
{0 ,8 }},73 },{{{0 ,9 }},242 },{{{80 ,7 }},4 },{{{0 ,8 }},85 },{{{0 ,8 }},21 },{{{80
,8 }},258 },{{{83 ,7 }},43 },{{{0 ,8 }},117 },{{{0 ,8 }},53 },{{{0 ,9 }},202 },{{{
81 ,7 }},13 },{{{0 ,8 }},101 },{{{0 ,8 }},37 },{{{0 ,9 }},170 },{{{0 ,8 }},5 },{{{0 ,
8 }},133 },{{{0 ,8 }},69 },{{{0 ,9 }},234 },{{{80 ,7 }},8 },{{{0 ,8 }},93 },{{{0 ,8 }
},29 },{{{0 ,9 }},154 },{{{84 ,7 }},83 },{{{0 ,8 }},125 },{{{0 ,8 }},61 },{{{0 ,9 }}
,218 },{{{82 ,7 }},23 },{{{0 ,8 }},109 },{{{0 ,8 }},45 },{{{0 ,9 }},186 },{{{0 ,8 }}
,13 },{{{0 ,8 }},141 },{{{0 ,8 }},77 },{{{0 ,9 }},250 },{{{80 ,7 }},3 },{{{0 ,8 }},
83 },{{{0 ,8 }},19 },{{{85 ,8 }},195 },{{{83 ,7 }},35 },{{{0 ,8 }},115 },{{{0 ,8 }},
51 },{{{0 ,9 }},198 },{{{81 ,7 }},11 },{{{0 ,8 }},99 },{{{0 ,8 }},35 },{{{0 ,9 }},
166 },{{{0 ,8 }},3 },{{{0 ,8 }},131 },{{{0 ,8 }},67 },{{{0 ,9 }},230 },{{{80 ,7 }},7
},{{{0 ,8 }},91 },{{{0 ,8 }},27 },{{{0 ,9 }},150 },{{{84 ,7 }},67 },{{{0 ,8 }},123 }
,{{{0 ,8 }},59 },{{{0 ,9 }},214 },{{{82 ,7 }},19 },{{{0 ,8 }},107 },{{{0 ,8 }},43 },
{{{0 ,9 }},182 },{{{0 ,8 }},11 },{{{0 ,8 }},139 },{{{0 ,8 }},75 },{{{0 ,9 }},246 },{
{{80 ,7 }},5 },{{{0 ,8 }},87 },{{{0 ,8 }},23 },{{{192 ,8 }},0 },{{{83 ,7 }},51 },{{{
0 ,8 }},119 },{{{0 ,8 }},55 },{{{0 ,9 }},206 },{{{81 ,7 }},15 },{{{0 ,8 }},103 },{{{
0 ,8 }},39 },{{{0 ,9 }},174 },{{{0 ,8 }},7 },{{{0 ,8 }},135 },{{{0 ,8 }},71 },{{{0 ,9
}},238 },{{{80 ,7 }},9 },{{{0 ,8 }},95 },{{{0 ,8 }},31 },{{{0 ,9 }},158 },{{{84 ,7 }
},99 },{{{0 ,8 }},127 },{{{0 ,8 }},63 },{{{0 ,9 }},222 },{{{82 ,7 }},27 },{{{0 ,8 }}
,111 },{{{0 ,8 }},47 },{{{0 ,9 }},190 },{{{0 ,8 }},15 },{{{0 ,8 }},143 },{{{0 ,8 }},
79 },{{{0 ,9 }},254 },{{{96 ,7 }},256 },{{{0 ,8 }},80 },{{{0 ,8 }},16 },{{{84 ,8 }},
115 },{{{82 ,7 }},31 },{{{0 ,8 }},112 },{{{0 ,8 }},48 },{{{0 ,9 }},193 },{{{80 ,7 }}
,10 },{{{0 ,8 }},96 },{{{0 ,8 }},32 },{{{0 ,9 }},161 },{{{0 ,8 }},0 },{{{0 ,8 }},128
},{{{0 ,8 }},64 },{{{0 ,9 }},225 },{{{80 ,7 }},6 },{{{0 ,8 }},88 },{{{0 ,8 }},24 },{
{{0 ,9 }},145 },{{{83 ,7 }},59 },{{{0 ,8 }},120 },{{{0 ,8 }},56 },{{{0 ,9 }},209 },{
{{81 ,7 }},17 },{{{0 ,8 }},104 },{{{0 ,8 }},40 },{{{0 ,9 }},177 },{{{0 ,8 }},8 },{{{
0 ,8 }},136 },{{{0 ,8 }},72 },{{{0 ,9 }},241 },{{{80 ,7 }},4 },{{{0 ,8 }},84 },{{{0 ,
8 }},20 },{{{85 ,8 }},227 },{{{83 ,7 }},43 },{{{0 ,8 }},116 },{{{0 ,8 }},52 },{{{0 ,
9 }},201 },{{{81 ,7 }},13 },{{{0 ,8 }},100 },{{{0 ,8 }},36 },{{{0 ,9 }},169 },{{{0 ,
8 }},4 },{{{0 ,8 }},132 },{{{0 ,8 }},68 },{{{0 ,9 }},233 },{{{80 ,7 }},8 },{{{0 ,8 }}
,92 },{{{0 ,8 }},28 },{{{0 ,9 }},153 },{{{84 ,7 }},83 },{{{0 ,8 }},124 },{{{0 ,8 }},
60 },{{{0 ,9 }},217 },{{{82 ,7 }},23 },{{{0 ,8 }},108 },{{{0 ,8 }},44 },{{{0 ,9 }},
185 },{{{0 ,8 }},12 },{{{0 ,8 }},140 },{{{0 ,8 }},76 },{{{0 ,9 }},249 },{{{80 ,7 }},
3 },{{{0 ,8 }},82 },{{{0 ,8 }},18 },{{{85 ,8 }},163 },{{{83 ,7 }},35 },{{{0 ,8 }},
114 },{{{0 ,8 }},50 },{{{0 ,9 }},197 },{{{81 ,7 }},11 },{{{0 ,8 }},98 },{{{0 ,8 }},
34 },{{{0 ,9 }},165 },{{{0 ,8 }},2 },{{{0 ,8 }},130 },{{{0 ,8 }},66 },{{{0 ,9 }},229
},{{{80 ,7 }},7 },{{{0 ,8 }},90 },{{{0 ,8 }},26 },{{{0 ,9 }},149 },{{{84 ,7 }},67 },
{{{0 ,8 }},122 },{{{0 ,8 }},58 },{{{0 ,9 }},213 },{{{82 ,7 }},19 },{{{0 ,8 }},106 },
{{{0 ,8 }},42 },{{{0 ,9 }},181 },{{{0 ,8 }},10 },{{{0 ,8 }},138 },{{{0 ,8 }},74 },{{
{0 ,9 }},245 },{{{80 ,7 }},5 },{{{0 ,8 }},86 },{{{0 ,8 }},22 },{{{192 ,8 }},0 },{{{
83 ,7 }},51 },{{{0 ,8 }},118 },{{{0 ,8 }},54 },{{{0 ,9 }},205 },{{{81 ,7 }},15 },{{{
0 ,8 }},102 },{{{0 ,8 }},38 },{{{0 ,9 }},173 },{{{0 ,8 }},6 },{{{0 ,8 }},134 },{{{0 ,
8 }},70 },{{{0 ,9 }},237 },{{{80 ,7 }},9 },{{{0 ,8 }},94 },{{{0 ,8 }},30 },{{{0 ,9 }}
,157 },{{{84 ,7 }},99 },{{{0 ,8 }},126 },{{{0 ,8 }},62 },{{{0 ,9 }},221 },{{{82 ,7 }
},27 },{{{0 ,8 }},110 },{{{0 ,8 }},46 },{{{0 ,9 }},189 },{{{0 ,8 }},14 },{{{0 ,8 }},
142 },{{{0 ,8 }},78 },{{{0 ,9 }},253 },{{{96 ,7 }},256 },{{{0 ,8 }},81 },{{{0 ,8 }},
17 },{{{85 ,8 }},131 },{{{82 ,7 }},31 },{{{0 ,8 }},113 },{{{0 ,8 }},49 },{{{0 ,9 }},
195 },{{{80 ,7 }},10 },{{{0 ,8 }},97 },{{{0 ,8 }},33 },{{{0 ,9 }},163 },{{{0 ,8 }},1
},{{{0 ,8 }},129 },{{{0 ,8 }},65 },{{{0 ,9 }},227 },{{{80 ,7 }},6 },{{{0 ,8 }},89 },
{{{0 ,8 }},25 },{{{0 ,9 }},147 },{{{83 ,7 }},59 },{{{0 ,8 }},121 },{{{0 ,8 }},57 },{
{{0 ,9 }},211 },{{{81 ,7 }},17 },{{{0 ,8 }},105 },{{{0 ,8 }},41 },{{{0 ,9 }},179 },{
{{0 ,8 }},9 },{{{0 ,8 }},137 },{{{0 ,8 }},73 },{{{0 ,9 }},243 },{{{80 ,7 }},4 },{{{0
,8 }},85 },{{{0 ,8 }},21 },{{{80 ,8 }},258 },{{{83 ,7 }},43 },{{{0 ,8 }},117 },{{{0
,8 }},53 },{{{0 ,9 }},203 },{{{81 ,7 }},13 },{{{0 ,8 }},101 },{{{0 ,8 }},37 },{{{0 ,
9 }},171 },{{{0 ,8 }},5 },{{{0 ,8 }},133 },{{{0 ,8 }},69 },{{{0 ,9 }},235 },{{{80 ,7
}},8 },{{{0 ,8 }},93 },{{{0 ,8 }},29 },{{{0 ,9 }},155 },{{{84 ,7 }},83 },{{{0 ,8 }},
125 },{{{0 ,8 }},61 },{{{0 ,9 }},219 },{{{82 ,7 }},23 },{{{0 ,8 }},109 },{{{0 ,8 }},
45 },{{{0 ,9 }},187 },{{{0 ,8 }},13 },{{{0 ,8 }},141 },{{{0 ,8 }},77 },{{{0 ,9 }},
251 },{{{80 ,7 }},3 },{{{0 ,8 }},83 },{{{0 ,8 }},19 },{{{85 ,8 }},195 },{{{83 ,7 }},
35 },{{{0 ,8 }},115 },{{{0 ,8 }},51 },{{{0 ,9 }},199 },{{{81 ,7 }},11 },{{{0 ,8 }},
99 },{{{0 ,8 }},35 },{{{0 ,9 }},167 },{{{0 ,8 }},3 },{{{0 ,8 }},131 },{{{0 ,8 }},67 }
,{{{0 ,9 }},231 },{{{80 ,7 }},7 },{{{0 ,8 }},91 },{{{0 ,8 }},27 },{{{0 ,9 }},151 },{
{{84 ,7 }},67 },{{{0 ,8 }},123 },{{{0 ,8 }},59 },{{{0 ,9 }},215 },{{{82 ,7 }},19 },{
{{0 ,8 }},107 },{{{0 ,8 }},43 },{{{0 ,9 }},183 },{{{0 ,8 }},11 },{{{0 ,8 }},139 },{{
{0 ,8 }},75 },{{{0 ,9 }},247 },{{{80 ,7 }},5 },{{{0 ,8 }},87 },{{{0 ,8 }},23 },{{{
192 ,8 }},0 },{{{83 ,7 }},51 },{{{0 ,8 }},119 },{{{0 ,8 }},55 },{{{0 ,9 }},207 },{{{
81 ,7 }},15 },{{{0 ,8 }},103 },{{{0 ,8 }},39 },{{{0 ,9 }},175 },{{{0 ,8 }},7 },{{{0 ,
8 }},135 },{{{0 ,8 }},71 },{{{0 ,9 }},239 },{{{80 ,7 }},9 },{{{0 ,8 }},95 },{{{0 ,8 }
},31 },{{{0 ,9 }},159 },{{{84 ,7 }},99 },{{{0 ,8 }},127 },{{{0 ,8 }},63 },{{{0 ,9 }}
,223 },{{{82 ,7 }},27 },{{{0 ,8 }},111 },{{{0 ,8 }},47 },{{{0 ,9 }},191 },{{{0 ,8 }}
,15 },{{{0 ,8 }},143 },{{{0 ,8 }},79 },{{{0 ,9 }},255 }};bb40 bb153 bb2338[]={{
{{80 ,5 }},1 },{{{87 ,5 }},257 },{{{83 ,5 }},17 },{{{91 ,5 }},4097 },{{{81 ,5 }},5 }
,{{{89 ,5 }},1025 },{{{85 ,5 }},65 },{{{93 ,5 }},16385 },{{{80 ,5 }},3 },{{{88 ,5 }
},513 },{{{84 ,5 }},33 },{{{92 ,5 }},8193 },{{{82 ,5 }},9 },{{{90 ,5 }},2049 },{{{
86 ,5 }},129 },{{{192 ,5 }},24577 },{{{80 ,5 }},2 },{{{87 ,5 }},385 },{{{83 ,5 }},
25 },{{{91 ,5 }},6145 },{{{81 ,5 }},7 },{{{89 ,5 }},1537 },{{{85 ,5 }},97 },{{{93 ,
5 }},24577 },{{{80 ,5 }},4 },{{{88 ,5 }},769 },{{{84 ,5 }},49 },{{{92 ,5 }},12289 }
,{{{82 ,5 }},13 },{{{90 ,5 }},3073 },{{{86 ,5 }},193 },{{{192 ,5 }},24577 }};
#endif
bbe bb2025(bb58,bb966,bb1053,bb1036,bb0)bb165*bb58;bb165*bb966;bb153*
 *bb1053;bb153* *bb1036;bb16 bb0;{(bbb)bb0;
#ifdef bb2318
bbm(!bb2409){bbe bb6;bb9 bb20=0 ;bb165*bbn;bb165*bb448;bbm((bbn=(bb165
 * )( * ((bb0)->bb414))((bb0)->bb122,(288 ),(bb12(bb9))))==0 )bb4(-4 );
bbm((bb448=(bb165* )( * ((bb0)->bb414))((bb0)->bb122,(288 ),(bb12(bb9))))==
0 ){( * ((bb0)->bb379))((bb0)->bb122,(bb72)(bbn));bb4(-4 );}bb90(bb6=0 ;
bb6<144 ;bb6++)bbn[bb6]=8 ;bb90(;bb6<256 ;bb6++)bbn[bb6]=9 ;bb90(;bb6<280
;bb6++)bbn[bb6]=7 ;bb90(;bb6<288 ;bb6++)bbn[bb6]=8 ;bb2200=9 ;bb2033(bbn,
288 ,257 ,bb2446,bb2434,&bb2333,&bb2200,bb2510,&bb20,bb448);bb90(bb6=0 ;
bb6<30 ;bb6++)bbn[bb6]=5 ;bb2201=5 ;bb2033(bbn,30 ,0 ,bb2484,bb2485,&
bb2338,&bb2201,bb2510,&bb20,bb448);( * ((bb0)->bb379))((bb0)->bb122,(
bb72)(bb448));( * ((bb0)->bb379))((bb0)->bb122,(bb72)(bbn));bb2409=1 ;
}
#endif
 *bb58=bb2200; *bb966=bb2201; *bb1053=bb2333; *bb1036=bb2338;bb4 0 ;}
