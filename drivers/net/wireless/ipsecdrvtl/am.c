/*
   'md5.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
bb40 bbb bb1298(bbd bb23[4 ],bbh bbf bb98[64 ]){bb40 bbd bb6[64 ]={
0xd76aa478 ,0xe8c7b756 ,0x242070db ,0xc1bdceee ,0xf57c0faf ,0x4787c62a ,
0xa8304613 ,0xfd469501 ,0x698098d8 ,0x8b44f7af ,0xffff5bb1 ,0x895cd7be ,
0x6b901122 ,0xfd987193 ,0xa679438e ,0x49b40821 ,0xf61e2562 ,0xc040b340 ,
0x265e5a51 ,0xe9b6c7aa ,0xd62f105d ,0x02441453 ,0xd8a1e681 ,0xe7d3fbc8 ,
0x21e1cde6 ,0xc33707d6 ,0xf4d50d87 ,0x455a14ed ,0xa9e3e905 ,0xfcefa3f8 ,
0x676f02d9 ,0x8d2a4c8a ,0xfffa3942 ,0x8771f681 ,0x6d9d6122 ,0xfde5380c ,
0xa4beea44 ,0x4bdecfa9 ,0xf6bb4b60 ,0xbebfbc70 ,0x289b7ec6 ,0xeaa127fa ,
0xd4ef3085 ,0x04881d05 ,0xd9d4d039 ,0xe6db99e5 ,0x1fa27cf8 ,0xc4ac5665 ,
0xf4292244 ,0x432aff97 ,0xab9423a7 ,0xfc93a039 ,0x655b59c3 ,0x8f0ccc92 ,
0xffeff47d ,0x85845dd1 ,0x6fa87e4f ,0xfe2ce6e0 ,0xa3014314 ,0x4e0811a1 ,
0xf7537e82 ,0xbd3af235 ,0x2ad7d2bb ,0xeb86d391 ,};bb27(bb12(bbe)>=4 );{bbd
bbc=bb23[0 ],bbp=bb23[1 ],bbn=bb23[2 ],bbs=bb23[3 ],bb10;bbd bbw[16 ],bbz;
bb90(bbz=0 ;bbz<16 ;bbz++,bb98+=4 )bbw[bbz]=(bb98[0 ]|bb98[1 ]<<8 |bb98[2 ]
<<16 |bb98[3 ]<<24 );bb10=bbc+(bbp&bbn|~bbp&bbs)+bb6[0 ]+bbw[(0 )];bbc=bbp
+((bb10)<<(7 )|(bb10)>>(32 -7 ));bb10=bbs+(bbc&bbp|~bbc&bbn)+bb6[0 +1 ]+
bbw[(0 +1 )];bbs=bbc+((bb10)<<(12 )|(bb10)>>(32 -12 ));bb10=bbn+(bbs&bbc|~
bbs&bbp)+bb6[0 +2 ]+bbw[(0 +2 )];bbn=bbs+((bb10)<<(17 )|(bb10)>>(32 -17 ));
bb10=bbp+(bbn&bbs|~bbn&bbc)+bb6[0 +3 ]+bbw[(0 +3 )];bbp=bbn+((bb10)<<(22 )|
(bb10)>>(32 -22 ));bb10=bbc+(bbp&bbn|~bbp&bbs)+bb6[0 +4 ]+bbw[(0 +4 )];bbc=
bbp+((bb10)<<(7 )|(bb10)>>(32 -7 ));bb10=bbs+(bbc&bbp|~bbc&bbn)+bb6[0 +4 +
1 ]+bbw[(0 +4 +1 )];bbs=bbc+((bb10)<<(12 )|(bb10)>>(32 -12 ));bb10=bbn+(bbs&
bbc|~bbs&bbp)+bb6[0 +4 +2 ]+bbw[(0 +4 +2 )];bbn=bbs+((bb10)<<(17 )|(bb10)>>(
32 -17 ));bb10=bbp+(bbn&bbs|~bbn&bbc)+bb6[0 +4 +3 ]+bbw[(0 +4 +3 )];bbp=bbn+(
(bb10)<<(22 )|(bb10)>>(32 -22 ));bb10=bbc+(bbp&bbn|~bbp&bbs)+bb6[0 +8 ]+
bbw[(0 +8 )];bbc=bbp+((bb10)<<(7 )|(bb10)>>(32 -7 ));bb10=bbs+(bbc&bbp|~
bbc&bbn)+bb6[0 +8 +1 ]+bbw[(0 +8 +1 )];bbs=bbc+((bb10)<<(12 )|(bb10)>>(32 -12
));bb10=bbn+(bbs&bbc|~bbs&bbp)+bb6[0 +8 +2 ]+bbw[(0 +8 +2 )];bbn=bbs+((bb10
)<<(17 )|(bb10)>>(32 -17 ));bb10=bbp+(bbn&bbs|~bbn&bbc)+bb6[0 +8 +3 ]+bbw[(
0 +8 +3 )];bbp=bbn+((bb10)<<(22 )|(bb10)>>(32 -22 ));bb10=bbc+(bbp&bbn|~bbp
&bbs)+bb6[0 +12 ]+bbw[(0 +12 )];bbc=bbp+((bb10)<<(7 )|(bb10)>>(32 -7 ));bb10
=bbs+(bbc&bbp|~bbc&bbn)+bb6[0 +12 +1 ]+bbw[(0 +12 +1 )];bbs=bbc+((bb10)<<(
12 )|(bb10)>>(32 -12 ));bb10=bbn+(bbs&bbc|~bbs&bbp)+bb6[0 +12 +2 ]+bbw[(0 +
12 +2 )];bbn=bbs+((bb10)<<(17 )|(bb10)>>(32 -17 ));bb10=bbp+(bbn&bbs|~bbn&
bbc)+bb6[0 +12 +3 ]+bbw[(0 +12 +3 )];bbp=bbn+((bb10)<<(22 )|(bb10)>>(32 -22 ));
bb10=bbc+(bbs&bbp|~bbs&bbn)+bb6[16 ]+bbw[(5 * (16 )+1 )%16 ];bbc=bbp+((
bb10)<<(5 )|(bb10)>>(32 -5 ));bb10=bbs+(bbn&bbc|~bbn&bbp)+bb6[16 +1 ]+bbw[
(5 * (16 +1 )+1 )%16 ];bbs=bbc+((bb10)<<(9 )|(bb10)>>(32 -9 ));bb10=bbn+(bbp&
bbs|~bbp&bbc)+bb6[16 +2 ]+bbw[(5 * (16 +2 )+1 )%16 ];bbn=bbs+((bb10)<<(14 )|(
bb10)>>(32 -14 ));bb10=bbp+(bbc&bbn|~bbc&bbs)+bb6[16 +3 ]+bbw[(5 * (16 +3 )+
1 )%16 ];bbp=bbn+((bb10)<<(20 )|(bb10)>>(32 -20 ));bb10=bbc+(bbs&bbp|~bbs&
bbn)+bb6[16 +4 ]+bbw[(5 * (16 +4 )+1 )%16 ];bbc=bbp+((bb10)<<(5 )|(bb10)>>(32
-5 ));bb10=bbs+(bbn&bbc|~bbn&bbp)+bb6[16 +4 +1 ]+bbw[(5 * (16 +4 +1 )+1 )%16 ];
bbs=bbc+((bb10)<<(9 )|(bb10)>>(32 -9 ));bb10=bbn+(bbp&bbs|~bbp&bbc)+bb6[
16 +4 +2 ]+bbw[(5 * (16 +4 +2 )+1 )%16 ];bbn=bbs+((bb10)<<(14 )|(bb10)>>(32 -14 ));
bb10=bbp+(bbc&bbn|~bbc&bbs)+bb6[16 +4 +3 ]+bbw[(5 * (16 +4 +3 )+1 )%16 ];bbp=
bbn+((bb10)<<(20 )|(bb10)>>(32 -20 ));bb10=bbc+(bbs&bbp|~bbs&bbn)+bb6[16
+8 ]+bbw[(5 * (16 +8 )+1 )%16 ];bbc=bbp+((bb10)<<(5 )|(bb10)>>(32 -5 ));bb10=
bbs+(bbn&bbc|~bbn&bbp)+bb6[16 +8 +1 ]+bbw[(5 * (16 +8 +1 )+1 )%16 ];bbs=bbc+((
bb10)<<(9 )|(bb10)>>(32 -9 ));bb10=bbn+(bbp&bbs|~bbp&bbc)+bb6[16 +8 +2 ]+
bbw[(5 * (16 +8 +2 )+1 )%16 ];bbn=bbs+((bb10)<<(14 )|(bb10)>>(32 -14 ));bb10=
bbp+(bbc&bbn|~bbc&bbs)+bb6[16 +8 +3 ]+bbw[(5 * (16 +8 +3 )+1 )%16 ];bbp=bbn+((
bb10)<<(20 )|(bb10)>>(32 -20 ));bb10=bbc+(bbs&bbp|~bbs&bbn)+bb6[16 +12 ]+
bbw[(5 * (16 +12 )+1 )%16 ];bbc=bbp+((bb10)<<(5 )|(bb10)>>(32 -5 ));bb10=bbs+
(bbn&bbc|~bbn&bbp)+bb6[16 +12 +1 ]+bbw[(5 * (16 +12 +1 )+1 )%16 ];bbs=bbc+((
bb10)<<(9 )|(bb10)>>(32 -9 ));bb10=bbn+(bbp&bbs|~bbp&bbc)+bb6[16 +12 +2 ]+
bbw[(5 * (16 +12 +2 )+1 )%16 ];bbn=bbs+((bb10)<<(14 )|(bb10)>>(32 -14 ));bb10=
bbp+(bbc&bbn|~bbc&bbs)+bb6[16 +12 +3 ]+bbw[(5 * (16 +12 +3 )+1 )%16 ];bbp=bbn+
((bb10)<<(20 )|(bb10)>>(32 -20 ));bb10=bbc+(bbp^bbn^bbs)+bb6[32 ]+bbw[(3 *
(32 )+5 )%16 ];bbc=bbp+((bb10)<<(4 )|(bb10)>>(32 -4 ));bb10=bbs+(bbc^bbp^
bbn)+bb6[32 +1 ]+bbw[(3 * (32 +1 )+5 )%16 ];bbs=bbc+((bb10)<<(11 )|(bb10)>>(
32 -11 ));bb10=bbn+(bbs^bbc^bbp)+bb6[32 +2 ]+bbw[(3 * (32 +2 )+5 )%16 ];bbn=
bbs+((bb10)<<(16 )|(bb10)>>(32 -16 ));bb10=bbp+(bbn^bbs^bbc)+bb6[32 +3 ]+
bbw[(3 * (32 +3 )+5 )%16 ];bbp=bbn+((bb10)<<(23 )|(bb10)>>(32 -23 ));bb10=bbc
+(bbp^bbn^bbs)+bb6[32 +4 ]+bbw[(3 * (32 +4 )+5 )%16 ];bbc=bbp+((bb10)<<(4 )|(
bb10)>>(32 -4 ));bb10=bbs+(bbc^bbp^bbn)+bb6[32 +4 +1 ]+bbw[(3 * (32 +4 +1 )+5 )%
16 ];bbs=bbc+((bb10)<<(11 )|(bb10)>>(32 -11 ));bb10=bbn+(bbs^bbc^bbp)+bb6
[32 +4 +2 ]+bbw[(3 * (32 +4 +2 )+5 )%16 ];bbn=bbs+((bb10)<<(16 )|(bb10)>>(32 -16
));bb10=bbp+(bbn^bbs^bbc)+bb6[32 +4 +3 ]+bbw[(3 * (32 +4 +3 )+5 )%16 ];bbp=bbn
+((bb10)<<(23 )|(bb10)>>(32 -23 ));bb10=bbc+(bbp^bbn^bbs)+bb6[32 +8 ]+bbw[
(3 * (32 +8 )+5 )%16 ];bbc=bbp+((bb10)<<(4 )|(bb10)>>(32 -4 ));bb10=bbs+(bbc^
bbp^bbn)+bb6[32 +8 +1 ]+bbw[(3 * (32 +8 +1 )+5 )%16 ];bbs=bbc+((bb10)<<(11 )|(
bb10)>>(32 -11 ));bb10=bbn+(bbs^bbc^bbp)+bb6[32 +8 +2 ]+bbw[(3 * (32 +8 +2 )+5
)%16 ];bbn=bbs+((bb10)<<(16 )|(bb10)>>(32 -16 ));bb10=bbp+(bbn^bbs^bbc)+
bb6[32 +8 +3 ]+bbw[(3 * (32 +8 +3 )+5 )%16 ];bbp=bbn+((bb10)<<(23 )|(bb10)>>(32
-23 ));bb10=bbc+(bbp^bbn^bbs)+bb6[32 +12 ]+bbw[(3 * (32 +12 )+5 )%16 ];bbc=
bbp+((bb10)<<(4 )|(bb10)>>(32 -4 ));bb10=bbs+(bbc^bbp^bbn)+bb6[32 +12 +1 ]+
bbw[(3 * (32 +12 +1 )+5 )%16 ];bbs=bbc+((bb10)<<(11 )|(bb10)>>(32 -11 ));bb10=
bbn+(bbs^bbc^bbp)+bb6[32 +12 +2 ]+bbw[(3 * (32 +12 +2 )+5 )%16 ];bbn=bbs+((
bb10)<<(16 )|(bb10)>>(32 -16 ));bb10=bbp+(bbn^bbs^bbc)+bb6[32 +12 +3 ]+bbw[
(3 * (32 +12 +3 )+5 )%16 ];bbp=bbn+((bb10)<<(23 )|(bb10)>>(32 -23 ));bb10=bbc+
(bbn^(bbp|~bbs))+bb6[48 ]+bbw[(7 * (48 ))%16 ];bbc=bbp+((bb10)<<(6 )|(bb10
)>>(32 -6 ));bb10=bbs+(bbp^(bbc|~bbn))+bb6[48 +1 ]+bbw[(7 * (48 +1 ))%16 ];
bbs=bbc+((bb10)<<(10 )|(bb10)>>(32 -10 ));bb10=bbn+(bbc^(bbs|~bbp))+bb6[
48 +2 ]+bbw[(7 * (48 +2 ))%16 ];bbn=bbs+((bb10)<<(15 )|(bb10)>>(32 -15 ));bb10
=bbp+(bbs^(bbn|~bbc))+bb6[48 +3 ]+bbw[(7 * (48 +3 ))%16 ];bbp=bbn+((bb10)<<
(21 )|(bb10)>>(32 -21 ));bb10=bbc+(bbn^(bbp|~bbs))+bb6[48 +4 ]+bbw[(7 * (48
+4 ))%16 ];bbc=bbp+((bb10)<<(6 )|(bb10)>>(32 -6 ));bb10=bbs+(bbp^(bbc|~bbn
))+bb6[48 +4 +1 ]+bbw[(7 * (48 +4 +1 ))%16 ];bbs=bbc+((bb10)<<(10 )|(bb10)>>(
32 -10 ));bb10=bbn+(bbc^(bbs|~bbp))+bb6[48 +4 +2 ]+bbw[(7 * (48 +4 +2 ))%16 ];
bbn=bbs+((bb10)<<(15 )|(bb10)>>(32 -15 ));bb10=bbp+(bbs^(bbn|~bbc))+bb6[
48 +4 +3 ]+bbw[(7 * (48 +4 +3 ))%16 ];bbp=bbn+((bb10)<<(21 )|(bb10)>>(32 -21 ));
bb10=bbc+(bbn^(bbp|~bbs))+bb6[48 +8 ]+bbw[(7 * (48 +8 ))%16 ];bbc=bbp+((
bb10)<<(6 )|(bb10)>>(32 -6 ));bb10=bbs+(bbp^(bbc|~bbn))+bb6[48 +8 +1 ]+bbw[
(7 * (48 +8 +1 ))%16 ];bbs=bbc+((bb10)<<(10 )|(bb10)>>(32 -10 ));bb10=bbn+(
bbc^(bbs|~bbp))+bb6[48 +8 +2 ]+bbw[(7 * (48 +8 +2 ))%16 ];bbn=bbs+((bb10)<<(
15 )|(bb10)>>(32 -15 ));bb10=bbp+(bbs^(bbn|~bbc))+bb6[48 +8 +3 ]+bbw[(7 * (
48 +8 +3 ))%16 ];bbp=bbn+((bb10)<<(21 )|(bb10)>>(32 -21 ));bb10=bbc+(bbn^(
bbp|~bbs))+bb6[48 +12 ]+bbw[(7 * (48 +12 ))%16 ];bbc=bbp+((bb10)<<(6 )|(bb10
)>>(32 -6 ));bb10=bbs+(bbp^(bbc|~bbn))+bb6[48 +12 +1 ]+bbw[(7 * (48 +12 +1 ))%
16 ];bbs=bbc+((bb10)<<(10 )|(bb10)>>(32 -10 ));bb10=bbn+(bbc^(bbs|~bbp))+
bb6[48 +12 +2 ]+bbw[(7 * (48 +12 +2 ))%16 ];bbn=bbs+((bb10)<<(15 )|(bb10)>>(32
-15 ));bb10=bbp+(bbs^(bbn|~bbc))+bb6[48 +12 +3 ]+bbw[(7 * (48 +12 +3 ))%16 ];
bbp=bbn+((bb10)<<(21 )|(bb10)>>(32 -21 ));bb23[0 ]+=bbc;bb23[1 ]+=bbp;bb23
[2 ]+=bbn;bb23[3 ]+=bbs;}}bbb bb1862(bb458*bbi){bb40 bbd bb23[4 ]={
0x67452301 ,0xefcdab89 ,0x98badcfe ,0x10325476 };bbi->bb5=0 ;bb74(bbi->
bb23,bb23,bb12(bb23));}bbb bb1350(bb458*bbi,bbh bbb*bb516,bbo bb5){
bbh bbf*bbx=(bbh bbf* )bb516;bbo bb398=bbi->bb5%bb12(bbi->bb105);bbi
->bb5+=bb5;bbm(bb398){bbo bb11=bb12(bbi->bb105)-bb398;bb74(bbi->bb105
+bb398,bbx,((bb5)<(bb11)?(bb5):(bb11)));bbm(bb5<bb11)bb4;bbx+=bb11;
bb5-=bb11;bb1298(bbi->bb23,bbi->bb105);}bb90(;bb5>=bb12(bbi->bb105);
bb5-=bb12(bbi->bb105),bbx+=bb12(bbi->bb105))bb1298(bbi->bb23,bbx);
bb74(bbi->bb105,bbx,bb5);}bbb bb1867(bb458*bbi,bbb*bb1){bbd bb1037[2 ]
={(bbd)(bbi->bb5<<3 ),(bbd)(bbi->bb5>>29 )};bbf bb437[bb12(bb1037)];bbo
bbz;bb90(bbz=0 ;bbz<bb12(bb437);bbz++)bb437[bbz]=bb1037[bbz/4 ]>>((bbz%
4 ) *8 )&0xff ;{bbf bb1352[]={0x80 },bb1353[bb12(bbi->bb105)]={0 };bbo
bb398=bbi->bb5%bb12(bbi->bb105);bb1350(bbi,bb1352,1 );bb1350(bbi,
bb1353,(bb12(bbi->bb105) *2 -1 -bb12(bb437)-bb398)%bb12(bbi->bb105));}
bb1350(bbi,bb437,bb12(bb437));bb90(bbz=0 ;bbz<bb12(bbi->bb23);bbz++)((
bbf* )bb1)[bbz]=bbi->bb23[bbz/4 ]>>((bbz%4 ) *8 )&0xff ;}bbb bb1902(bbb*
bb1,bbh bbb*bbx,bbo bb5){bb458 bb82;bb1862(&bb82);bb1350(&bb82,bbx,
bb5);bb1867(&bb82,bb1);}bbb bb2023(bbb*bb1,bb62 bbx){bb1902(bb1,bbx,(
bbo)bb1133(bbx));}
