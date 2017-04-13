/*
   'ripemd.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbj{bbd bb5;bbd bb23[5 ];bbf bb105[64 ];}bb463;bbb bb1843(bb463*bbi
);bbb bb1338(bb463*bbi,bbh bbb*bb516,bbo bb5);bbb bb1899(bb463*bbi,
bbb*bb1);bbb bb1979(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2032(bbb*bb1,
bb62 bbx);
#ifdef __cplusplus
}
#endif
bb40 bbb bb1298(bbd bb23[5 ],bbh bbf bb98[64 ]){bb27(bb12(bbe)>=4 );{bbd
bb69,bb64,bb70,bb58,bb66,bb67,bb65,bb52,bb71,bb68;bbd bbw[16 ],bbz;
bb69=bb64=bb23[0 ];bb70=bb58=bb23[1 ];bb66=bb67=bb23[2 ];bb65=bb52=bb23[
3 ];bb71=bb68=bb23[4 ];bb90(bbz=0 ;bbz<16 ;bbz++,bb98+=4 )bbw[bbz]=(bb98[0
]|bb98[1 ]<<8 |bb98[2 ]<<16 |bb98[3 ]<<24 );bb64+=(bb58^bb67^bb52)+bbw[0 ];
bb64=((bb64)<<(11 )|(bb64)>>(32 -11 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(
32 -10 ));bb68+=(bb64^bb58^bb67)+bbw[1 ];bb68=((bb68)<<(14 )|(bb68)>>(32 -
14 ))+bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=(bb68^bb64^bb58)+
bbw[2 ];bb52=((bb52)<<(15 )|(bb52)>>(32 -15 ))+bb67;bb64=((bb64)<<(10 )|(
bb64)>>(32 -10 ));bb67+=(bb52^bb68^bb64)+bbw[3 ];bb67=((bb67)<<(12 )|(
bb67)>>(32 -12 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67^
bb52^bb68)+bbw[4 ];bb58=((bb58)<<(5 )|(bb58)>>(32 -5 ))+bb64;bb52=((bb52)<<
(10 )|(bb52)>>(32 -10 ));bb64+=(bb58^bb67^bb52)+bbw[5 ];bb64=((bb64)<<(8 )|
(bb64)>>(32 -8 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64^
bb58^bb67)+bbw[6 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+bb52;bb58=((bb58)<<
(10 )|(bb58)>>(32 -10 ));bb52+=(bb68^bb64^bb58)+bbw[7 ];bb52=((bb52)<<(9 )|
(bb52)>>(32 -9 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb52^
bb68^bb64)+bbw[8 ];bb67=((bb67)<<(11 )|(bb67)>>(32 -11 ))+bb58;bb68=((
bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67^bb52^bb68)+bbw[9 ];bb58=((
bb58)<<(13 )|(bb58)>>(32 -13 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));
bb64+=(bb58^bb67^bb52)+bbw[10 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14 ))+
bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64^bb58^bb67)+bbw[
11 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+bb52;bb58=((bb58)<<(10 )|(bb58
)>>(32 -10 ));bb52+=(bb68^bb64^bb58)+bbw[12 ];bb52=((bb52)<<(6 )|(bb52)>>
(32 -6 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb52^bb68^
bb64)+bbw[13 ];bb67=((bb67)<<(7 )|(bb67)>>(32 -7 ))+bb58;bb68=((bb68)<<(
10 )|(bb68)>>(32 -10 ));bb58+=(bb67^bb52^bb68)+bbw[14 ];bb58=((bb58)<<(9 )|
(bb58)>>(32 -9 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58^
bb67^bb52)+bbw[15 ];bb64=((bb64)<<(8 )|(bb64)>>(32 -8 ))+bb68;bb67=((bb67
)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&bb58|~bb64&bb67)+0x5a827999 +bbw[
7 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+bb52;bb58=((bb58)<<(10 )|(bb58)>>
(32 -10 ));bb52+=(bb68&bb64|~bb68&bb58)+0x5a827999 +bbw[4 ];bb52=((bb52)<<
(6 )|(bb52)>>(32 -6 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(
bb52&bb68|~bb52&bb64)+0x5a827999 +bbw[13 ];bb67=((bb67)<<(8 )|(bb67)>>(
32 -8 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67&bb52|~
bb67&bb68)+0x5a827999 +bbw[1 ];bb58=((bb58)<<(13 )|(bb58)>>(32 -13 ))+bb64
;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58&bb67|~bb58&bb52)+
0x5a827999 +bbw[10 ];bb64=((bb64)<<(11 )|(bb64)>>(32 -11 ))+bb68;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&bb58|~bb64&bb67)+0x5a827999 +
bbw[6 ];bb68=((bb68)<<(9 )|(bb68)>>(32 -9 ))+bb52;bb58=((bb58)<<(10 )|(
bb58)>>(32 -10 ));bb52+=(bb68&bb64|~bb68&bb58)+0x5a827999 +bbw[15 ];bb52=
((bb52)<<(7 )|(bb52)>>(32 -7 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb52&bb68|~bb52&bb64)+0x5a827999 +bbw[3 ];bb67=((bb67)<<(15 )|(
bb67)>>(32 -15 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67&
bb52|~bb67&bb68)+0x5a827999 +bbw[12 ];bb58=((bb58)<<(7 )|(bb58)>>(32 -7 ))+
bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58&bb67|~bb58&bb52)+
0x5a827999 +bbw[0 ];bb64=((bb64)<<(12 )|(bb64)>>(32 -12 ))+bb68;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&bb58|~bb64&bb67)+0x5a827999 +
bbw[9 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+bb52;bb58=((bb58)<<(10 )|(
bb58)>>(32 -10 ));bb52+=(bb68&bb64|~bb68&bb58)+0x5a827999 +bbw[5 ];bb52=(
(bb52)<<(9 )|(bb52)>>(32 -9 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb52&bb68|~bb52&bb64)+0x5a827999 +bbw[2 ];bb67=((bb67)<<(11 )|(
bb67)>>(32 -11 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67&
bb52|~bb67&bb68)+0x5a827999 +bbw[14 ];bb58=((bb58)<<(7 )|(bb58)>>(32 -7 ))+
bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58&bb67|~bb58&bb52)+
0x5a827999 +bbw[11 ];bb64=((bb64)<<(13 )|(bb64)>>(32 -13 ))+bb68;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&bb58|~bb64&bb67)+0x5a827999 +
bbw[8 ];bb68=((bb68)<<(12 )|(bb68)>>(32 -12 ))+bb52;bb58=((bb58)<<(10 )|(
bb58)>>(32 -10 ));bb52+=((bb68|~bb64)^bb58)+0x6ed9eba1 +bbw[3 ];bb52=((
bb52)<<(11 )|(bb52)>>(32 -11 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=((bb52|~bb68)^bb64)+0x6ed9eba1 +bbw[10 ];bb67=((bb67)<<(13 )|(bb67
)>>(32 -13 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=((bb67|~
bb52)^bb68)+0x6ed9eba1 +bbw[14 ];bb58=((bb58)<<(6 )|(bb58)>>(32 -6 ))+bb64
;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=((bb58|~bb67)^bb52)+
0x6ed9eba1 +bbw[4 ];bb64=((bb64)<<(7 )|(bb64)>>(32 -7 ))+bb68;bb67=((bb67)<<
(10 )|(bb67)>>(32 -10 ));bb68+=((bb64|~bb58)^bb67)+0x6ed9eba1 +bbw[9 ];
bb68=((bb68)<<(14 )|(bb68)>>(32 -14 ))+bb52;bb58=((bb58)<<(10 )|(bb58)>>(
32 -10 ));bb52+=((bb68|~bb64)^bb58)+0x6ed9eba1 +bbw[15 ];bb52=((bb52)<<(9
)|(bb52)>>(32 -9 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=((
bb52|~bb68)^bb64)+0x6ed9eba1 +bbw[8 ];bb67=((bb67)<<(13 )|(bb67)>>(32 -13
))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=((bb67|~bb52)^bb68)+
0x6ed9eba1 +bbw[1 ];bb58=((bb58)<<(15 )|(bb58)>>(32 -15 ))+bb64;bb52=((
bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=((bb58|~bb67)^bb52)+0x6ed9eba1 +bbw
[2 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14 ))+bb68;bb67=((bb67)<<(10 )|(bb67
)>>(32 -10 ));bb68+=((bb64|~bb58)^bb67)+0x6ed9eba1 +bbw[7 ];bb68=((bb68)<<
(8 )|(bb68)>>(32 -8 ))+bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=((
bb68|~bb64)^bb58)+0x6ed9eba1 +bbw[0 ];bb52=((bb52)<<(13 )|(bb52)>>(32 -13
))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=((bb52|~bb68)^bb64)+
0x6ed9eba1 +bbw[6 ];bb67=((bb67)<<(6 )|(bb67)>>(32 -6 ))+bb58;bb68=((bb68)<<
(10 )|(bb68)>>(32 -10 ));bb58+=((bb67|~bb52)^bb68)+0x6ed9eba1 +bbw[13 ];
bb58=((bb58)<<(5 )|(bb58)>>(32 -5 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32
-10 ));bb64+=((bb58|~bb67)^bb52)+0x6ed9eba1 +bbw[11 ];bb64=((bb64)<<(12 )|
(bb64)>>(32 -12 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=((
bb64|~bb58)^bb67)+0x6ed9eba1 +bbw[5 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+
bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=((bb68|~bb64)^bb58)+
0x6ed9eba1 +bbw[12 ];bb52=((bb52)<<(5 )|(bb52)>>(32 -5 ))+bb67;bb64=((bb64
)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb52&bb64|bb68&~bb64)+0x8f1bbcdc +bbw[
1 ];bb67=((bb67)<<(11 )|(bb67)>>(32 -11 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>
(32 -10 ));bb58+=(bb67&bb68|bb52&~bb68)+0x8f1bbcdc +bbw[9 ];bb58=((bb58)<<
(12 )|(bb58)>>(32 -12 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=
(bb58&bb52|bb67&~bb52)+0x8f1bbcdc +bbw[11 ];bb64=((bb64)<<(14 )|(bb64)>>
(32 -14 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&bb67|
bb58&~bb67)+0x8f1bbcdc +bbw[10 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+
bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=(bb68&bb58|bb64&~bb58)+
0x8f1bbcdc +bbw[0 ];bb52=((bb52)<<(14 )|(bb52)>>(32 -14 ))+bb67;bb64=((
bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb52&bb64|bb68&~bb64)+0x8f1bbcdc +
bbw[8 ];bb67=((bb67)<<(15 )|(bb67)>>(32 -15 ))+bb58;bb68=((bb68)<<(10 )|(
bb68)>>(32 -10 ));bb58+=(bb67&bb68|bb52&~bb68)+0x8f1bbcdc +bbw[12 ];bb58=
((bb58)<<(9 )|(bb58)>>(32 -9 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));
bb64+=(bb58&bb52|bb67&~bb52)+0x8f1bbcdc +bbw[4 ];bb64=((bb64)<<(8 )|(
bb64)>>(32 -8 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&
bb67|bb58&~bb67)+0x8f1bbcdc +bbw[13 ];bb68=((bb68)<<(9 )|(bb68)>>(32 -9 ))+
bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=(bb68&bb58|bb64&~bb58)+
0x8f1bbcdc +bbw[3 ];bb52=((bb52)<<(14 )|(bb52)>>(32 -14 ))+bb67;bb64=((
bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb52&bb64|bb68&~bb64)+0x8f1bbcdc +
bbw[7 ];bb67=((bb67)<<(5 )|(bb67)>>(32 -5 ))+bb58;bb68=((bb68)<<(10 )|(
bb68)>>(32 -10 ));bb58+=(bb67&bb68|bb52&~bb68)+0x8f1bbcdc +bbw[15 ];bb58=
((bb58)<<(6 )|(bb58)>>(32 -6 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));
bb64+=(bb58&bb52|bb67&~bb52)+0x8f1bbcdc +bbw[14 ];bb64=((bb64)<<(8 )|(
bb64)>>(32 -8 ))+bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64&
bb67|bb58&~bb67)+0x8f1bbcdc +bbw[5 ];bb68=((bb68)<<(6 )|(bb68)>>(32 -6 ))+
bb52;bb58=((bb58)<<(10 )|(bb58)>>(32 -10 ));bb52+=(bb68&bb58|bb64&~bb58)+
0x8f1bbcdc +bbw[6 ];bb52=((bb52)<<(5 )|(bb52)>>(32 -5 ))+bb67;bb64=((bb64)<<
(10 )|(bb64)>>(32 -10 ));bb67+=(bb52&bb64|bb68&~bb64)+0x8f1bbcdc +bbw[2 ];
bb67=((bb67)<<(12 )|(bb67)>>(32 -12 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(
32 -10 ));bb58+=(bb67^(bb52|~bb68))+0xa953fd4e +bbw[4 ];bb58=((bb58)<<(9 )|
(bb58)>>(32 -9 ))+bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58^
(bb67|~bb52))+0xa953fd4e +bbw[0 ];bb64=((bb64)<<(15 )|(bb64)>>(32 -15 ))+
bb68;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64^(bb58|~bb67))+
0xa953fd4e +bbw[5 ];bb68=((bb68)<<(5 )|(bb68)>>(32 -5 ))+bb52;bb58=((bb58)<<
(10 )|(bb58)>>(32 -10 ));bb52+=(bb68^(bb64|~bb58))+0xa953fd4e +bbw[9 ];
bb52=((bb52)<<(11 )|(bb52)>>(32 -11 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(
32 -10 ));bb67+=(bb52^(bb68|~bb64))+0xa953fd4e +bbw[7 ];bb67=((bb67)<<(6 )|
(bb67)>>(32 -6 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67^
(bb52|~bb68))+0xa953fd4e +bbw[12 ];bb58=((bb58)<<(8 )|(bb58)>>(32 -8 ))+
bb64;bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58^(bb67|~bb52))+
0xa953fd4e +bbw[2 ];bb64=((bb64)<<(13 )|(bb64)>>(32 -13 ))+bb68;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64^(bb58|~bb67))+0xa953fd4e +bbw
[10 ];bb68=((bb68)<<(12 )|(bb68)>>(32 -12 ))+bb52;bb58=((bb58)<<(10 )|(
bb58)>>(32 -10 ));bb52+=(bb68^(bb64|~bb58))+0xa953fd4e +bbw[14 ];bb52=((
bb52)<<(5 )|(bb52)>>(32 -5 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb52^(bb68|~bb64))+0xa953fd4e +bbw[1 ];bb67=((bb67)<<(12 )|(bb67)>>
(32 -12 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67^(bb52|~
bb68))+0xa953fd4e +bbw[3 ];bb58=((bb58)<<(13 )|(bb58)>>(32 -13 ))+bb64;
bb52=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb64+=(bb58^(bb67|~bb52))+
0xa953fd4e +bbw[8 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14 ))+bb68;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb68+=(bb64^(bb58|~bb67))+0xa953fd4e +bbw
[11 ];bb68=((bb68)<<(11 )|(bb68)>>(32 -11 ))+bb52;bb58=((bb58)<<(10 )|(
bb58)>>(32 -10 ));bb52+=(bb68^(bb64|~bb58))+0xa953fd4e +bbw[6 ];bb52=((
bb52)<<(8 )|(bb52)>>(32 -8 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb52^(bb68|~bb64))+0xa953fd4e +bbw[15 ];bb67=((bb67)<<(5 )|(bb67)>>
(32 -5 ))+bb58;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb58+=(bb67^(bb52|~
bb68))+0xa953fd4e +bbw[13 ];bb58=((bb58)<<(6 )|(bb58)>>(32 -6 ))+bb64;bb52
=((bb52)<<(10 )|(bb52)>>(32 -10 ));bb69+=(bb70^(bb66|~bb65))+0x50a28be6 +
bbw[5 ];bb69=((bb69)<<(8 )|(bb69)>>(32 -8 ))+bb71;bb66=((bb66)<<(10 )|(
bb66)>>(32 -10 ));bb71+=(bb69^(bb70|~bb66))+0x50a28be6 +bbw[14 ];bb71=((
bb71)<<(9 )|(bb71)>>(32 -9 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));
bb65+=(bb71^(bb69|~bb70))+0x50a28be6 +bbw[7 ];bb65=((bb65)<<(9 )|(bb65)>>
(32 -9 ))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65^(bb71|~
bb69))+0x50a28be6 +bbw[0 ];bb66=((bb66)<<(11 )|(bb66)>>(32 -11 ))+bb70;
bb71=((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66^(bb65|~bb71))+
0x50a28be6 +bbw[9 ];bb70=((bb70)<<(13 )|(bb70)>>(32 -13 ))+bb69;bb65=((
bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=(bb70^(bb66|~bb65))+0x50a28be6 +bbw
[2 ];bb69=((bb69)<<(15 )|(bb69)>>(32 -15 ))+bb71;bb66=((bb66)<<(10 )|(bb66
)>>(32 -10 ));bb71+=(bb69^(bb70|~bb66))+0x50a28be6 +bbw[11 ];bb71=((bb71)<<
(15 )|(bb71)>>(32 -15 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=
(bb71^(bb69|~bb70))+0x50a28be6 +bbw[4 ];bb65=((bb65)<<(5 )|(bb65)>>(32 -5
))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65^(bb71|~bb69))+
0x50a28be6 +bbw[13 ];bb66=((bb66)<<(7 )|(bb66)>>(32 -7 ))+bb70;bb71=((bb71
)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66^(bb65|~bb71))+0x50a28be6 +bbw[6 ];
bb70=((bb70)<<(7 )|(bb70)>>(32 -7 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32
-10 ));bb69+=(bb70^(bb66|~bb65))+0x50a28be6 +bbw[15 ];bb69=((bb69)<<(8 )|
(bb69)>>(32 -8 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69^
(bb70|~bb66))+0x50a28be6 +bbw[8 ];bb71=((bb71)<<(11 )|(bb71)>>(32 -11 ))+
bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71^(bb69|~bb70))+
0x50a28be6 +bbw[1 ];bb65=((bb65)<<(14 )|(bb65)>>(32 -14 ))+bb66;bb69=((
bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65^(bb71|~bb69))+0x50a28be6 +bbw
[10 ];bb66=((bb66)<<(14 )|(bb66)>>(32 -14 ))+bb70;bb71=((bb71)<<(10 )|(
bb71)>>(32 -10 ));bb70+=(bb66^(bb65|~bb71))+0x50a28be6 +bbw[3 ];bb70=((
bb70)<<(12 )|(bb70)>>(32 -12 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));
bb69+=(bb70^(bb66|~bb65))+0x50a28be6 +bbw[12 ];bb69=((bb69)<<(6 )|(bb69)>>
(32 -6 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69&bb66|
bb70&~bb66)+0x5c4dd124 +bbw[6 ];bb71=((bb71)<<(9 )|(bb71)>>(32 -9 ))+bb65;
bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&bb70|bb69&~bb70)+
0x5c4dd124 +bbw[11 ];bb65=((bb65)<<(13 )|(bb65)>>(32 -13 ))+bb66;bb69=((
bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb69|bb71&~bb69)+0x5c4dd124 +
bbw[3 ];bb66=((bb66)<<(15 )|(bb66)>>(32 -15 ))+bb70;bb71=((bb71)<<(10 )|(
bb71)>>(32 -10 ));bb70+=(bb66&bb71|bb65&~bb71)+0x5c4dd124 +bbw[7 ];bb70=(
(bb70)<<(7 )|(bb70)>>(32 -7 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));
bb69+=(bb70&bb65|bb66&~bb65)+0x5c4dd124 +bbw[0 ];bb69=((bb69)<<(12 )|(
bb69)>>(32 -12 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69&
bb66|bb70&~bb66)+0x5c4dd124 +bbw[13 ];bb71=((bb71)<<(8 )|(bb71)>>(32 -8 ))+
bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&bb70|bb69&~bb70)+
0x5c4dd124 +bbw[5 ];bb65=((bb65)<<(9 )|(bb65)>>(32 -9 ))+bb66;bb69=((bb69)<<
(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb69|bb71&~bb69)+0x5c4dd124 +bbw[10 ]
;bb66=((bb66)<<(11 )|(bb66)>>(32 -11 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>
(32 -10 ));bb70+=(bb66&bb71|bb65&~bb71)+0x5c4dd124 +bbw[14 ];bb70=((bb70)<<
(7 )|(bb70)>>(32 -7 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=(
bb70&bb65|bb66&~bb65)+0x5c4dd124 +bbw[15 ];bb69=((bb69)<<(7 )|(bb69)>>(
32 -7 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69&bb66|bb70
&~bb66)+0x5c4dd124 +bbw[8 ];bb71=((bb71)<<(12 )|(bb71)>>(32 -12 ))+bb65;
bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&bb70|bb69&~bb70)+
0x5c4dd124 +bbw[12 ];bb65=((bb65)<<(7 )|(bb65)>>(32 -7 ))+bb66;bb69=((bb69
)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb69|bb71&~bb69)+0x5c4dd124 +bbw[
4 ];bb66=((bb66)<<(6 )|(bb66)>>(32 -6 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>
(32 -10 ));bb70+=(bb66&bb71|bb65&~bb71)+0x5c4dd124 +bbw[9 ];bb70=((bb70)<<
(15 )|(bb70)>>(32 -15 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=
(bb70&bb65|bb66&~bb65)+0x5c4dd124 +bbw[1 ];bb69=((bb69)<<(13 )|(bb69)>>(
32 -13 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69&bb66|
bb70&~bb66)+0x5c4dd124 +bbw[2 ];bb71=((bb71)<<(11 )|(bb71)>>(32 -11 ))+
bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=((bb71|~bb69)^bb70)+
0x6d703ef3 +bbw[15 ];bb65=((bb65)<<(9 )|(bb65)>>(32 -9 ))+bb66;bb69=((bb69
)<<(10 )|(bb69)>>(32 -10 ));bb66+=((bb65|~bb71)^bb69)+0x6d703ef3 +bbw[5 ];
bb66=((bb66)<<(7 )|(bb66)>>(32 -7 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>(32
-10 ));bb70+=((bb66|~bb65)^bb71)+0x6d703ef3 +bbw[1 ];bb70=((bb70)<<(15 )|
(bb70)>>(32 -15 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=((
bb70|~bb66)^bb65)+0x6d703ef3 +bbw[3 ];bb69=((bb69)<<(11 )|(bb69)>>(32 -11
))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=((bb69|~bb70)^bb66)+
0x6d703ef3 +bbw[7 ];bb71=((bb71)<<(8 )|(bb71)>>(32 -8 ))+bb65;bb70=((bb70)<<
(10 )|(bb70)>>(32 -10 ));bb65+=((bb71|~bb69)^bb70)+0x6d703ef3 +bbw[14 ];
bb65=((bb65)<<(6 )|(bb65)>>(32 -6 ))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32
-10 ));bb66+=((bb65|~bb71)^bb69)+0x6d703ef3 +bbw[6 ];bb66=((bb66)<<(6 )|(
bb66)>>(32 -6 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=((bb66|
~bb65)^bb71)+0x6d703ef3 +bbw[9 ];bb70=((bb70)<<(14 )|(bb70)>>(32 -14 ))+
bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=((bb70|~bb66)^bb65)+
0x6d703ef3 +bbw[11 ];bb69=((bb69)<<(12 )|(bb69)>>(32 -12 ))+bb71;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=((bb69|~bb70)^bb66)+0x6d703ef3 +bbw
[8 ];bb71=((bb71)<<(13 )|(bb71)>>(32 -13 ))+bb65;bb70=((bb70)<<(10 )|(bb70
)>>(32 -10 ));bb65+=((bb71|~bb69)^bb70)+0x6d703ef3 +bbw[12 ];bb65=((bb65)<<
(5 )|(bb65)>>(32 -5 ))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=((
bb65|~bb71)^bb69)+0x6d703ef3 +bbw[2 ];bb66=((bb66)<<(14 )|(bb66)>>(32 -14
))+bb70;bb71=((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=((bb66|~bb65)^bb71)+
0x6d703ef3 +bbw[10 ];bb70=((bb70)<<(13 )|(bb70)>>(32 -13 ))+bb69;bb65=((
bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=((bb70|~bb66)^bb65)+0x6d703ef3 +bbw
[0 ];bb69=((bb69)<<(13 )|(bb69)>>(32 -13 ))+bb71;bb66=((bb66)<<(10 )|(bb66
)>>(32 -10 ));bb71+=((bb69|~bb70)^bb66)+0x6d703ef3 +bbw[4 ];bb71=((bb71)<<
(7 )|(bb71)>>(32 -7 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=((
bb71|~bb69)^bb70)+0x6d703ef3 +bbw[13 ];bb65=((bb65)<<(5 )|(bb65)>>(32 -5 ))+
bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb71|~bb65&bb69)+
0x7a6d76e9 +bbw[8 ];bb66=((bb66)<<(15 )|(bb66)>>(32 -15 ))+bb70;bb71=((
bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66&bb65|~bb66&bb71)+0x7a6d76e9 +
bbw[6 ];bb70=((bb70)<<(5 )|(bb70)>>(32 -5 ))+bb69;bb65=((bb65)<<(10 )|(
bb65)>>(32 -10 ));bb69+=(bb70&bb66|~bb70&bb65)+0x7a6d76e9 +bbw[4 ];bb69=(
(bb69)<<(8 )|(bb69)>>(32 -8 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));
bb71+=(bb69&bb70|~bb69&bb66)+0x7a6d76e9 +bbw[1 ];bb71=((bb71)<<(11 )|(
bb71)>>(32 -11 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&
bb69|~bb71&bb70)+0x7a6d76e9 +bbw[3 ];bb65=((bb65)<<(14 )|(bb65)>>(32 -14 ))+
bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb71|~bb65&bb69)+
0x7a6d76e9 +bbw[11 ];bb66=((bb66)<<(14 )|(bb66)>>(32 -14 ))+bb70;bb71=((
bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66&bb65|~bb66&bb71)+0x7a6d76e9 +
bbw[15 ];bb70=((bb70)<<(6 )|(bb70)>>(32 -6 ))+bb69;bb65=((bb65)<<(10 )|(
bb65)>>(32 -10 ));bb69+=(bb70&bb66|~bb70&bb65)+0x7a6d76e9 +bbw[0 ];bb69=(
(bb69)<<(14 )|(bb69)>>(32 -14 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));
bb71+=(bb69&bb70|~bb69&bb66)+0x7a6d76e9 +bbw[5 ];bb71=((bb71)<<(6 )|(
bb71)>>(32 -6 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&
bb69|~bb71&bb70)+0x7a6d76e9 +bbw[12 ];bb65=((bb65)<<(9 )|(bb65)>>(32 -9 ))+
bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb71|~bb65&bb69)+
0x7a6d76e9 +bbw[2 ];bb66=((bb66)<<(12 )|(bb66)>>(32 -12 ))+bb70;bb71=((
bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66&bb65|~bb66&bb71)+0x7a6d76e9 +
bbw[13 ];bb70=((bb70)<<(9 )|(bb70)>>(32 -9 ))+bb69;bb65=((bb65)<<(10 )|(
bb65)>>(32 -10 ));bb69+=(bb70&bb66|~bb70&bb65)+0x7a6d76e9 +bbw[9 ];bb69=(
(bb69)<<(12 )|(bb69)>>(32 -12 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));
bb71+=(bb69&bb70|~bb69&bb66)+0x7a6d76e9 +bbw[7 ];bb71=((bb71)<<(5 )|(
bb71)>>(32 -5 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71&
bb69|~bb71&bb70)+0x7a6d76e9 +bbw[10 ];bb65=((bb65)<<(15 )|(bb65)>>(32 -15
))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65&bb71|~bb65&
bb69)+0x7a6d76e9 +bbw[14 ];bb66=((bb66)<<(8 )|(bb66)>>(32 -8 ))+bb70;bb71=
((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66^bb65^bb71)+bbw[12 ];bb70=((
bb70)<<(8 )|(bb70)>>(32 -8 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));
bb69+=(bb70^bb66^bb65)+bbw[15 ];bb69=((bb69)<<(5 )|(bb69)>>(32 -5 ))+bb71
;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb71+=(bb69^bb70^bb66)+bbw[10 ];
bb71=((bb71)<<(12 )|(bb71)>>(32 -12 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(
32 -10 ));bb65+=(bb71^bb69^bb70)+bbw[4 ];bb65=((bb65)<<(9 )|(bb65)>>(32 -9
))+bb66;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb66+=(bb65^bb71^bb69)+
bbw[1 ];bb66=((bb66)<<(12 )|(bb66)>>(32 -12 ))+bb70;bb71=((bb71)<<(10 )|(
bb71)>>(32 -10 ));bb70+=(bb66^bb65^bb71)+bbw[5 ];bb70=((bb70)<<(5 )|(bb70
)>>(32 -5 ))+bb69;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=(bb70^bb66^
bb65)+bbw[8 ];bb69=((bb69)<<(14 )|(bb69)>>(32 -14 ))+bb71;bb66=((bb66)<<(
10 )|(bb66)>>(32 -10 ));bb71+=(bb69^bb70^bb66)+bbw[7 ];bb71=((bb71)<<(6 )|
(bb71)>>(32 -6 ))+bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71^
bb69^bb70)+bbw[6 ];bb65=((bb65)<<(8 )|(bb65)>>(32 -8 ))+bb66;bb69=((bb69)<<
(10 )|(bb69)>>(32 -10 ));bb66+=(bb65^bb71^bb69)+bbw[2 ];bb66=((bb66)<<(13
)|(bb66)>>(32 -13 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(
bb66^bb65^bb71)+bbw[13 ];bb70=((bb70)<<(6 )|(bb70)>>(32 -6 ))+bb69;bb65=(
(bb65)<<(10 )|(bb65)>>(32 -10 ));bb69+=(bb70^bb66^bb65)+bbw[14 ];bb69=((
bb69)<<(5 )|(bb69)>>(32 -5 ))+bb71;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));
bb71+=(bb69^bb70^bb66)+bbw[0 ];bb71=((bb71)<<(15 )|(bb71)>>(32 -15 ))+
bb65;bb70=((bb70)<<(10 )|(bb70)>>(32 -10 ));bb65+=(bb71^bb69^bb70)+bbw[3
];bb65=((bb65)<<(13 )|(bb65)>>(32 -13 ))+bb66;bb69=((bb69)<<(10 )|(bb69)>>
(32 -10 ));bb66+=(bb65^bb71^bb69)+bbw[9 ];bb66=((bb66)<<(11 )|(bb66)>>(32
-11 ))+bb70;bb71=((bb71)<<(10 )|(bb71)>>(32 -10 ));bb70+=(bb66^bb65^bb71)+
bbw[11 ];bb70=((bb70)<<(11 )|(bb70)>>(32 -11 ))+bb69;bb65=((bb65)<<(10 )|(
bb65)>>(32 -10 ));bb65+=bb23[1 ]+bb67;bb23[1 ]=bb23[2 ]+bb52+bb71;bb23[2 ]=
bb23[3 ]+bb68+bb69;bb23[3 ]=bb23[4 ]+bb64+bb70;bb23[4 ]=bb23[0 ]+bb58+bb66
;bb23[0 ]=bb65;}}bbb bb1843(bb463*bbi){bb40 bbd bb23[5 ]={0x67452301 ,
0xefcdab89 ,0x98badcfe ,0x10325476 ,0xc3d2e1f0 };bbi->bb5=0 ;bb74(bbi->
bb23,bb23,bb12(bb23));}bbb bb1338(bb463*bbi,bbh bbb*bb516,bbo bb5){
bbh bbf*bbx=(bbh bbf* )bb516;bbo bb398=bbi->bb5%bb12(bbi->bb105);bbi
->bb5+=bb5;bbm(bb398){bbo bb11=bb12(bbi->bb105)-bb398;bb74(bbi->bb105
+bb398,bbx,((bb5)<(bb11)?(bb5):(bb11)));bbm(bb5<bb11)bb4;bbx+=bb11;
bb5-=bb11;bb1298(bbi->bb23,bbi->bb105);}bb90(;bb5>=bb12(bbi->bb105);
bb5-=bb12(bbi->bb105),bbx+=bb12(bbi->bb105))bb1298(bbi->bb23,bbx);
bb74(bbi->bb105,bbx,bb5);}bbb bb1899(bb463*bbi,bbb*bb1){bbd bb1037[2 ]
={(bbd)(bbi->bb5<<3 ),(bbd)(bbi->bb5>>29 )};bbf bb437[bb12(bb1037)];bbo
bbz;bb90(bbz=0 ;bbz<bb12(bb437);bbz++)bb437[bbz]=bb1037[bbz/4 ]>>((bbz%
4 ) *8 )&0xff ;{bbf bb1352[]={0x80 },bb1353[bb12(bbi->bb105)]={0 };bbo
bb398=bbi->bb5%bb12(bbi->bb105);bb1338(bbi,bb1352,1 );bb1338(bbi,
bb1353,(bb12(bbi->bb105) *2 -1 -bb12(bb437)-bb398)%bb12(bbi->bb105));}
bb1338(bbi,bb437,bb12(bb437));bb90(bbz=0 ;bbz<bb12(bbi->bb23);bbz++)((
bbf* )bb1)[bbz]=bbi->bb23[bbz/4 ]>>((bbz%4 ) *8 )&0xff ;}bbb bb1979(bbb*
bb1,bbh bbb*bbx,bbo bb5){bb463 bb82;bb1843(&bb82);bb1338(&bb82,bbx,
bb5);bb1899(&bb82,bb1);}bbb bb2032(bbb*bb1,bb62 bbx){bb1979(bb1,bbx,(
bbo)bb1133(bbx));}
