/*
   'aria.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
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
bba bbj{bbo bb432;bbd bb367[4 * (16 +1 )];}bb385;bbb bb1261(bb385*bbi,
bbh bbb*bb30,bbo bb100);bbb bb1827(bb385*bbi,bbh bbb*bb30,bbo bb100);
bbb bb1141(bb385*bbi,bbb*bb1,bbh bbb*bbx);
#ifdef __cplusplus
}
#endif
bb40 bbh bbf bb1926[256 ]={0x63 ,0x7c ,0x77 ,0x7b ,0xf2 ,0x6b ,0x6f ,0xc5 ,
0x30 ,0x01 ,0x67 ,0x2b ,0xfe ,0xd7 ,0xab ,0x76 ,0xca ,0x82 ,0xc9 ,0x7d ,0xfa ,0x59
,0x47 ,0xf0 ,0xad ,0xd4 ,0xa2 ,0xaf ,0x9c ,0xa4 ,0x72 ,0xc0 ,0xb7 ,0xfd ,0x93 ,
0x26 ,0x36 ,0x3f ,0xf7 ,0xcc ,0x34 ,0xa5 ,0xe5 ,0xf1 ,0x71 ,0xd8 ,0x31 ,0x15 ,0x04
,0xc7 ,0x23 ,0xc3 ,0x18 ,0x96 ,0x05 ,0x9a ,0x07 ,0x12 ,0x80 ,0xe2 ,0xeb ,0x27 ,
0xb2 ,0x75 ,0x09 ,0x83 ,0x2c ,0x1a ,0x1b ,0x6e ,0x5a ,0xa0 ,0x52 ,0x3b ,0xd6 ,0xb3
,0x29 ,0xe3 ,0x2f ,0x84 ,0x53 ,0xd1 ,0x00 ,0xed ,0x20 ,0xfc ,0xb1 ,0x5b ,0x6a ,
0xcb ,0xbe ,0x39 ,0x4a ,0x4c ,0x58 ,0xcf ,0xd0 ,0xef ,0xaa ,0xfb ,0x43 ,0x4d ,0x33
,0x85 ,0x45 ,0xf9 ,0x02 ,0x7f ,0x50 ,0x3c ,0x9f ,0xa8 ,0x51 ,0xa3 ,0x40 ,0x8f ,
0x92 ,0x9d ,0x38 ,0xf5 ,0xbc ,0xb6 ,0xda ,0x21 ,0x10 ,0xff ,0xf3 ,0xd2 ,0xcd ,0x0c
,0x13 ,0xec ,0x5f ,0x97 ,0x44 ,0x17 ,0xc4 ,0xa7 ,0x7e ,0x3d ,0x64 ,0x5d ,0x19 ,
0x73 ,0x60 ,0x81 ,0x4f ,0xdc ,0x22 ,0x2a ,0x90 ,0x88 ,0x46 ,0xee ,0xb8 ,0x14 ,0xde
,0x5e ,0x0b ,0xdb ,0xe0 ,0x32 ,0x3a ,0x0a ,0x49 ,0x06 ,0x24 ,0x5c ,0xc2 ,0xd3 ,
0xac ,0x62 ,0x91 ,0x95 ,0xe4 ,0x79 ,0xe7 ,0xc8 ,0x37 ,0x6d ,0x8d ,0xd5 ,0x4e ,0xa9
,0x6c ,0x56 ,0xf4 ,0xea ,0x65 ,0x7a ,0xae ,0x08 ,0xba ,0x78 ,0x25 ,0x2e ,0x1c ,
0xa6 ,0xb4 ,0xc6 ,0xe8 ,0xdd ,0x74 ,0x1f ,0x4b ,0xbd ,0x8b ,0x8a ,0x70 ,0x3e ,0xb5
,0x66 ,0x48 ,0x03 ,0xf6 ,0x0e ,0x61 ,0x35 ,0x57 ,0xb9 ,0x86 ,0xc1 ,0x1d ,0x9e ,
0xe1 ,0xf8 ,0x98 ,0x11 ,0x69 ,0xd9 ,0x8e ,0x94 ,0x9b ,0x1e ,0x87 ,0xe9 ,0xce ,0x55
,0x28 ,0xdf ,0x8c ,0xa1 ,0x89 ,0x0d ,0xbf ,0xe6 ,0x42 ,0x68 ,0x41 ,0x99 ,0x2d ,
0x0f ,0xb0 ,0x54 ,0xbb ,0x16 };bb40 bbh bbf bb1925[256 ]={0xe2 ,0x4e ,0x54 ,
0xfc ,0x94 ,0xc2 ,0x4a ,0xcc ,0x62 ,0x0d ,0x6a ,0x46 ,0x3c ,0x4d ,0x8b ,0xd1 ,0x5e
,0xfa ,0x64 ,0xcb ,0xb4 ,0x97 ,0xbe ,0x2b ,0xbc ,0x77 ,0x2e ,0x03 ,0xd3 ,0x19 ,
0x59 ,0xc1 ,0x1d ,0x06 ,0x41 ,0x6b ,0x55 ,0xf0 ,0x99 ,0x69 ,0xea ,0x9c ,0x18 ,0xae
,0x63 ,0xdf ,0xe7 ,0xbb ,0x00 ,0x73 ,0x66 ,0xfb ,0x96 ,0x4c ,0x85 ,0xe4 ,0x3a ,
0x09 ,0x45 ,0xaa ,0x0f ,0xee ,0x10 ,0xeb ,0x2d ,0x7f ,0xf4 ,0x29 ,0xac ,0xcf ,0xad
,0x91 ,0x8d ,0x78 ,0xc8 ,0x95 ,0xf9 ,0x2f ,0xce ,0xcd ,0x08 ,0x7a ,0x88 ,0x38 ,
0x5c ,0x83 ,0x2a ,0x28 ,0x47 ,0xdb ,0xb8 ,0xc7 ,0x93 ,0xa4 ,0x12 ,0x53 ,0xff ,0x87
,0x0e ,0x31 ,0x36 ,0x21 ,0x58 ,0x48 ,0x01 ,0x8e ,0x37 ,0x74 ,0x32 ,0xca ,0xe9 ,
0xb1 ,0xb7 ,0xab ,0x0c ,0xd7 ,0xc4 ,0x56 ,0x42 ,0x26 ,0x07 ,0x98 ,0x60 ,0xd9 ,0xb6
,0xb9 ,0x11 ,0x40 ,0xec ,0x20 ,0x8c ,0xbd ,0xa0 ,0xc9 ,0x84 ,0x04 ,0x49 ,0x23 ,
0xf1 ,0x4f ,0x50 ,0x1f ,0x13 ,0xdc ,0xd8 ,0xc0 ,0x9e ,0x57 ,0xe3 ,0xc3 ,0x7b ,0x65
,0x3b ,0x02 ,0x8f ,0x3e ,0xe8 ,0x25 ,0x92 ,0xe5 ,0x15 ,0xdd ,0xfd ,0x17 ,0xa9 ,
0xbf ,0xd4 ,0x9a ,0x7e ,0xc5 ,0x39 ,0x67 ,0xfe ,0x76 ,0x9d ,0x43 ,0xa7 ,0xe1 ,0xd0
,0xf5 ,0x68 ,0xf2 ,0x1b ,0x34 ,0x70 ,0x05 ,0xa3 ,0x8a ,0xd5 ,0x79 ,0x86 ,0xa8 ,
0x30 ,0xc6 ,0x51 ,0x4b ,0x1e ,0xa6 ,0x27 ,0xf6 ,0x35 ,0xd2 ,0x6e ,0x24 ,0x16 ,0x82
,0x5f ,0xda ,0xe6 ,0x75 ,0xa2 ,0xef ,0x2c ,0xb2 ,0x1c ,0x9f ,0x5d ,0x6f ,0x80 ,
0x0a ,0x72 ,0x44 ,0x9b ,0x6c ,0x90 ,0x0b ,0x5b ,0x33 ,0x7d ,0x5a ,0x52 ,0xf3 ,0x61
,0xa1 ,0xf7 ,0xb0 ,0xd6 ,0x3f ,0x7c ,0x6d ,0xed ,0x14 ,0xe0 ,0xa5 ,0x3d ,0x22 ,
0xb3 ,0xf8 ,0x89 ,0xde ,0x71 ,0x1a ,0xaf ,0xba ,0xb5 ,0x81 };bb40 bbf bb2163[
256 ],bb2162[256 ];bb40 bbd bb687[256 ],bb686[256 ];bb40 bbd bb864[256 ],
bb772[256 ];bb40 bbb bb2120(){bbo bbz;bb90(bbz=0 ;bbz<256 ;bbz++){bbo
bb2176=bb1926[bbz],bb1884=bb1925[bbz];bb2163[bb2176]=bbz;bb2162[
bb1884]=bbz;bb687[bbz]=bb2176<<16 |bb2176<<8 |bb2176;bb686[bbz]=bb1884
<<24 |bb1884<<8 |bb1884;bb864[bb2176]=bbz<<24 |bbz<<16 |bbz;bb772[bb1884]
=bbz<<24 |bbz<<16 |bbz<<8 ;}}bbb bb1261(bb385*bbi,bbh bbb*bb30,bbo bb100
){bbo bb1286,bb432,bbz,bb77;bb31 bb6=bbi->bb367;bbd bbw[4 ][4 ];bb40 bbu
bb1880=1 ;bbm(bb1880){bb2120();bb1880=0 ;}bb27(bb100==16 ||bb100==24 ||
bb100==32 );bb1286=bb100/4 ;bbi->bb432=bb432=bb1286+8 ;bb90(bbz=0 ;bbz<4 ;
bbz++){bbm(bbz==0 )bb960(bbw[0 ],0 ,16 );bb50{bb40 bbd bb2634[3 ][4 ]={{
0x517cc1b7 ,0x27220a94 ,0xfe13abe8 ,0xfa9a6ee0 },{0x6db14acc ,0x9e21c820 ,
0xff28b1d5 ,0xef5de2b0 },{0xdb92371d ,0x2126e970 ,0x03249775 ,0x04e8c90e }}
;bb31 bb110=bbw[bbz],bb1372=bbw[bbz-1 ],bbn=bb2634[(bbz-1 +(bb1286-4 )/2
)%3 ];bbm(bbz%2 ==1 ){bb110[0 ]=bb1372[0 ]^bbn[0 ];bb110[1 ]=bb1372[1 ]^bbn[1
];bb110[2 ]=bb1372[2 ]^bbn[2 ];bb110[3 ]=bb1372[3 ]^bbn[3 ];bb110[0 ]=bb687[
bb110[0 ]>>24 ]^bb686[bb110[0 ]>>16 &0xff ]^bb864[bb110[0 ]>>8 &0xff ]^bb772[
bb110[0 ]&0xff ];bb110[1 ]=bb687[bb110[1 ]>>24 ]^bb686[bb110[1 ]>>16 &0xff ]^
bb864[bb110[1 ]>>8 &0xff ]^bb772[bb110[1 ]&0xff ];bb110[2 ]=bb687[bb110[2 ]
>>24 ]^bb686[bb110[2 ]>>16 &0xff ]^bb864[bb110[2 ]>>8 &0xff ]^bb772[bb110[2 ]
&0xff ];bb110[3 ]=bb687[bb110[3 ]>>24 ]^bb686[bb110[3 ]>>16 &0xff ]^bb864[
bb110[3 ]>>8 &0xff ]^bb772[bb110[3 ]&0xff ];bb110[1 ]^=bb110[2 ];bb110[2 ]^=
bb110[3 ];bb110[0 ]^=bb110[1 ];bb110[3 ]^=bb110[1 ];bb110[2 ]^=bb110[0 ];
bb110[1 ]^=bb110[2 ];bb110[1 ]=(bb110[1 ]&0xff00ff )<<8 |bb110[1 ]>>8 &
0xff00ff ;bb110[2 ]=bb110[2 ]<<16 |bb110[2 ]>>16 ;bb110[3 ]=bb110[3 ]<<24 |(
bb110[3 ]&0xff00 )<<8 |bb110[3 ]>>8 &0xff00 |bb110[3 ]>>24 ;bb110[1 ]^=bb110[2
];bb110[2 ]^=bb110[3 ];bb110[0 ]^=bb110[1 ];bb110[3 ]^=bb110[1 ];bb110[2 ]^=
bb110[0 ];bb110[1 ]^=bb110[2 ];}bb50{bb110[0 ]=bb1372[0 ]^bbn[0 ];bb110[1 ]=
bb1372[1 ]^bbn[1 ];bb110[2 ]=bb1372[2 ]^bbn[2 ];bb110[3 ]=bb1372[3 ]^bbn[3 ];
bb110[0 ]=bb864[bb110[0 ]>>24 ]^bb772[bb110[0 ]>>16 &0xff ]^bb687[bb110[0 ]
>>8 &0xff ]^bb686[bb110[0 ]&0xff ];bb110[1 ]=bb864[bb110[1 ]>>24 ]^bb772[
bb110[1 ]>>16 &0xff ]^bb687[bb110[1 ]>>8 &0xff ]^bb686[bb110[1 ]&0xff ];bb110
[2 ]=bb864[bb110[2 ]>>24 ]^bb772[bb110[2 ]>>16 &0xff ]^bb687[bb110[2 ]>>8 &
0xff ]^bb686[bb110[2 ]&0xff ];bb110[3 ]=bb864[bb110[3 ]>>24 ]^bb772[bb110[3
]>>16 &0xff ]^bb687[bb110[3 ]>>8 &0xff ]^bb686[bb110[3 ]&0xff ];bb110[1 ]^=
bb110[2 ];bb110[2 ]^=bb110[3 ];bb110[0 ]^=bb110[1 ];bb110[3 ]^=bb110[1 ];
bb110[2 ]^=bb110[0 ];bb110[1 ]^=bb110[2 ];bb110[3 ]=(bb110[3 ]&0xff00ff )<<8
|bb110[3 ]>>8 &0xff00ff ;bb110[0 ]=bb110[0 ]<<16 |bb110[0 ]>>16 ;bb110[1 ]=
bb110[1 ]<<24 |(bb110[1 ]&0xff00 )<<8 |bb110[1 ]>>8 &0xff00 |bb110[1 ]>>24 ;
bb110[1 ]^=bb110[2 ];bb110[2 ]^=bb110[3 ];bb110[0 ]^=bb110[1 ];bb110[3 ]^=
bb110[1 ];bb110[2 ]^=bb110[0 ];bb110[1 ]^=bb110[2 ];}}bb90(bb77=0 ;bb77<4 ;
bb77++){bbo bb178=4 *bbz+bb77;bbw[bbz][bb77]^=(bbz<2 ?(bb178<bb1286?(
bbd)(((bb3)((bb31)bb30+bb178))[3 ]|((bb3)((bb31)bb30+bb178))[2 ]<<8 |((
bb3)((bb31)bb30+bb178))[1 ]<<16 |((bb3)((bb31)bb30+bb178))[0 ]<<24 ):0U ):
bbw[bbz-2 ][bb77]);}}bb90(bbz=0 ;bbz<=bb432;bbz++){bb90(bb77=0 ;bb77<4 ;
bb77++){bb40 bbo bbg[]={19 ,31 ,67 ,97 ,109 };bbo bb178=128 -bbg[bbz/4 ];
bb31 bb110=bbw[bbz%4 ],bb1372=bbw[(bbz+1 )%4 ]; *bb6++=bb110[bb77]^
bb1372[(bb77+bb178/32 )%4 ]<<(bb178%32 )^bb1372[(bb77+bb178/32 +1 )%4 ]>>(
32 -bb178%32 );}}}bbb bb1827(bb385*bbi,bbh bbb*bb30,bbo bb100){bb385 bbv
;bb31 bb6=bbi->bb367;bbo bbz;bb1261(&bbv,bb30,bb100);bbi->bb432=bbv.
bb432;bb90(bbz=0 ;bbz<=bbv.bb432;bbz++){bb74(bb6+4 *bbz,bbv.bb367+4 * (
bbv.bb432-bbz),16 );}bb90(bbz=1 ;bbz<bbv.bb432;bbz++){bb6+=4 ;bb6[0 ]=bb6
[0 ]<<8 ^bb6[0 ]>>8 ^bb6[0 ]<<16 ^bb6[0 ]>>16 ^bb6[0 ]<<24 ^bb6[0 ]>>24 ;bb6[1 ]=
bb6[1 ]<<8 ^bb6[1 ]>>8 ^bb6[1 ]<<16 ^bb6[1 ]>>16 ^bb6[1 ]<<24 ^bb6[1 ]>>24 ;bb6[2
]=bb6[2 ]<<8 ^bb6[2 ]>>8 ^bb6[2 ]<<16 ^bb6[2 ]>>16 ^bb6[2 ]<<24 ^bb6[2 ]>>24 ;bb6
[3 ]=bb6[3 ]<<8 ^bb6[3 ]>>8 ^bb6[3 ]<<16 ^bb6[3 ]>>16 ^bb6[3 ]<<24 ^bb6[3 ]>>24 ;
bb6[1 ]^=bb6[2 ];bb6[2 ]^=bb6[3 ];bb6[0 ]^=bb6[1 ];bb6[3 ]^=bb6[1 ];bb6[2 ]^=
bb6[0 ];bb6[1 ]^=bb6[2 ];bb6[1 ]=(bb6[1 ]&0xff00ff )<<8 |bb6[1 ]>>8 &0xff00ff ;
bb6[2 ]=bb6[2 ]<<16 |bb6[2 ]>>16 ;bb6[3 ]=bb6[3 ]<<24 |(bb6[3 ]&0xff00 )<<8 |bb6
[3 ]>>8 &0xff00 |bb6[3 ]>>24 ;bb6[1 ]^=bb6[2 ];bb6[2 ]^=bb6[3 ];bb6[0 ]^=bb6[1 ]
;bb6[3 ]^=bb6[1 ];bb6[2 ]^=bb6[0 ];bb6[1 ]^=bb6[2 ];}}bbb bb1141(bb385*bbi,
bbb*bb1,bbh bbb*bbx){bbd bbg[4 ];bb31 bb6=bbi->bb367;bbo bb432=bbi->
bb432,bbz;bbg[0 ]=(bbd)(((bb3)((bb31)bbx))[3 ]|((bb3)((bb31)bbx))[2 ]<<8
|((bb3)((bb31)bbx))[1 ]<<16 |((bb3)((bb31)bbx))[0 ]<<24 );bbg[1 ]=(bbd)(((
bb3)((bb31)bbx+1 ))[3 ]|((bb3)((bb31)bbx+1 ))[2 ]<<8 |((bb3)((bb31)bbx+1 ))[
1 ]<<16 |((bb3)((bb31)bbx+1 ))[0 ]<<24 );bbg[2 ]=(bbd)(((bb3)((bb31)bbx+2 ))[
3 ]|((bb3)((bb31)bbx+2 ))[2 ]<<8 |((bb3)((bb31)bbx+2 ))[1 ]<<16 |((bb3)((
bb31)bbx+2 ))[0 ]<<24 );bbg[3 ]=(bbd)(((bb3)((bb31)bbx+3 ))[3 ]|((bb3)((
bb31)bbx+3 ))[2 ]<<8 |((bb3)((bb31)bbx+3 ))[1 ]<<16 |((bb3)((bb31)bbx+3 ))[0
]<<24 );bb90(bbz=2 ;bbz<bb432;bbz+=2 ){bbg[0 ]=bbg[0 ]^bb6[0 ];bbg[1 ]=bbg[1
]^bb6[1 ];bbg[2 ]=bbg[2 ]^bb6[2 ];bbg[3 ]=bbg[3 ]^bb6[3 ];bbg[0 ]=bb687[bbg[0
]>>24 ]^bb686[bbg[0 ]>>16 &0xff ]^bb864[bbg[0 ]>>8 &0xff ]^bb772[bbg[0 ]&0xff
];bbg[1 ]=bb687[bbg[1 ]>>24 ]^bb686[bbg[1 ]>>16 &0xff ]^bb864[bbg[1 ]>>8 &
0xff ]^bb772[bbg[1 ]&0xff ];bbg[2 ]=bb687[bbg[2 ]>>24 ]^bb686[bbg[2 ]>>16 &
0xff ]^bb864[bbg[2 ]>>8 &0xff ]^bb772[bbg[2 ]&0xff ];bbg[3 ]=bb687[bbg[3 ]>>
24 ]^bb686[bbg[3 ]>>16 &0xff ]^bb864[bbg[3 ]>>8 &0xff ]^bb772[bbg[3 ]&0xff ];
bbg[1 ]^=bbg[2 ];bbg[2 ]^=bbg[3 ];bbg[0 ]^=bbg[1 ];bbg[3 ]^=bbg[1 ];bbg[2 ]^=
bbg[0 ];bbg[1 ]^=bbg[2 ];bbg[1 ]=(bbg[1 ]&0xff00ff )<<8 |bbg[1 ]>>8 &0xff00ff ;
bbg[2 ]=bbg[2 ]<<16 |bbg[2 ]>>16 ;bbg[3 ]=bbg[3 ]<<24 |(bbg[3 ]&0xff00 )<<8 |bbg
[3 ]>>8 &0xff00 |bbg[3 ]>>24 ;bbg[1 ]^=bbg[2 ];bbg[2 ]^=bbg[3 ];bbg[0 ]^=bbg[1 ]
;bbg[3 ]^=bbg[1 ];bbg[2 ]^=bbg[0 ];bbg[1 ]^=bbg[2 ];bb6+=4 ;bbg[0 ]=bbg[0 ]^
bb6[0 ];bbg[1 ]=bbg[1 ]^bb6[1 ];bbg[2 ]=bbg[2 ]^bb6[2 ];bbg[3 ]=bbg[3 ]^bb6[3 ]
;bbg[0 ]=bb864[bbg[0 ]>>24 ]^bb772[bbg[0 ]>>16 &0xff ]^bb687[bbg[0 ]>>8 &0xff
]^bb686[bbg[0 ]&0xff ];bbg[1 ]=bb864[bbg[1 ]>>24 ]^bb772[bbg[1 ]>>16 &0xff ]^
bb687[bbg[1 ]>>8 &0xff ]^bb686[bbg[1 ]&0xff ];bbg[2 ]=bb864[bbg[2 ]>>24 ]^
bb772[bbg[2 ]>>16 &0xff ]^bb687[bbg[2 ]>>8 &0xff ]^bb686[bbg[2 ]&0xff ];bbg[3
]=bb864[bbg[3 ]>>24 ]^bb772[bbg[3 ]>>16 &0xff ]^bb687[bbg[3 ]>>8 &0xff ]^
bb686[bbg[3 ]&0xff ];bbg[1 ]^=bbg[2 ];bbg[2 ]^=bbg[3 ];bbg[0 ]^=bbg[1 ];bbg[3
]^=bbg[1 ];bbg[2 ]^=bbg[0 ];bbg[1 ]^=bbg[2 ];bbg[3 ]=(bbg[3 ]&0xff00ff )<<8 |
bbg[3 ]>>8 &0xff00ff ;bbg[0 ]=bbg[0 ]<<16 |bbg[0 ]>>16 ;bbg[1 ]=bbg[1 ]<<24 |(
bbg[1 ]&0xff00 )<<8 |bbg[1 ]>>8 &0xff00 |bbg[1 ]>>24 ;bbg[1 ]^=bbg[2 ];bbg[2 ]^=
bbg[3 ];bbg[0 ]^=bbg[1 ];bbg[3 ]^=bbg[1 ];bbg[2 ]^=bbg[0 ];bbg[1 ]^=bbg[2 ];
bb6+=4 ;}bbg[0 ]=bbg[0 ]^bb6[0 ];bbg[1 ]=bbg[1 ]^bb6[1 ];bbg[2 ]=bbg[2 ]^bb6[2
];bbg[3 ]=bbg[3 ]^bb6[3 ];bbg[0 ]=bb687[bbg[0 ]>>24 ]^bb686[bbg[0 ]>>16 &0xff
]^bb864[bbg[0 ]>>8 &0xff ]^bb772[bbg[0 ]&0xff ];bbg[1 ]=bb687[bbg[1 ]>>24 ]^
bb686[bbg[1 ]>>16 &0xff ]^bb864[bbg[1 ]>>8 &0xff ]^bb772[bbg[1 ]&0xff ];bbg[2
]=bb687[bbg[2 ]>>24 ]^bb686[bbg[2 ]>>16 &0xff ]^bb864[bbg[2 ]>>8 &0xff ]^
bb772[bbg[2 ]&0xff ];bbg[3 ]=bb687[bbg[3 ]>>24 ]^bb686[bbg[3 ]>>16 &0xff ]^
bb864[bbg[3 ]>>8 &0xff ]^bb772[bbg[3 ]&0xff ];bbg[1 ]^=bbg[2 ];bbg[2 ]^=bbg[3
];bbg[0 ]^=bbg[1 ];bbg[3 ]^=bbg[1 ];bbg[2 ]^=bbg[0 ];bbg[1 ]^=bbg[2 ];bbg[1 ]=
(bbg[1 ]&0xff00ff )<<8 |bbg[1 ]>>8 &0xff00ff ;bbg[2 ]=bbg[2 ]<<16 |bbg[2 ]>>16 ;
bbg[3 ]=bbg[3 ]<<24 |(bbg[3 ]&0xff00 )<<8 |bbg[3 ]>>8 &0xff00 |bbg[3 ]>>24 ;bbg[
1 ]^=bbg[2 ];bbg[2 ]^=bbg[3 ];bbg[0 ]^=bbg[1 ];bbg[3 ]^=bbg[1 ];bbg[2 ]^=bbg[0
];bbg[1 ]^=bbg[2 ];bb6+=4 ;bbg[0 ]^=bb6[0 ];bbg[1 ]^=bb6[1 ];bbg[2 ]^=bb6[2 ];
bbg[3 ]^=bb6[3 ];bb6+=4 ;bbg[0 ]=bb2163[bbg[0 ]>>24 ]<<24 ^bb2162[bbg[0 ]>>16
&0xff ]<<16 ^bb1926[bbg[0 ]>>8 &0xff ]<<8 ^bb1925[bbg[0 ]&0xff ]^bb6[0 ];bbg[1
]=bb2163[bbg[1 ]>>24 ]<<24 ^bb2162[bbg[1 ]>>16 &0xff ]<<16 ^bb1926[bbg[1 ]>>8
&0xff ]<<8 ^bb1925[bbg[1 ]&0xff ]^bb6[1 ];bbg[2 ]=bb2163[bbg[2 ]>>24 ]<<24 ^
bb2162[bbg[2 ]>>16 &0xff ]<<16 ^bb1926[bbg[2 ]>>8 &0xff ]<<8 ^bb1925[bbg[2 ]&
0xff ]^bb6[2 ];bbg[3 ]=bb2163[bbg[3 ]>>24 ]<<24 ^bb2162[bbg[3 ]>>16 &0xff ]<<
16 ^bb1926[bbg[3 ]>>8 &0xff ]<<8 ^bb1925[bbg[3 ]&0xff ]^bb6[3 ];((bb31)bb1)[0
]=(bbd)(((bb3)(bbg))[3 ]|((bb3)(bbg))[2 ]<<8 |((bb3)(bbg))[1 ]<<16 |((bb3)(
bbg))[0 ]<<24 );((bb31)bb1)[1 ]=(bbd)(((bb3)(bbg+1 ))[3 ]|((bb3)(bbg+1 ))[2
]<<8 |((bb3)(bbg+1 ))[1 ]<<16 |((bb3)(bbg+1 ))[0 ]<<24 );((bb31)bb1)[2 ]=(bbd
)(((bb3)(bbg+2 ))[3 ]|((bb3)(bbg+2 ))[2 ]<<8 |((bb3)(bbg+2 ))[1 ]<<16 |((bb3)(
bbg+2 ))[0 ]<<24 );((bb31)bb1)[3 ]=(bbd)(((bb3)(bbg+3 ))[3 ]|((bb3)(bbg+3 ))[
2 ]<<8 |((bb3)(bbg+3 ))[1 ]<<16 |((bb3)(bbg+3 ))[0 ]<<24 );}
