/*
   'src_compress_deflate_trees.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
bb935));bba bbj bb2009{bb329{bb130 bb444;bb130 bb171;}bb294;bb329{
bb130 bb2234;bb130 bb22;}bb52;}bb475;bba bbj bb2346 bb2069;bba bbj
bb1995{bb475*bb1780;bbe bb522;bb2069*bb1736;}bb1765;bba bb130 bb1072;
bba bb1072 bb1390;bba bbt bb1356;bba bbj bb390{bb16 bb15;bbe bb368;
bb34*bb173;bb410 bb2190;bb34*bb1921;bbe bb189;bbe bb1386;bb154 bb1000
;bb154 bb590;bbe bb1948;bb9 bb957;bb9 bb2235;bb9 bb1850;bb34*bb159;
bb410 bb2297;bb1390*bb998;bb1390*bb395;bb9 bb515;bb9 bb1365;bb9 bb2202
;bb9 bb1755;bb9 bb1629;bb8 bb443;bb9 bb970;bb1356 bb2369;bbe bb1993;
bb9 bb187;bb9 bb2088;bb9 bb473;bb9 bb1318;bb9 bb2251;bb9 bb2121;bbe
bb126;bbe bb303;bb9 bb2255;bbe bb1888;bbj bb2009 bb1001[(2 * (256 +1 +29
)+1 )];bbj bb2009 bb1693[2 *30 +1 ];bbj bb2009 bb552[2 *19 +1 ];bbj bb1995
bb1992;bbj bb1995 bb1914;bbj bb1995 bb2126;bb130 bb1229[15 +1 ];bbe
bb542[2 * (256 +1 +29 )+1 ];bbe bb1523;bbe bb1998;bb156 bb1281[2 * (256 +1 +
29 )+1 ];bb1236*bb1743;bb9 bb1159;bb9 bb637;bb521*bb1661;bb410 bb1961;
bb410 bb2185;bb9 bb2300;bbe bb2053;bb130 bb102;bbe bb84;}bb192;bbb
bb2283 bbq((bb192*bbg));bbe bb2467 bbq((bb192*bbg,bbt bb429,bbt bb1162
));bbb bb1637 bbq((bb192*bbg,bb447*bb42,bb410 bb1333,bbe bb1147));bbb
bb2323 bbq((bb192*bbg));bbb bb2218 bbq((bb192*bbg,bb447*bb42,bb410
bb1333,bbe bb1147));bb40 bbh bbe bb2492[29 ]={0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,
2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,0 };bb40 bbh bbe bb2456[30 ]={0 ,0 ,0 ,0 ,1
,1 ,2 ,2 ,3 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,7 ,7 ,8 ,8 ,9 ,9 ,10 ,10 ,11 ,11 ,12 ,12 ,13 ,13 };bb40 bbh
bbe bb2554[19 ]={0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,3 ,7 };bb40 bbh bb156
bb2382[19 ]={16 ,17 ,18 ,0 ,8 ,7 ,9 ,6 ,10 ,5 ,11 ,4 ,12 ,3 ,13 ,2 ,14 ,1 ,15 };bb40 bbh
bb475 bb1766[(256 +1 +29 )+2 ]={{{12 },{8 }},{{140 },{8 }},{{76 },{8 }},{{204 },
{8 }},{{44 },{8 }},{{172 },{8 }},{{108 },{8 }},{{236 },{8 }},{{28 },{8 }},{{156 }
,{8 }},{{92 },{8 }},{{220 },{8 }},{{60 },{8 }},{{188 },{8 }},{{124 },{8 }},{{252
},{8 }},{{2 },{8 }},{{130 },{8 }},{{66 },{8 }},{{194 },{8 }},{{34 },{8 }},{{162 }
,{8 }},{{98 },{8 }},{{226 },{8 }},{{18 },{8 }},{{146 },{8 }},{{82 },{8 }},{{210 }
,{8 }},{{50 },{8 }},{{178 },{8 }},{{114 },{8 }},{{242 },{8 }},{{10 },{8 }},{{138
},{8 }},{{74 },{8 }},{{202 },{8 }},{{42 },{8 }},{{170 },{8 }},{{106 },{8 }},{{
234 },{8 }},{{26 },{8 }},{{154 },{8 }},{{90 },{8 }},{{218 },{8 }},{{58 },{8 }},{{
186 },{8 }},{{122 },{8 }},{{250 },{8 }},{{6 },{8 }},{{134 },{8 }},{{70 },{8 }},{{
198 },{8 }},{{38 },{8 }},{{166 },{8 }},{{102 },{8 }},{{230 },{8 }},{{22 },{8 }},{
{150 },{8 }},{{86 },{8 }},{{214 },{8 }},{{54 },{8 }},{{182 },{8 }},{{118 },{8 }},
{{246 },{8 }},{{14 },{8 }},{{142 },{8 }},{{78 },{8 }},{{206 },{8 }},{{46 },{8 }},
{{174 },{8 }},{{110 },{8 }},{{238 },{8 }},{{30 },{8 }},{{158 },{8 }},{{94 },{8 }}
,{{222 },{8 }},{{62 },{8 }},{{190 },{8 }},{{126 },{8 }},{{254 },{8 }},{{1 },{8 }}
,{{129 },{8 }},{{65 },{8 }},{{193 },{8 }},{{33 },{8 }},{{161 },{8 }},{{97 },{8 }}
,{{225 },{8 }},{{17 },{8 }},{{145 },{8 }},{{81 },{8 }},{{209 },{8 }},{{49 },{8 }}
,{{177 },{8 }},{{113 },{8 }},{{241 },{8 }},{{9 },{8 }},{{137 },{8 }},{{73 },{8 }}
,{{201 },{8 }},{{41 },{8 }},{{169 },{8 }},{{105 },{8 }},{{233 },{8 }},{{25 },{8 }
},{{153 },{8 }},{{89 },{8 }},{{217 },{8 }},{{57 },{8 }},{{185 },{8 }},{{121 },{8
}},{{249 },{8 }},{{5 },{8 }},{{133 },{8 }},{{69 },{8 }},{{197 },{8 }},{{37 },{8 }
},{{165 },{8 }},{{101 },{8 }},{{229 },{8 }},{{21 },{8 }},{{149 },{8 }},{{85 },{8
}},{{213 },{8 }},{{53 },{8 }},{{181 },{8 }},{{117 },{8 }},{{245 },{8 }},{{13 },{
8 }},{{141 },{8 }},{{77 },{8 }},{{205 },{8 }},{{45 },{8 }},{{173 },{8 }},{{109 },
{8 }},{{237 },{8 }},{{29 },{8 }},{{157 },{8 }},{{93 },{8 }},{{221 },{8 }},{{61 },
{8 }},{{189 },{8 }},{{125 },{8 }},{{253 },{8 }},{{19 },{9 }},{{275 },{9 }},{{147
},{9 }},{{403 },{9 }},{{83 },{9 }},{{339 },{9 }},{{211 },{9 }},{{467 },{9 }},{{
51 },{9 }},{{307 },{9 }},{{179 },{9 }},{{435 },{9 }},{{115 },{9 }},{{371 },{9 }},
{{243 },{9 }},{{499 },{9 }},{{11 },{9 }},{{267 },{9 }},{{139 },{9 }},{{395 },{9 }
},{{75 },{9 }},{{331 },{9 }},{{203 },{9 }},{{459 },{9 }},{{43 },{9 }},{{299 },{9
}},{{171 },{9 }},{{427 },{9 }},{{107 },{9 }},{{363 },{9 }},{{235 },{9 }},{{491 }
,{9 }},{{27 },{9 }},{{283 },{9 }},{{155 },{9 }},{{411 },{9 }},{{91 },{9 }},{{347
},{9 }},{{219 },{9 }},{{475 },{9 }},{{59 },{9 }},{{315 },{9 }},{{187 },{9 }},{{
443 },{9 }},{{123 },{9 }},{{379 },{9 }},{{251 },{9 }},{{507 },{9 }},{{7 },{9 }},{
{263 },{9 }},{{135 },{9 }},{{391 },{9 }},{{71 },{9 }},{{327 },{9 }},{{199 },{9 }}
,{{455 },{9 }},{{39 },{9 }},{{295 },{9 }},{{167 },{9 }},{{423 },{9 }},{{103 },{9
}},{{359 },{9 }},{{231 },{9 }},{{487 },{9 }},{{23 },{9 }},{{279 },{9 }},{{151 },
{9 }},{{407 },{9 }},{{87 },{9 }},{{343 },{9 }},{{215 },{9 }},{{471 },{9 }},{{55 }
,{9 }},{{311 },{9 }},{{183 },{9 }},{{439 },{9 }},{{119 },{9 }},{{375 },{9 }},{{
247 },{9 }},{{503 },{9 }},{{15 },{9 }},{{271 },{9 }},{{143 },{9 }},{{399 },{9 }},
{{79 },{9 }},{{335 },{9 }},{{207 },{9 }},{{463 },{9 }},{{47 },{9 }},{{303 },{9 }}
,{{175 },{9 }},{{431 },{9 }},{{111 },{9 }},{{367 },{9 }},{{239 },{9 }},{{495 },{
9 }},{{31 },{9 }},{{287 },{9 }},{{159 },{9 }},{{415 },{9 }},{{95 },{9 }},{{351 },
{9 }},{{223 },{9 }},{{479 },{9 }},{{63 },{9 }},{{319 },{9 }},{{191 },{9 }},{{447
},{9 }},{{127 },{9 }},{{383 },{9 }},{{255 },{9 }},{{511 },{9 }},{{0 },{7 }},{{64
},{7 }},{{32 },{7 }},{{96 },{7 }},{{16 },{7 }},{{80 },{7 }},{{48 },{7 }},{{112 },
{7 }},{{8 },{7 }},{{72 },{7 }},{{40 },{7 }},{{104 },{7 }},{{24 },{7 }},{{88 },{7 }
},{{56 },{7 }},{{120 },{7 }},{{4 },{7 }},{{68 },{7 }},{{36 },{7 }},{{100 },{7 }},
{{20 },{7 }},{{84 },{7 }},{{52 },{7 }},{{116 },{7 }},{{3 },{8 }},{{131 },{8 }},{{
67 },{8 }},{{195 },{8 }},{{35 },{8 }},{{163 },{8 }},{{99 },{8 }},{{227 },{8 }}};
bb40 bbh bb475 bb2304[30 ]={{{0 },{5 }},{{16 },{5 }},{{8 },{5 }},{{24 },{5 }},
{{4 },{5 }},{{20 },{5 }},{{12 },{5 }},{{28 },{5 }},{{2 },{5 }},{{18 },{5 }},{{10 }
,{5 }},{{26 },{5 }},{{6 },{5 }},{{22 },{5 }},{{14 },{5 }},{{30 },{5 }},{{1 },{5 }}
,{{17 },{5 }},{{9 },{5 }},{{25 },{5 }},{{5 },{5 }},{{21 },{5 }},{{13 },{5 }},{{29
},{5 }},{{3 },{5 }},{{19 },{5 }},{{11 },{5 }},{{27 },{5 }},{{7 },{5 }},{{23 },{5 }
}};bb40 bbh bb156 bb1764[512 ]={0 ,1 ,2 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,6 ,6 ,7 ,7 ,7 ,7 ,8 ,8 ,8 ,
8 ,8 ,8 ,8 ,8 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,
10 ,10 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,12 ,12 ,12 ,12 ,12 ,
12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,
12 ,12 ,12 ,12 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,
13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,0 ,0 ,16 ,17 ,18 ,18 ,19 ,19 ,20 ,20 ,20 ,20 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22
,22 ,22 ,22 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24
,24 ,24 ,24 ,24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27
,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 };bb40 bbh bb156 bb2116[258 -3 +1 ]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,8 ,
9 ,9 ,10 ,10 ,11 ,11 ,12 ,12 ,12 ,12 ,13 ,13 ,13 ,13 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15 ,16 ,16
,16 ,16 ,16 ,16 ,16 ,16 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,19
,19 ,19 ,19 ,19 ,19 ,19 ,19 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20
,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22 ,22 ,22
,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23
,23 ,23 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24
,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25
,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27
,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,28 };bb40 bbh bbe bb2407[29
]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,10 ,12 ,14 ,16 ,20 ,24 ,28 ,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112 ,
128 ,160 ,192 ,224 ,0 };bb40 bbh bbe bb2493[30 ]={0 ,1 ,2 ,3 ,4 ,6 ,8 ,12 ,16 ,24 ,32
,48 ,64 ,96 ,128 ,192 ,256 ,384 ,512 ,768 ,1024 ,1536 ,2048 ,3072 ,4096 ,6144 ,8192 ,
12288 ,16384 ,24576 };bbj bb2346{bbh bb475*bb2418;bbh bb1171*bb2549;bbe
bb2532;bbe bb2248;bbe bb1997;};bb40 bb2069 bb2568={bb1766,bb2492,256 +
1 ,(256 +1 +29 ),15 };bb40 bb2069 bb2631={bb2304,bb2456,0 ,30 ,15 };bb40
bb2069 bb2573={(bbh bb475* )0 ,bb2554,0 ,19 ,7 };bb40 bbb bb2317 bbq((
bb192*bbg));bb40 bbb bb2241 bbq((bb192*bbg,bb475*bb309,bbe bb6));bb40
bbb bb2438 bbq((bb192*bbg,bb1765*bb1111));bb40 bbb bb2465 bbq((bb475*
bb309,bbe bb522,bb521*bb1229));bb40 bbb bb2259 bbq((bb192*bbg,bb1765*
bb1111));bb40 bbb bb2378 bbq((bb192*bbg,bb475*bb309,bbe bb522));bb40
bbb bb2303 bbq((bb192*bbg,bb475*bb309,bbe bb522));bb40 bbe bb2463 bbq
((bb192*bbg));bb40 bbb bb2476 bbq((bb192*bbg,bbe bb2117,bbe bb2160,
bbe bb2109));bb40 bbb bb2341 bbq((bb192*bbg,bb475*bb1087,bb475*bb1778
));bb40 bbb bb2523 bbq((bb192*bbg));bb40 bbt bb2447 bbq((bbt bb1362,
bbe bb483));bb40 bbb bb2329 bbq((bb192*bbg));bb40 bbb bb2375 bbq((
bb192*bbg));bb40 bbb bb2386 bbq((bb192*bbg,bb447*bb42,bbt bb22,bbe
bb1017));bbb bb2283(bbg)bb192*bbg;{bbg->bb1992.bb1780=bbg->bb1001;bbg
->bb1992.bb1736=&bb2568;bbg->bb1914.bb1780=bbg->bb1693;bbg->bb1914.
bb1736=&bb2631;bbg->bb2126.bb1780=bbg->bb552;bbg->bb2126.bb1736=&
bb2573;bbg->bb102=0 ;bbg->bb84=0 ;bbg->bb2053=8 ;bb2317(bbg);}bb40 bbb
bb2317(bbg)bb192*bbg;{bbe bb11;bb90(bb11=0 ;bb11<(256 +1 +29 );bb11++)bbg
->bb1001[bb11].bb294.bb444=0 ;bb90(bb11=0 ;bb11<30 ;bb11++)bbg->bb1693[
bb11].bb294.bb444=0 ;bb90(bb11=0 ;bb11<19 ;bb11++)bbg->bb552[bb11].bb294
.bb444=0 ;bbg->bb1001[256 ].bb294.bb444=1 ;bbg->bb1961=bbg->bb2185=0L ;
bbg->bb637=bbg->bb2300=0 ;}bb40 bbb bb2241(bbg,bb309,bb6)bb192*bbg;
bb475*bb309;bbe bb6;{bbe bb448=bbg->bb542[bb6];bbe bb77=bb6<<1 ;bb109(
bb77<=bbg->bb1523){bbm(bb77<bbg->bb1523&&(bb309[bbg->bb542[bb77+1 ]].
bb294.bb444<bb309[bbg->bb542[bb77]].bb294.bb444||(bb309[bbg->bb542[
bb77+1 ]].bb294.bb444==bb309[bbg->bb542[bb77]].bb294.bb444&&bbg->
bb1281[bbg->bb542[bb77+1 ]]<=bbg->bb1281[bbg->bb542[bb77]]))){bb77++;}
bbm((bb309[bb448].bb294.bb444<bb309[bbg->bb542[bb77]].bb294.bb444||(
bb309[bb448].bb294.bb444==bb309[bbg->bb542[bb77]].bb294.bb444&&bbg->
bb1281[bb448]<=bbg->bb1281[bbg->bb542[bb77]])))bb21;bbg->bb542[bb6]=
bbg->bb542[bb77];bb6=bb77;bb77<<=1 ;}bbg->bb542[bb6]=bb448;}bb40 bbb
bb2438(bbg,bb1111)bb192*bbg;bb1765*bb1111;{bb475*bb309=bb1111->bb1780
;bbe bb522=bb1111->bb522;bbh bb475*bb2183=bb1111->bb1736->bb2418;bbh
bb1171*bb1842=bb1111->bb1736->bb2549;bbe bb625=bb1111->bb1736->bb2532
;bbe bb1997=bb1111->bb1736->bb1997;bbe bb44;bbe bb11,bb82;bbe bb544;
bbe bb2217;bb130 bb20;bbe bb2212=0 ;bb90(bb544=0 ;bb544<=15 ;bb544++)bbg
->bb1229[bb544]=0 ;bb309[bbg->bb542[bbg->bb1998]].bb52.bb22=0 ;bb90(
bb44=bbg->bb1998+1 ;bb44<(2 * (256 +1 +29 )+1 );bb44++){bb11=bbg->bb542[
bb44];bb544=bb309[bb309[bb11].bb52.bb2234].bb52.bb22+1 ;bbm(bb544>
bb1997)bb544=bb1997,bb2212++;bb309[bb11].bb52.bb22=(bb130)bb544;bbm(
bb11>bb522)bb1698;bbg->bb1229[bb544]++;bb2217=0 ;bbm(bb11>=bb625)bb2217
=bb1842[bb11-bb625];bb20=bb309[bb11].bb294.bb444;bbg->bb1961+=(bb410)bb20
 * (bb544+bb2217);bbm(bb2183)bbg->bb2185+=(bb410)bb20* (bb2183[bb11].
bb52.bb22+bb2217);}bbm(bb2212==0 )bb4;;bb599{bb544=bb1997-1 ;bb109(bbg
->bb1229[bb544]==0 )bb544--;bbg->bb1229[bb544]--;bbg->bb1229[bb544+1 ]
+=2 ;bbg->bb1229[bb1997]--;bb2212-=2 ;}bb109(bb2212>0 );bb90(bb544=
bb1997;bb544!=0 ;bb544--){bb11=bbg->bb1229[bb544];bb109(bb11!=0 ){bb82=
bbg->bb542[--bb44];bbm(bb82>bb522)bb1698;bbm(bb309[bb82].bb52.bb22!=(
bbt)bb544){;bbg->bb1961+=((bb8)bb544-(bb8)bb309[bb82].bb52.bb22) * (
bb8)bb309[bb82].bb294.bb444;bb309[bb82].bb52.bb22=(bb130)bb544;}bb11
--;}}}bb40 bbb bb2465(bb309,bb522,bb1229)bb475*bb309;bbe bb522;bb521*
bb1229;{bb130 bb2482[15 +1 ];bb130 bb171=0 ;bbe bb544;bbe bb11;bb90(
bb544=1 ;bb544<=15 ;bb544++){bb2482[bb544]=bb171=(bb171+bb1229[bb544-1 ]
)<<1 ;};;bb90(bb11=0 ;bb11<=bb522;bb11++){bbe bb22=bb309[bb11].bb52.
bb22;bbm(bb22==0 )bb1698;bb309[bb11].bb294.bb171=bb2447(bb2482[bb22]++
,bb22);;}}bb40 bbb bb2259(bbg,bb1111)bb192*bbg;bb1765*bb1111;{bb475*
bb309=bb1111->bb1780;bbh bb475*bb2183=bb1111->bb1736->bb2418;bbe
bb2248=bb1111->bb1736->bb2248;bbe bb11,bb82;bbe bb522=-1 ;bbe bb1813;
bbg->bb1523=0 ,bbg->bb1998=(2 * (256 +1 +29 )+1 );bb90(bb11=0 ;bb11<bb2248;
bb11++){bbm(bb309[bb11].bb294.bb444!=0 ){bbg->bb542[++(bbg->bb1523)]=
bb522=bb11;bbg->bb1281[bb11]=0 ;}bb50{bb309[bb11].bb52.bb22=0 ;}}bb109(
bbg->bb1523<2 ){bb1813=bbg->bb542[++(bbg->bb1523)]=(bb522<2 ?++bb522:0 );
bb309[bb1813].bb294.bb444=1 ;bbg->bb1281[bb1813]=0 ;bbg->bb1961--;bbm(
bb2183)bbg->bb2185-=bb2183[bb1813].bb52.bb22;}bb1111->bb522=bb522;
bb90(bb11=bbg->bb1523/2 ;bb11>=1 ;bb11--)bb2241(bbg,bb309,bb11);bb1813=
bb2248;bb599{{bb11=bbg->bb542[1 ];bbg->bb542[1 ]=bbg->bb542[bbg->bb1523
--];bb2241(bbg,bb309,1 );};bb82=bbg->bb542[1 ];bbg->bb542[--(bbg->
bb1998)]=bb11;bbg->bb542[--(bbg->bb1998)]=bb82;bb309[bb1813].bb294.
bb444=bb309[bb11].bb294.bb444+bb309[bb82].bb294.bb444;bbg->bb1281[
bb1813]=(bb156)((bbg->bb1281[bb11]>=bbg->bb1281[bb82]?bbg->bb1281[
bb11]:bbg->bb1281[bb82])+1 );bb309[bb11].bb52.bb2234=bb309[bb82].bb52.
bb2234=(bb130)bb1813;bbg->bb542[1 ]=bb1813++;bb2241(bbg,bb309,1 );}
bb109(bbg->bb1523>=2 );bbg->bb542[--(bbg->bb1998)]=bbg->bb542[1 ];
bb2438(bbg,(bb1765* )bb1111);bb2465((bb475* )bb309,bb522,bbg->bb1229);
}bb40 bbb bb2378(bbg,bb309,bb522)bb192*bbg;bb475*bb309;bbe bb522;{bbe
bb11;bbe bb2152=-1 ;bbe bb916;bbe bb1251=bb309[0 ].bb52.bb22;bbe bb1002
=0 ;bbe bb1376=7 ;bbe bb1343=4 ;bbm(bb1251==0 )bb1376=138 ,bb1343=3 ;bb309[
bb522+1 ].bb52.bb22=(bb130)0xffff ;bb90(bb11=0 ;bb11<=bb522;bb11++){
bb916=bb1251;bb1251=bb309[bb11+1 ].bb52.bb22;bbm(++bb1002<bb1376&&
bb916==bb1251){bb1698;}bb50 bbm(bb1002<bb1343){bbg->bb552[bb916].
bb294.bb444+=bb1002;}bb50 bbm(bb916!=0 ){bbm(bb916!=bb2152)bbg->bb552[
bb916].bb294.bb444++;bbg->bb552[16 ].bb294.bb444++;}bb50 bbm(bb1002<=
10 ){bbg->bb552[17 ].bb294.bb444++;}bb50{bbg->bb552[18 ].bb294.bb444++;}
bb1002=0 ;bb2152=bb916;bbm(bb1251==0 ){bb1376=138 ,bb1343=3 ;}bb50 bbm(
bb916==bb1251){bb1376=6 ,bb1343=3 ;}bb50{bb1376=7 ,bb1343=4 ;}}}bb40 bbb
bb2303(bbg,bb309,bb522)bb192*bbg;bb475*bb309;bbe bb522;{bbe bb11;bbe
bb2152=-1 ;bbe bb916;bbe bb1251=bb309[0 ].bb52.bb22;bbe bb1002=0 ;bbe
bb1376=7 ;bbe bb1343=4 ;bbm(bb1251==0 )bb1376=138 ,bb1343=3 ;bb90(bb11=0 ;
bb11<=bb522;bb11++){bb916=bb1251;bb1251=bb309[bb11+1 ].bb52.bb22;bbm(
++bb1002<bb1376&&bb916==bb1251){bb1698;}bb50 bbm(bb1002<bb1343){bb599
{{bbe bb22=bbg->bb552[bb916].bb52.bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12(
bbl))-bb22){bbe bb170=bbg->bb552[bb916].bb294.bb171;bbg->bb102|=(
bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bbg->bb552[bb916].bb294
.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};}bb109(--bb1002!=0 );}bb50 bbm(
bb916!=0 ){bbm(bb916!=bb2152){{bbe bb22=bbg->bb552[bb916].bb52.bb22;
bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb552[bb916].
bb294.bb171;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]
=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((
bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(
bbg->bb552[bb916].bb294.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};bb1002--
;};{bbe bb22=bbg->bb552[16 ].bb52.bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12(
bbl))-bb22){bbe bb170=bbg->bb552[16 ].bb294.bb171;bbg->bb102|=(bb170<<
bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};
{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->
bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-
(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bbg->bb552[16 ].bb294.bb171)<<bbg->
bb84;bbg->bb84+=bb22;}};{bbe bb22=2 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb1002-3 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[
bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++
]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((
bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{
bbg->bb102|=(bb1002-3 )<<bbg->bb84;bbg->bb84+=bb22;}};}bb50 bbm(bb1002
<=10 ){{bbe bb22=bbg->bb552[17 ].bb52.bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12
(bbl))-bb22){bbe bb170=bbg->bb552[17 ].bb294.bb171;bbg->bb102|=(bb170
<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));
};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg
->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=
bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bbg->bb552[17 ].bb294.bb171)<<
bbg->bb84;bbg->bb84+=bb22;}};{bbe bb22=3 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12
(bbl))-bb22){bbe bb170=bb1002-3 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg
->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg
->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}
bb50{bbg->bb102|=(bb1002-3 )<<bbg->bb84;bbg->bb84+=bb22;}};}bb50{{bbe
bb22=bbg->bb552[18 ].bb52.bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22
){bbe bb170=bbg->bb552[18 ].bb294.bb171;bbg->bb102|=(bb170<<bbg->bb84);
{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173
[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(
bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *
bb12(bbl));}bb50{bbg->bb102|=(bbg->bb552[18 ].bb294.bb171)<<bbg->bb84;
bbg->bb84+=bb22;}};{bbe bb22=7 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb1002-11 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173
[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189
++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((
bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{
bbg->bb102|=(bb1002-11 )<<bbg->bb84;bbg->bb84+=bb22;}};}bb1002=0 ;
bb2152=bb916;bbm(bb1251==0 ){bb1376=138 ,bb1343=3 ;}bb50 bbm(bb916==
bb1251){bb1376=6 ,bb1343=3 ;}bb50{bb1376=7 ,bb1343=4 ;}}}bb40 bbe bb2463(
bbg)bb192*bbg;{bbe bb1818;bb2378(bbg,(bb475* )bbg->bb1001,bbg->bb1992
.bb522);bb2378(bbg,(bb475* )bbg->bb1693,bbg->bb1914.bb522);bb2259(bbg
,(bb1765* )(&(bbg->bb2126)));bb90(bb1818=19 -1 ;bb1818>=3 ;bb1818--){bbm
(bbg->bb552[bb2382[bb1818]].bb52.bb22!=0 )bb21;}bbg->bb1961+=3 * (
bb1818+1 )+5 +5 +4 ;;bb4 bb1818;}bb40 bbb bb2476(bbg,bb2117,bb2160,bb2109
)bb192*bbg;bbe bb2117,bb2160,bb2109;{bbe bb2187;;;;{bbe bb22=5 ;bbm(
bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb2117-257 ;bbg->bb102
|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb2117-257 )<<bbg->bb84;
bbg->bb84+=bb22;}};{bbe bb22=5 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb2160-1 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[
bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++
]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((
bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{
bbg->bb102|=(bb2160-1 )<<bbg->bb84;bbg->bb84+=bb22;}};{bbe bb22=4 ;bbm(
bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb2109-4 ;bbg->bb102|=(
bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb2109-4 )<<bbg->bb84;
bbg->bb84+=bb22;}};bb90(bb2187=0 ;bb2187<bb2109;bb2187++){;{bbe bb22=3
;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb552[bb2382
[bb2187]].bb52.bb22;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->
bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((
bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8
 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->
bb102|=(bbg->bb552[bb2382[bb2187]].bb52.bb22)<<bbg->bb84;bbg->bb84+=
bb22;}};};bb2303(bbg,(bb475* )bbg->bb1001,bb2117-1 );;bb2303(bbg,(
bb475* )bbg->bb1693,bb2160-1 );;}bbb bb2218(bbg,bb42,bb1333,bb1147)bb192
 *bbg;bb447*bb42;bb410 bb1333;bbe bb1147;{{bbe bb22=3 ;bbm(bbg->bb84>(
bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=(0 <<1 )+bb1147;bbg->bb102|=(bb170
<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));
};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg
->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=
bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=((0 <<1 )+bb1147)<<bbg->bb84;bbg
->bb84+=bb22;}};bb2386(bbg,bb42,(bbt)bb1333,1 );}bbb bb2323(bbg)bb192*
bbg;{{bbe bb22=3 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=1
<<1 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156
)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(
bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-
bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(1 <<1 )<<
bbg->bb84;bbg->bb84+=bb22;}};{bbe bb22=bb1766[256 ].bb52.bb22;bbm(bbg
->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1766[256 ].bb294.bb171;
bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((
bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->
bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->
bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb1766[256 ].
bb294.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};bb2375(bbg);bbm(1 +bbg->
bb2053+10 -bbg->bb84<9 ){{bbe bb22=3 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=1 <<1 ;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg
->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=(
(bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(
8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->
bb102|=(1 <<1 )<<bbg->bb84;bbg->bb84+=bb22;}};{bbe bb22=bb1766[256 ].
bb52.bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1766[
256 ].bb294.bb171;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->
bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((
bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8
 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->
bb102|=(bb1766[256 ].bb294.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};bb2375
(bbg);}bbg->bb2053=7 ;}bbb bb1637(bbg,bb42,bb1333,bb1147)bb192*bbg;
bb447*bb42;bb410 bb1333;bbe bb1147;{bb410 bb2076,bb2139;bbe bb1818=0 ;
bbm(bbg->bb126>0 ){bbm(bbg->bb1000==2 )bb2523(bbg);bb2259(bbg,(bb1765* )(
&(bbg->bb1992)));;bb2259(bbg,(bb1765* )(&(bbg->bb1914)));;bb1818=
bb2463(bbg);bb2076=(bbg->bb1961+3 +7 )>>3 ;bb2139=(bbg->bb2185+3 +7 )>>3 ;;
bbm(bb2139<=bb2076)bb2076=bb2139;}bb50{;bb2076=bb2139=bb1333+5 ;}bbm(
bb1333+4 <=bb2076&&bb42!=(bbl* )0 ){bb2218(bbg,bb42,bb1333,bb1147);}
bb50 bbm(bb2139==bb2076){{bbe bb22=3 ;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl
))-bb22){bbe bb170=(1 <<1 )+bb1147;bbg->bb102|=(bb170<<bbg->bb84);{{bbg
->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg
->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}
bb50{bbg->bb102|=((1 <<1 )+bb1147)<<bbg->bb84;bbg->bb84+=bb22;}};bb2341
(bbg,(bb475* )bb1766,(bb475* )bb2304);}bb50{{bbe bb22=3 ;bbm(bbg->bb84
>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=(2 <<1 )+bb1147;bbg->bb102|=(
bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=((2 <<1 )+bb1147)<<bbg->
bb84;bbg->bb84+=bb22;}};bb2476(bbg,bbg->bb1992.bb522+1 ,bbg->bb1914.
bb522+1 ,bb1818+1 );bb2341(bbg,(bb475* )bbg->bb1001,(bb475* )bbg->
bb1693);};bb2317(bbg);bbm(bb1147){bb2329(bbg);};}bbe bb2467(bbg,bb429
,bb1162)bb192*bbg;bbt bb429;bbt bb1162;{bbg->bb1661[bbg->bb637]=(
bb130)bb429;bbg->bb1743[bbg->bb637++]=(bb156)bb1162;bbm(bb429==0 ){bbg
->bb1001[bb1162].bb294.bb444++;}bb50{bbg->bb2300++;bb429--;;bbg->
bb1001[bb2116[bb1162]+256 +1 ].bb294.bb444++;bbg->bb1693[((bb429)<256 ?
bb1764[bb429]:bb1764[256 +((bb429)>>7 )])].bb294.bb444++;}bb4(bbg->
bb637==bbg->bb1159-1 );}bb40 bbb bb2341(bbg,bb1087,bb1778)bb192*bbg;
bb475*bb1087;bb475*bb1778;{bbt bb429;bbe bb1162;bbt bb2358=0 ;bbt bb171
;bbe bb1842;bbm(bbg->bb637!=0 )bb599{bb429=bbg->bb1661[bb2358];bb1162=
bbg->bb1743[bb2358++];bbm(bb429==0 ){{bbe bb22=bb1087[bb1162].bb52.
bb22;bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1087[bb1162
].bb294.bb171;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189
++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)(
(bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12
(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(
bb1087[bb1162].bb294.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};;}bb50{
bb171=bb2116[bb1162];{bbe bb22=bb1087[bb171+256 +1 ].bb52.bb22;bbm(bbg
->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1087[bb171+256 +1 ].bb294
.bb171;bbg->bb102|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((
bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((
bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(
bb1087[bb171+256 +1 ].bb294.bb171)<<bbg->bb84;bbg->bb84+=bb22;}};bb1842
=bb2492[bb171];bbm(bb1842!=0 ){bb1162-=bb2407[bb171];{bbe bb22=bb1842;
bbm(bbg->bb84>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1162;bbg->bb102
|=(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb1162)<<bbg->bb84;bbg
->bb84+=bb22;}};}bb429--;bb171=((bb429)<256 ?bb1764[bb429]:bb1764[256 +
((bb429)>>7 )]);;{bbe bb22=bb1778[bb171].bb52.bb22;bbm(bbg->bb84>(bbe)(
8 *2 *bb12(bbl))-bb22){bbe bb170=bb1778[bb171].bb294.bb171;bbg->bb102|=
(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb1778[bb171].bb294.
bb171)<<bbg->bb84;bbg->bb84+=bb22;}};bb1842=bb2456[bb171];bbm(bb1842
!=0 ){bb429-=bb2493[bb171];{bbe bb22=bb1842;bbm(bbg->bb84>(bbe)(8 *2 *
bb12(bbl))-bb22){bbe bb170=bb429;bbg->bb102|=(bb170<<bbg->bb84);{{bbg
->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg
->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=(bb130)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->bb84+=bb22-(8 *2 *bb12(bbl));}
bb50{bbg->bb102|=(bb429)<<bbg->bb84;bbg->bb84+=bb22;}};}};}bb109(
bb2358<bbg->bb637);{bbe bb22=bb1087[256 ].bb52.bb22;bbm(bbg->bb84>(bbe
)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1087[256 ].bb294.bb171;bbg->bb102|=
(bb170<<bbg->bb84);{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&
0xff ));};{bbg->bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));
};};bbg->bb102=(bb130)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb84);bbg->
bb84+=bb22-(8 *2 *bb12(bbl));}bb50{bbg->bb102|=(bb1087[256 ].bb294.bb171
)<<bbg->bb84;bbg->bb84+=bb22;}};bbg->bb2053=bb1087[256 ].bb52.bb22;}
bb40 bbb bb2523(bbg)bb192*bbg;{bbe bb11=0 ;bbt bb2400=0 ;bbt bb2340=0 ;
bb109(bb11<7 )bb2340+=bbg->bb1001[bb11++].bb294.bb444;bb109(bb11<128 )bb2400
+=bbg->bb1001[bb11++].bb294.bb444;bb109(bb11<256 )bb2340+=bbg->bb1001[
bb11++].bb294.bb444;bbg->bb1000=(bb154)(bb2340>(bb2400>>2 )?0 :1 );}bb40
bbt bb2447(bb171,bb22)bbt bb171;bbe bb22;{bb943 bbt bb2376=0 ;bb599{
bb2376|=bb171&1 ;bb171>>=1 ,bb2376<<=1 ;}bb109(--bb22>0 );bb4 bb2376>>1 ;}
bb40 bbb bb2375(bbg)bb192*bbg;{bbm(bbg->bb84==16 ){{{bbg->bb173[bbg->
bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->bb173[bbg->bb189++]=((
bb156)((bb130)(bbg->bb102)>>8 ));};};bbg->bb102=0 ;bbg->bb84=0 ;}bb50 bbm
(bbg->bb84>=8 ){{bbg->bb173[bbg->bb189++]=((bb154)bbg->bb102);};bbg->
bb102>>=8 ;bbg->bb84-=8 ;}}bb40 bbb bb2329(bbg)bb192*bbg;{bbm(bbg->bb84
>8 ){{{bbg->bb173[bbg->bb189++]=((bb156)((bbg->bb102)&0xff ));};{bbg->
bb173[bbg->bb189++]=((bb156)((bb130)(bbg->bb102)>>8 ));};};}bb50 bbm(
bbg->bb84>0 ){{bbg->bb173[bbg->bb189++]=((bb154)bbg->bb102);};}bbg->
bb102=0 ;bbg->bb84=0 ;}bb40 bbb bb2386(bbg,bb42,bb22,bb1017)bb192*bbg;
bb447*bb42;bbt bb22;bbe bb1017;{bb2329(bbg);bbg->bb2053=8 ;bbm(bb1017){
{{bbg->bb173[bbg->bb189++]=((bb156)(((bb130)bb22)&0xff ));};{bbg->
bb173[bbg->bb189++]=((bb156)((bb130)((bb130)bb22)>>8 ));};};{{bbg->
bb173[bbg->bb189++]=((bb156)(((bb130)~bb22)&0xff ));};{bbg->bb173[bbg
->bb189++]=((bb156)((bb130)((bb130)~bb22)>>8 ));};};}bb109(bb22--){{
bbg->bb173[bbg->bb189++]=( *bb42++);};}}
