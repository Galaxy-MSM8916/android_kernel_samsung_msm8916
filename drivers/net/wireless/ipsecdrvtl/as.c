/*
   'src_compress_deflate_adler32.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
#ifdef bb533
#define bb292 8
#else
#define bb292 9
#endif
#ifdef bb139
#define bbq( bb421) bb421
#else
#define bbq( bb421) ()
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
bb1122 bbq((bb16 bb15,bbe bb466,bbh bbl*bb195,bbe bb196));bbj bb390{
bbe bb460;};bbr bbh bbl*bb1209 bbq((bbe bb18));bbr bbe bb1214 bbq((
bb16 bb0));bbr bbh bb167*bb1204 bbq((bbb));
#ifdef __cplusplus
}
#endif
bb25 bb1018(bb377,bb42,bb22)bb25 bb377;bbh bb34*bb42;bb9 bb22;{bbd
bb198=bb377&0xffff ;bbd bb363=(bb377>>16 )&0xffff ;bbe bb6;bbm(bb42==0 )bb4
1L ;bb109(bb22>0 ){bb6=bb22<5552 ?bb22:5552 ;bb22-=bb6;bb109(bb6>=16 ){{
bb198+=bb42[0 ];bb363+=bb198;};{bb198+=bb42[0 +1 ];bb363+=bb198;};;{
bb198+=bb42[0 +2 ];bb363+=bb198;};{bb198+=bb42[0 +2 +1 ];bb363+=bb198;};;;
{bb198+=bb42[0 +4 ];bb363+=bb198;};{bb198+=bb42[0 +4 +1 ];bb363+=bb198;};;
{bb198+=bb42[0 +4 +2 ];bb363+=bb198;};{bb198+=bb42[0 +4 +2 +1 ];bb363+=bb198
;};;;;{bb198+=bb42[8 ];bb363+=bb198;};{bb198+=bb42[8 +1 ];bb363+=bb198;}
;;{bb198+=bb42[8 +2 ];bb363+=bb198;};{bb198+=bb42[8 +2 +1 ];bb363+=bb198;}
;;;{bb198+=bb42[8 +4 ];bb363+=bb198;};{bb198+=bb42[8 +4 +1 ];bb363+=bb198;
};;{bb198+=bb42[8 +4 +2 ];bb363+=bb198;};{bb198+=bb42[8 +4 +2 +1 ];bb363+=
bb198;};;;;;bb42+=16 ;bb6-=16 ;}bbm(bb6!=0 )bb599{bb198+= *bb42++;bb363
+=bb198;}bb109(--bb6);bb198%=65521L ;bb363%=65521L ;}bb4(bb363<<16 )|
bb198;}
