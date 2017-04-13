/*
   'src_compress_LZS_lzsc.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
bba bbj bb1915 bb1915;bba bbj bb1915*bb495;bbd bb2269(bbb);bbb bb2211
(bb495 bb2,bbb*bb1354);bbk bb2249(bb495 bb2,bbf* *bb1770,bbf* *bb1835
,bbd*bb938,bbd*bb640,bbb*bb1354,bbk bb387,bbk bb2233);bbk bb2365(
bb495 bb2,bbf* *bb1770,bbf* *bb1835,bbd*bb938,bbd*bb640,bbb*bb1354,
bbk bb387);
#define bb200 1
#define bb202 0
#define bb1292 bb200
#define bb2452 bb202
bba bbj{bbf*bb1789;bbd bb1021;bbf*bb1774;bbd bb633;bbd bb1314;bbd
bb1776;bbk bb2113;bbf bb1966;bbf bb2690;bbk bb1618;bbd bb563;bbd
bb1890;bbk bb1297;bbd*bb2339;bbd bb2362;bbk bb1364;bbk bb2319;bbk
bb2115;bbk bb2165;bbk bb2123;bbk bb2182;bbk bb1055;bb125 bb1094;bbf
bb1728;bbf bb2689;bb125 bb2000;bbk bb2059;bbk bb1868;}bb2587;bba bbj{
bbf*bb1789;bbd bb1021;bbf*bb1774;bbd bb633;bbk bb2157;bbk bb1795;bbk
bb1205;bbk bb2650;bb125 bb1190;bbk bb1186;bbk bb1054;bb125 bb1082;
bb125 bb97;bbd bb483;bbk bb2030;}bb2611;bba bbj{bbf bb1893[2048 ];bbk
bb1927[2048 ];bbd bb2150[4096 ];bb2587 bb46;bbf bb2236[2048 ];bb2611 bb86
;bbf bb1189[64 ];}bb2110;bbj bb1915{bb2110*bb468;bb2110 bb14;bbk bb2179
;bbk bb1781;};bba bbj bb2662{bbk bb2384;bb125 bb48;}bb2528;bb40 bb2528
bb2228[24 ]={{0x0 ,0 },{0x0 ,0 },{0x0 ,2 },{0x1 ,2 },{0x2 ,2 },{0xC ,4 },{0xD ,4 },{
0xE ,4 },{0xF0 ,8 },{0xF1 ,8 },{0xF2 ,8 },{0xF3 ,8 },{0xF4 ,8 },{0xF5 ,8 },{0xF6 ,8 }
,{0xF7 ,8 },{0xF8 ,8 },{0xF9 ,8 },{0xFA ,8 },{0xFB ,8 },{0xFC ,8 },{0xFD ,8 },{0xFE
,8 }};bb40 bbk bb2557[5 ][3 ]={{1 ,1 ,1 },{1 ,1 ,1 },{0 ,0 ,1 },{0 ,0 ,1 },{0 ,1 ,1 }};
bb13{bb2301,bb2640,bb2566,bb2550,bb2466};bb13{bb2149,bb2421,bb2458,
bb2424,bb2519,bb2518,bb2501,bb2310,bb2439};bb40 bbb bb2289(bb495 bb2);
bb40 bbb bb2449(bb495 bb2);bb40 bbb bb1135(bb495 bb2);bb40 bbb bb2454
(bb495 bb2);bb40 bbb bb2210(bbd*bb2486,bbd bb2396);bb40 bbb bb2143(
bb495 bb2,bbk bb2229,bbd bb2166);bb40 bbb bb2197(bb495 bb2);bb40 bbk
bb2353(bb495 bb2);bb40 bbk bb2017(bb495 bb2);bb40 bbb bb2137(bb495 bb2
,bbf bbn);bbd bb2269(bbb){bb4 bb12(bb1915);}bb40 bbb bb2289(bb495 bb2
){bb2->bb14.bb46.bb2000=8 ;bb2->bb14.bb46.bb1728=0 ;bb2->bb14.bb46.
bb1364=bb2->bb14.bb46.bb1618=0 ;bb2->bb14.bb46.bb563=bb2->bb14.bb46.
bb1890=0 ;bb2->bb14.bb46.bb2182=bb2->bb14.bb46.bb1868=0 ;bb4;}bbb bb2211
(bb495 bb2,bbb*bb1354){bb2->bb468=(bb2110* )bb1354;bb2->bb14.bb46=bb2
->bb468->bb46;bb2->bb14.bb46.bb1314=0xFFFFFFFFL ;bb2->bb14.bb46.bb1776
=bb2->bb14.bb46.bb1314-1 ;bb2->bb14.bb46.bb2059=0 ;bb2289(bb2);bb2210(
bb2->bb468->bb2150,0xC0000000L );bb2->bb468->bb46=bb2->bb14.bb46;bb2->
bb14.bb86=bb2->bb468->bb86;bb2->bb14.bb86.bb2157=0 ;bb2197(bb2);bb2->
bb468->bb86=bb2->bb14.bb86;bb4;}bb40 bbb bb2210(bbd*bb2486,bbd bb2396
){bbk bbz;bb90(bbz=0 ;bbz<4096 ;bbz++) *bb2486++=bb2396;bb4;}bb40 bbb
bb2449(bb495 bb2){bb943 bbk bbz;bbd*bb2204;bbd bb2298;bb2298=
0xC0000000L ;bbm(bb2->bb14.bb46.bb1314!=0 )bb2298=0x40000000L ;bb2204=
bb2->bb468->bb2150;bb90(bbz=0 ;bbz<4096 ;bbz++,bb2204++)bbm(bb2->bb14.
bb46.bb1314- *bb2204>2048 -2 ) *bb2204=bb2298;bb4;}bb40 bbb bb1135(
bb495 bb2){bb943 bb125 bb2028;bbm(bb2->bb14.bb46.bb633==0 )bb2->bb14.
bb46.bb2059=1 ;bbm((bb2028=bb2->bb14.bb46.bb1094-bb2->bb14.bb46.bb2000
)>0 ){bb2->bb14.bb46.bb1728|=(bb2->bb14.bb46.bb1055>>bb2028);bb2->bb14
.bb46.bb2000=8 ; *bb2->bb14.bb46.bb1774++=bb2->bb14.bb46.bb1728;--bb2
->bb14.bb46.bb633;bb2->bb14.bb46.bb1728=0 ;bb2->bb14.bb46.bb1055&=((1
<<bb2028)-1 );bb2->bb14.bb46.bb1094=bb2028;bb1135(bb2);}bb50 bbm(
bb2028<0 ){bb2->bb14.bb46.bb1728|=(bb2->bb14.bb46.bb1055<<-bb2028);bb2
->bb14.bb46.bb2000-=bb2->bb14.bb46.bb1094;}bb50{bb2->bb14.bb46.bb1728
|=bb2->bb14.bb46.bb1055;bb2->bb14.bb46.bb2000=8 ; *bb2->bb14.bb46.
bb1774++=bb2->bb14.bb46.bb1728;--bb2->bb14.bb46.bb633;bb2->bb14.bb46.
bb1728=0 ;}bb4;}bb40 bbb bb2454(bb495 bb2){bb2->bb14.bb46.bb1055=(
0x180 );bb2->bb14.bb46.bb1094=(9 );bb1135(bb2);bbm(bb2->bb14.bb46.
bb2000!=8 ){ *bb2->bb14.bb46.bb1774++=bb2->bb14.bb46.bb1728;--bb2->
bb14.bb46.bb633;bbm(bb2->bb14.bb46.bb633==0 ){bb2->bb14.bb46.bb2059=1 ;
}}bb2->bb14.bb46.bb2000=8 ;bb2->bb14.bb46.bb1728=0 ;bb4;}bb40 bbb bb2143
(bb495 bb2,bbk bb2229,bbd bb2166){bbm(bb2->bb14.bb46.bb1868==0 ){bbm(
bb2229<128 ){bb2->bb14.bb46.bb1055=(0x180 |bb2229);bb2->bb14.bb46.
bb1094=(9 );bb1135(bb2);}bb50{bb2->bb14.bb46.bb1055=(0x1000 |bb2229);
bb2->bb14.bb46.bb1094=(13 );bb1135(bb2);}}bbm(bb2166>=23 ){bb2->bb14.
bb46.bb1055=(((1 <<4 )-1 ));bb2->bb14.bb46.bb1094=(4 );bb1135(bb2);bb2166
-=((1 <<4 )-1 );bb2->bb14.bb46.bb1890+=((1 <<4 )-1 );bb2->bb14.bb46.bb1868=
1 ;}bb50{bb2->bb14.bb46.bb1055=(bb2228[(bbk)bb2166].bb2384);bb2->bb14.
bb46.bb1094=(bb2228[(bbk)bb2166].bb48);bb1135(bb2);bb2->bb14.bb46.
bb1868=0 ;bb2->bb14.bb46.bb563=0 ;bb2->bb14.bb46.bb1890=0 ;bb2->bb1781=
bb2466;}bb4;}bb40 bbb bb2267(bb495 bb2,bbk bb387){bbm(bb2->bb14.bb46.
bb2182==1 ){bbm(bb2->bb14.bb46.bb563==0 ){bb2->bb14.bb46.bb1055=((bb2->
bb14.bb46.bb1966));bb2->bb14.bb46.bb1094=(9 );bb1135(bb2);bb2->bb1781=
bb2301;}bb50 bb2143(bb2,bb2->bb14.bb46.bb1364,bb2->bb14.bb46.bb563);}
bb2454(bb2);bbm((bb387&0x04 )==0 ){bbm(0 -bb2->bb14.bb46.bb1314>=2048 )bb2210
(bb2->bb468->bb2150,0xC0000000L );bb50 bbm(0x80000000L -bb2->bb14.bb46.
bb1314>=2048 )bb2210(bb2->bb468->bb2150,0x40000000L );bb2->bb14.bb46.
bb1314+=2048 ;bb2->bb14.bb46.bb1776+=2048 ;bb2289(bb2);}bb2->bb14.bb46.
bb2182=0 ;bb4;}bbk bb2249(bb495 bb2,bbf* *bb1770,bbf* *bb1835,bbd*
bb938,bbd*bb640,bbb*bb1354,bbk bb387,bbk bb2233){bb943 bbk bb2257;
bb943 bbk bb2105;bbk bb2242;bbk bb1077=0 ;bb2->bb468=(bb2110* )bb1354;
bb2->bb14.bb46=bb2->bb468->bb46;bb2->bb14.bb46.bb1789= *bb1770;bb2->
bb14.bb46.bb1021= *bb938;bb2->bb14.bb46.bb1774= *bb1835;bb2->bb14.
bb46.bb633= *bb640;bb2->bb14.bb46.bb2059=0 ;bb2->bb2179=0X0018 &bb387;
bbm(bb2->bb2179>0x0010 ){bb2->bb2179=0x0010 ;}bb2->bb2179>>=3 ;bbm( *
bb640<=15 )bb1077=0 ;bb50 bbm(bb2->bb14.bb46.bb1021!=0 ){bb2->bb14.bb46.
bb633-=15 ;bbm(bb2->bb14.bb46.bb2182==0 ){bb2->bb14.bb46.bb1966= *bb2->
bb14.bb46.bb1789++;--bb2->bb14.bb46.bb1021;++bb2->bb14.bb46.bb1314;++
bb2->bb14.bb46.bb1776;bb2->bb14.bb46.bb2113=(bbk)bb2->bb14.bb46.
bb1776&(2048 -1 );bb2->bb468->bb1893[(bbk)bb2->bb14.bb46.bb1314&(2048 -1
)]=bb2->bb14.bb46.bb1966;bb2->bb14.bb46.bb1618=(bb2->bb14.bb46.bb1618
<<8 )+bb2->bb14.bb46.bb1966;bb2->bb14.bb46.bb2182=1 ;}bb109((bb2->bb14.
bb46.bb1021!=0 )&&(bb2->bb14.bb46.bb2059==0 )){++bb2->bb14.bb46.bb1314;
++bb2->bb14.bb46.bb1776;bb2->bb14.bb46.bb2113=(bbk)bb2->bb14.bb46.
bb1776&(2048 -1 );bbm(((bb2->bb14.bb46.bb1314&0x7FFFFFFFL )==0 ))bb2449(
bb2);bb2->bb468->bb1893[(bbk)bb2->bb14.bb46.bb1314&(2048 -1 )]=bb2->
bb14.bb46.bb1966= *bb2->bb14.bb46.bb1789++;bb2->bb14.bb46.bb1618=(bb2
->bb14.bb46.bb1618<<8 )+bb2->bb14.bb46.bb1966;--bb2->bb14.bb46.bb1021;
bb2->bb14.bb46.bb2339=bb2->bb468->bb2150+((((bb2->bb14.bb46.bb1618)&
0xFF00 )>>4 )^((bb2->bb14.bb46.bb1618)&0x00FF ));bbm((bb2->bb14.bb46.
bb2362=bb2->bb14.bb46.bb1776- *bb2->bb14.bb46.bb2339)>2048 -2 ){bb2->
bb468->bb1927[bb2->bb14.bb46.bb2113]=0 ;bbm(bb2->bb14.bb46.bb563!=0 ){
bb2143(bb2,bb2->bb14.bb46.bb1364,bb2->bb14.bb46.bb563);}bb50{bb2->
bb14.bb46.bb1055=((bb2->bb14.bb46.bb1618>>8 ));bb2->bb14.bb46.bb1094=(
9 );bb1135(bb2);bb2->bb1781=bb2301;}}bb50{bb2->bb468->bb1927[bb2->bb14
.bb46.bb2113]=(bbk)bb2->bb14.bb46.bb2362;bbm(bb2->bb14.bb46.bb563!=0 ){
bbm((bb2->bb468->bb1893[(bbk)(((bbd)bb2->bb14.bb46.bb1297+bb2->bb14.
bb46.bb563+bb2->bb14.bb46.bb1890)&(bbd)(2048 -1 ))]==bb2->bb14.bb46.
bb1966)&&((bb2->bb14.bb46.bb563+bb2->bb14.bb46.bb1890)<(bbd)0xFFFFFFFFL
)){++bb2->bb14.bb46.bb563;bb2->bb1781=bb2566;bbm(bb2->bb14.bb46.
bb1868){bbm(bb2->bb14.bb46.bb563>=23 ){bb2->bb14.bb46.bb1055=(((1 <<4 )-
1 ));bb2->bb14.bb46.bb1094=(4 );bb1135(bb2);bb2->bb14.bb46.bb563-=((1 <<
4 )-1 );bb2->bb14.bb46.bb1890+=((1 <<4 )-1 );}}bb50 bbm(bb2->bb14.bb46.
bb563>=23 ){bbm(bb2->bb14.bb46.bb1364<128 ){bb2->bb14.bb46.bb1055=(
0x180 |bb2->bb14.bb46.bb1364);bb2->bb14.bb46.bb1094=(9 );bb1135(bb2);}
bb50{bb2->bb14.bb46.bb1055=(0x1000 |bb2->bb14.bb46.bb1364);bb2->bb14.
bb46.bb1094=(13 );bb1135(bb2);}bb2->bb14.bb46.bb1055=(((1 <<4 )-1 ));bb2
->bb14.bb46.bb1094=(4 );bb1135(bb2);bb2->bb14.bb46.bb563-=((1 <<4 )-1 );
bb2->bb14.bb46.bb1890+=((1 <<4 )-1 );bb2->bb14.bb46.bb1868=1 ;}}bb50 bbm(
bb2->bb14.bb46.bb1868){bb2->bb14.bb46.bb1055=(bb2228[(bbk)bb2->bb14.
bb46.bb563].bb2384);bb2->bb14.bb46.bb1094=(bb2228[(bbk)bb2->bb14.bb46
.bb563].bb48);bb1135(bb2);bb2->bb14.bb46.bb563=0 ;bb2->bb14.bb46.
bb1890=0 ;bb2->bb14.bb46.bb1868=0 ;bb2->bb1781=bb2466;}bb50 bbm(bb2->
bb14.bb46.bb563>=8 ){bb2143(bb2,bb2->bb14.bb46.bb1364,bb2->bb14.bb46.
bb563);}bb50{bb2105=0 ;bb2->bb14.bb46.bb2165=bb2->bb14.bb46.bb1364;
bb109((bb2->bb468->bb1927[bb2->bb14.bb46.bb1297]!=0 )&&(bb2105==0 )&&(
bb2->bb14.bb46.bb2123<bb2233)&&(bb2->bb14.bb46.bb2165<(bbk)(2048 -bb2
->bb14.bb46.bb563))){bb2->bb14.bb46.bb2165+=bb2->bb468->bb1927[bb2->
bb14.bb46.bb1297];++bb2->bb14.bb46.bb2123;bbm(bb2->bb14.bb46.bb2165<(
bbk)(2048 -bb2->bb14.bb46.bb563)){bb2->bb14.bb46.bb1297=bb2->bb14.bb46
.bb1297-bb2->bb468->bb1927[bb2->bb14.bb46.bb1297]&(2048 -1 );bbm(bb2->
bb468->bb1893[bb2->bb14.bb46.bb1297]==bb2->bb468->bb1893[bb2->bb14.
bb46.bb2319]){bb2105=1 ;bb90(bb2257=2 ,bb2242=(bb2->bb14.bb46.bb1297+2 )&
(2048 -1 );bb2257<=(bbk)bb2->bb14.bb46.bb563;bb2257++,bb2242=(bb2242+1 )&
(2048 -1 )){bbm(bb2->bb468->bb1893[bb2242]!=bb2->bb468->bb1893[(bb2->
bb14.bb46.bb2319+bb2257)&(2048 -1 )]){bb2105=0 ;bb21;}}}}}bbm(bb2105){
bb2->bb14.bb46.bb1364=bb2->bb14.bb46.bb2165;++bb2->bb14.bb46.bb563;
bb2->bb1781=bb2550;}bb50{bb2143(bb2,bb2->bb14.bb46.bb1364,bb2->bb14.
bb46.bb563);}}}bb50{bb2->bb14.bb46.bb2115=(bbk)bb2->bb14.bb46.bb2362;
bb2->bb14.bb46.bb2123=0 ;bb599{bb2->bb14.bb46.bb1297=(bbk)(bb2->bb14.
bb46.bb1776-bb2->bb14.bb46.bb2115&(2048 -1 ));bbm(bb2->bb468->bb1893[
bb2->bb14.bb46.bb1297]==(bbf)(bb2->bb14.bb46.bb1618>>8 )){bb2->bb14.
bb46.bb563=2 ;bb2->bb14.bb46.bb2319=bb2->bb14.bb46.bb2113;bb2->bb14.
bb46.bb1364=bb2->bb14.bb46.bb2115;bb2->bb1781=bb2640;bb21;}bb50{bb2->
bb14.bb46.bb2115+=bb2->bb468->bb1927[bb2->bb14.bb46.bb1297];++bb2->
bb14.bb46.bb2123;}}bb109((bb2->bb468->bb1927[bb2->bb14.bb46.bb1297]!=
0 )&&(bb2->bb14.bb46.bb2123<bb2233)&&(bb2->bb14.bb46.bb2115<2048 -2 ));
bbm(bb2->bb14.bb46.bb563==0 ){bb2->bb14.bb46.bb1055=((bb2->bb14.bb46.
bb1618>>8 ));bb2->bb14.bb46.bb1094=(9 );bb1135(bb2);bb2->bb1781=bb2301;
}}}bbm(bb2557[bb2->bb1781][bb2->bb2179]){ *bb2->bb14.bb46.bb2339=bb2
->bb14.bb46.bb1776;}}bbm(bb2->bb14.bb46.bb1021==0 ){bb1077=1 ;bbm(bb387
&0x01 ){bb2267(bb2,bb387);bb1077|=4 ;}}bbm((bb2->bb14.bb46.bb633==0 )||(
bb2->bb14.bb46.bb2059)){bbm(!(bb1077&1 )){bb1077=2 ;bbm(bb387&0x02 ){
bb2267(bb2,bb387);bb1077|=4 ;}}bb50{bb1077|=3 ;bbm((!(bb387&0x01 ))&&(
bb387&0x02 )){bb2267(bb2,bb387);bb1077|=4 ;}}}bb2->bb14.bb46.bb633+=15 ;
}bb50{bb1077=1 ;bbm(bb387&0x01 ){bb2267(bb2,bb387);bb1077|=4 ;}}bb2->
bb468->bb46=bb2->bb14.bb46; *bb1770=bb2->bb14.bb46.bb1789; *bb938=bb2
->bb14.bb46.bb1021; *bb1835=bb2->bb14.bb46.bb1774; *bb640=bb2->bb14.
bb46.bb633;bb4(bb1077);}bb40 bbb bb2197(bb495 bb2){bb2->bb14.bb86.
bb2157&=(2048 -1 );bb2->bb14.bb86.bb1082=bb2->bb14.bb86.bb1190=bb2->
bb14.bb86.bb97=0 ;bb2->bb14.bb86.bb1054=bb2->bb14.bb86.bb1795=bb2->
bb14.bb86.bb1186=0 ;bb2->bb14.bb86.bb483=0 ;bb2->bb14.bb86.bb2030=0 ;bb2
->bb14.bb86.bb1205=bb2149;bb2->bb14.bb86.bb2650=1 ;bb4;}bb40 bbk bb2353
(bb495 bb2){bbm((bb2->bb14.bb86.bb1021==0 )&&(bb2->bb14.bb86.bb1082==0
))bb2->bb14.bb86.bb1054=bb2452;bb50{bb2->bb14.bb86.bb1054=bb1292;bbm(
bb2->bb14.bb86.bb1082==0 ){bb2->bb14.bb86.bb1795= *bb2->bb14.bb86.
bb1789++;--bb2->bb14.bb86.bb1021;bb2->bb14.bb86.bb1082=7 ;bb2->bb14.
bb86.bb1186=(bb2->bb14.bb86.bb1795>127 )?1 :0 ;bb2->bb14.bb86.bb1795&=((
bbf)0x7F );}bb50{bb2->bb14.bb86.bb1186=(bb2->bb14.bb86.bb1795>>(bb2->
bb14.bb86.bb1082-1 ));--bb2->bb14.bb86.bb1082;bb2->bb14.bb86.bb1795&=(
(bbf)0xFF >>(8 -bb2->bb14.bb86.bb1082));}}bb4(bb2->bb14.bb86.bb1054);}
bb40 bbk bb2017(bb495 bb2){bbk bb2227;bb125 bb10;bbm(bb2->bb14.bb86.
bb1054==bb1292)bb2->bb14.bb86.bb1186=0 ;bb50 bb2->bb14.bb86.bb1054=
bb1292;bb109((bb2->bb14.bb86.bb1190>0 )&&(bb2->bb14.bb86.bb1054==
bb1292)){bbm((bb2->bb14.bb86.bb1021==0 )&&(bb2->bb14.bb86.bb1082==0 ))bb2
->bb14.bb86.bb1054=bb2452;bb50{bbm(bb2->bb14.bb86.bb1082==0 ){bb2->
bb14.bb86.bb1795= *bb2->bb14.bb86.bb1789++;--bb2->bb14.bb86.bb1021;
bb2->bb14.bb86.bb1082=8 ;}bb2227=bb2->bb14.bb86.bb1795;bbm((bb10=bb2->
bb14.bb86.bb1190-bb2->bb14.bb86.bb1082)>0 )bb2227<<=bb10;bb50 bb2227
>>=-bb10;bb2->bb14.bb86.bb1186|=bb2227;bb10=((((8 )<(bb2->bb14.bb86.
bb1190)?(8 ):(bb2->bb14.bb86.bb1190)))<(bb2->bb14.bb86.bb1082)?(((8 )<(
bb2->bb14.bb86.bb1190)?(8 ):(bb2->bb14.bb86.bb1190))):(bb2->bb14.bb86.
bb1082));bb2->bb14.bb86.bb1190-=bb10;bb2->bb14.bb86.bb1082-=bb10;bb2
->bb14.bb86.bb1795&=((bbf)0xFF >>(8 -bb2->bb14.bb86.bb1082));}}bb4(bb2
->bb14.bb86.bb1054);}bb40 bbb bb2137(bb495 bb2,bbf bbn){bbm(bb2->bb14
.bb86.bb633!=0 ){ *bb2->bb14.bb86.bb1774++=bbn;--bb2->bb14.bb86.bb633;
bb2->bb468->bb2236[bb2->bb14.bb86.bb2157++]=(bbf)bbn;bb2->bb14.bb86.
bb2157&=(2048 -1 );}}bbk bb2365(bb495 bb2,bbf* *bb1770,bbf* *bb1835,bbd
 *bb938,bbd*bb640,bbb*bb1354,bbk bb387){bbk bb2272=0 ;bbk bb1077=0 ;bb2
->bb468=(bb2110* )bb1354;bb2->bb14.bb86=bb2->bb468->bb86;bb2->bb14.
bb86.bb1789= *bb1770;bb2->bb14.bb86.bb1021= *bb938;bb2->bb14.bb86.
bb1774= *bb1835;bb2->bb14.bb86.bb633= *bb640;bbm(bb387&0x01 ){bb2197(
bb2);}bbm((bb2->bb14.bb86.bb1021!=0 )&&(bb2->bb14.bb86.bb633!=0 )){
bb109((bb2->bb14.bb86.bb633!=0 )&&((bb2->bb14.bb86.bb1021!=0 )||(bb2->
bb14.bb86.bb1082!=0 ))&&(bb2272==0 )){bbm(bb2->bb14.bb86.bb2030){bb109(
(bb2->bb14.bb86.bb633!=0 )&&(bb2->bb14.bb86.bb483>0 )){bb2->bb14.bb86.
bb97&=(2048 -1 );bb2137(bb2,bb2->bb468->bb2236[bb2->bb14.bb86.bb97++]);
--bb2->bb14.bb86.bb483;}bbm(bb2->bb14.bb86.bb483==0 )bb2->bb14.bb86.
bb2030=0 ;bb2->bb14.bb86.bb1205=bb2149;}bb50{bb338(bb2->bb14.bb86.
bb1205){bb17 bb2149:bb2353(bb2);bbm(bb2->bb14.bb86.bb1186==0 ){bb2->
bb14.bb86.bb1190=8 ;bb2->bb14.bb86.bb1205=bb2421;bb17 bb2421:bb2017(
bb2);bbm(bb2->bb14.bb86.bb1054==bb1292){bb2137(bb2,(bbf)bb2->bb14.
bb86.bb1186);bb2->bb14.bb86.bb1205=bb2149;}}bb50{bb2->bb14.bb86.
bb1205=bb2458;bb17 bb2458:bb2353(bb2);bbm(bb2->bb14.bb86.bb1054==
bb1292){bb2->bb14.bb86.bb1190=bb2->bb14.bb86.bb1186?7 :11 ;bb2->bb14.
bb86.bb1205=bb2424;bb17 bb2424:bb2017(bb2);bbm(bb2->bb14.bb86.bb1054
==bb1292){bb2->bb14.bb86.bb97=bb2->bb14.bb86.bb1186;bbm(bb2->bb14.
bb86.bb97==0 )bb2272=1 ;bb50{bb2->bb14.bb86.bb97=bb2->bb14.bb86.bb2157-
bb2->bb14.bb86.bb97;bb2->bb14.bb86.bb1190=2 ;bb2->bb14.bb86.bb1205=
bb2519;bb17 bb2519:bb2017(bb2);bbm(bb2->bb14.bb86.bb1054==bb1292){bb2
->bb14.bb86.bb483=2 +bb2->bb14.bb86.bb1186;bbm(bb2->bb14.bb86.bb483==5
){bb2->bb14.bb86.bb1190=2 ;bb2->bb14.bb86.bb1205=bb2518;bb17 bb2518:
bb2017(bb2);bbm(bb2->bb14.bb86.bb1054==bb1292){bb2->bb14.bb86.bb483+=
bb2->bb14.bb86.bb1186;bbm(bb2->bb14.bb86.bb483==8 ){bb2->bb14.bb86.
bb1190=4 ;bb2->bb14.bb86.bb1205=bb2501;bb17 bb2501:bb2017(bb2);bbm(bb2
->bb14.bb86.bb1054==bb1292){bb2->bb14.bb86.bb483+=bb2->bb14.bb86.
bb1186;bbm(bb2->bb14.bb86.bb483==23 ){bb599{bb17 bb2310:bb109((bb2->
bb14.bb86.bb633!=0 )&&(bb2->bb14.bb86.bb483>0 )){bb2->bb14.bb86.bb97&=(
2048 -1 );bb2137(bb2,bb2->bb468->bb2236[bb2->bb14.bb86.bb97++]);--bb2->
bb14.bb86.bb483;}bbm(bb2->bb14.bb86.bb633==0 ){bb2->bb14.bb86.bb1205=
bb2310;bb21;}bb50{bb2->bb14.bb86.bb1190=4 ;bb2->bb14.bb86.bb1205=
bb2439;bb17 bb2439:bb2017(bb2);bbm(bb2->bb14.bb86.bb1054==bb1292)bb2
->bb14.bb86.bb483+=bb2->bb14.bb86.bb1186;bb50 bb21;}}bb109(bb2->bb14.
bb86.bb1186==((1 <<4 )-1 ));}}}}}}bbm((bb2->bb14.bb86.bb1054==bb1292)&&(
bb2->bb14.bb86.bb1205!=bb2310)){bb2->bb14.bb86.bb2030=1 ;}}}}}}}}bbm(
bb2->bb14.bb86.bb2030){bb109((bb2->bb14.bb86.bb633!=0 )&&(bb2->bb14.
bb86.bb483>0 )){bb2->bb14.bb86.bb97&=(2048 -1 );bb2137(bb2,bb2->bb468->
bb2236[bb2->bb14.bb86.bb97++]);--bb2->bb14.bb86.bb483;}bbm(bb2->bb14.
bb86.bb483==0 )bb2->bb14.bb86.bb2030=0 ;bb2->bb14.bb86.bb1205=bb2149;}}
bbm(bb2272){bb2197(bb2);bb1077|=4 ;}bbm(bb2->bb14.bb86.bb1021==0 ){
bb1077|=1 ;}bbm(bb2->bb14.bb86.bb633==0 ){bb1077|=2 ;}bb2->bb468->bb86=
bb2->bb14.bb86; *bb1770=bb2->bb14.bb86.bb1789; *bb938=bb2->bb14.bb86.
bb1021; *bb1835=bb2->bb14.bb86.bb1774; *bb640=bb2->bb14.bb86.bb633;
bb4(bb1077);}
