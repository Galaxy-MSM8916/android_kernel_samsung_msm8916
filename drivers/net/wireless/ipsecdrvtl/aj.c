/*
   'hmac.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
bba bbj{bbd bb5;bbd bb23[4 ];bbf bb105[64 ];}bb458;bbb bb1862(bb458*bbi
);bbb bb1350(bb458*bbi,bbh bbb*bb516,bbo bb5);bbb bb1867(bb458*bbi,
bbb*bb1);bbb bb1902(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2023(bbb*bb1,
bb62 bbx);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbj{bbd bb5;bbd bb23[5 ];bbf bb105[64 ];}bb459;bbb bb1898(bb459*bbi
);bbb bb1331(bb459*bbi,bbh bbb*bbx,bbo bb5);bbb bb1844(bb459*bbi,bbb*
bb1);bba bbj{bbd bb5;bbd bb23[8 ];bbf bb105[64 ];}bb412;bbb bb1865(
bb412*bbi);bbb bb1277(bb412*bbi,bbh bbb*bbx,bbo bb5);bbb bb1860(bb412
 *bbi,bbb*bb1);bba bb412 bb965;bbb bb1962(bb965*bbi);bbb bb1913(bb965
 *bbi,bbb*bb1);bba bbj{bbd bb5;bb57 bb23[8 ];bbf bb105[128 ];}bb315;bbb
bb1851(bb315*bbi);bbb bb1065(bb315*bbi,bbh bbb*bbx,bbo bb5);bbb bb1885
(bb315*bbi,bbb*bb1);bba bb315 bb626;bbb bb1837(bb626*bbi);bbb bb1855(
bb626*bbi,bbb*bb1);bba bb315 bb978;bbb bb1838(bb978*bbi);bbb bb1897(
bb978*bbi,bbb*bb1);bba bb315 bb961;bbb bb1854(bb961*bbi);bbb bb1845(
bb961*bbi,bbb*bb1);bbb bb1955(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb1916
(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2020(bbb*bb1,bbh bbb*bbx,bbo bb5);
bbb bb1983(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb1967(bbb*bb1,bbh bbb*
bbx,bbo bb5);bbb bb1987(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2080(bbb*
bb1,bb62 bbx);bbb bb2022(bbb*bb1,bb62 bbx);bbb bb2091(bbb*bb1,bb62 bbx
);bbb bb2082(bbb*bb1,bb62 bbx);bbb bb2057(bbb*bb1,bb62 bbx);bbb bb2087
(bbb*bb1,bb62 bbx);
#ifdef __cplusplus
}
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
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbj{bbd bb5;bbd bb23[5 ];bbf bb105[64 ];}bb462;bbb bb1847(bb462*bbi
);bbb bb1391(bb462*bbi,bbh bbb*bb516,bbo bb5);bbb bb1886(bb462*bbi,
bbb*bb1);bbb bb1922(bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2063(bbb*bb1,
bb62 bbx);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbb( *bb969)(bbb*bbi);bba bbb( *bb572)(bbb*bbi,bbh bbb*bbx,bbo bb5
);bba bbb( *bb576)(bbb*bbi,bbb*bb1);bba bbj{bbe bb131;bbo bb33;bbo
bb366;bb969 bb585;bb572 bb333;bb576 bb577;}bb631;bbb bb2021(bb631*bbi
,bbe bb131);bba bbj{bb631 bbp;bb329{bb458 bb991;bb459 bb981;bb412
bb956;bb315 bb987;bb463 bb1866;bb462 bb963;}bbs;}bb467;bbb bb2045(
bb467*bbi,bbe bb131);bbb bb2052(bb467*bbi);bbb bb2085(bb467*bbi,bbe
bb131);bbb bb2043(bb467*bbi,bbh bbb*bbx,bbo bb5);bbb bb2038(bb467*bbi
,bbb*bb1);bbb bb2050(bbe bb131,bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2114
(bbe bb131,bbb*bb1,bb62 bbx);bb62 bb2029(bbe bb131);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbj{bb631 bbp;bb329{bb458 bb991;bb459 bb981;bb412 bb956;bb315
bb987;bb463 bb1866;bb462 bb963;}bb557,bb1379;}bb482;bbb bb2100(bb482*
bbi,bbe bb418);bbb bb2068(bb482*bbi,bbh bbb*bb30,bbo bb100);bbb bb2175
(bb482*bbi,bbe bb418,bbh bbb*bb30,bbo bb100);bbb bb2048(bb482*bbi,bbh
bbb*bbx,bbo bb5);bbb bb2070(bb482*bbi,bbb*bb1);bbb bb2171(bbe bb418,
bbh bbb*bb30,bbo bb100,bbb*bb1,bbh bbb*bbx,bbo bb5);bbb bb2265(bbe
bb418,bb62 bb30,bbb*bb1,bb62 bbx);
#ifdef __cplusplus
}
#endif
bbb bb2100(bb482*bbi,bbe bb418){bb2021(&bbi->bbp,bb418);}bbb bb2068(
bb482*bbi,bbh bbb*bb488,bbo bb100){bb631 bbp=bbi->bbp;bbb*bb557=&bbi
->bb557, *bb1379=&bbi->bb1379;bbh bbf*bb30=(bbh bbf* )bb488;bbf bb2343
[256 ],bb1301[256 ];bbp.bb585(bb557);bbm(bb100>bbp.bb33){bbp.bb333(
bb557,bb30,bb100);bb27(bbp.bb366<=bb12(bb2343));bbp.bb577(bb557,
bb2343);bb30=bb2343;bb100=bbp.bb366;bb27(bb100<=bbp.bb33);}{bbo bbz;
bb27(bbp.bb33<=bb12(bb1301));bb90(bbz=0 ;bbz<bbp.bb33;bbz++)bb1301[bbz
]=0x36 ^(bbz<bb100?bb30[bbz]:0 );bbp.bb585(bb557);bbp.bb333(bb557,
bb1301,bbp.bb33);}{bbo bbz;bb90(bbz=0 ;bbz<bbp.bb33;bbz++)bb1301[bbz]=
0x5c ^(bbz<bb100?bb30[bbz]:0 );bbp.bb585(bb1379);bbp.bb333(bb1379,
bb1301,bbp.bb33);}}bbb bb2175(bb482*bbi,bbe bb418,bbh bbb*bb30,bbo
bb100){bb2100(bbi,bb418);bb2068(bbi,bb30,bb100);}bbb bb2048(bb482*bbi
,bbh bbb*bbx,bbo bb5){bbi->bbp.bb333(&bbi->bb557,bbx,bb5);}bbb bb2070
(bb482*bbi,bbb*bb1){bb631 bbp=bbi->bbp;bbb*bb557=&bbi->bb557, *bb1379
=&bbi->bb1379;bbp.bb577(bb557,bb1);bbp.bb333(bb1379,bb1,bbp.bb366);
bbp.bb577(bb1379,bb1);}bbb bb2171(bbe bb418,bbh bbb*bb30,bbo bb100,
bbb*bb1,bbh bbb*bbx,bbo bb5){bb482 bb82;bb2175(&bb82,bb418,bb30,bb100
);bb2048(&bb82,bbx,bb5);bb2070(&bb82,bb1);}bbb bb2265(bbe bb418,bb62
bb30,bbb*bb1,bb62 bbx){bb2171(bb418,bb30,(bbo)bb1133(bb30),bb1,bbx,(
bbo)bb1133(bbx));}
