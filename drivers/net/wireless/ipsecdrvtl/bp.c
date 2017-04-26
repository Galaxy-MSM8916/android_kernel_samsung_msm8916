/*
   'src_nf_nf.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Dec 22 18:00:49 2014
*/
#include<linux/version.h>
#include<linux/netdevice.h>
#include<linux/netfilter.h>
#include<linux/netfilter_ipv4.h>
#include<linux/skbuff.h>
#include<linux/ip.h>
#include<net/ip.h>
#include<linux/udp.h>
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
#include"uncobf.h"
#include<linux/miscdevice.h>
#include<linux/fs.h>
#include<asm/uaccess.h>
#include"cobf.h"
bba bb13{bb423,bb1524,}bb306;bbk bb1235(bb306 bb881,bbh bbf*bb464);
bbd bb553(bb306 bb881,bbh bbf*bb464);bbb bb1213(bbk bb158,bb306 bb584
,bbf bb452[2 ]);bbb bb1006(bbd bb158,bb306 bb584,bbf bb452[4 ]);bba bb85
bb7;bb13{bb101=0 ,bb372=-12000 ,bb365=-11999 ,bb392=-11998 ,bb689=-11997 ,
bb727=-11996 ,bb767=-11995 ,bb911=-11994 ,bb793=-11992 ,bb804=-11991 ,
bb896=-11990 ,bb761=-11989 ,bb825=-11988 ,bb657=-11987 ,bb685=-11986 ,
bb902=-11985 ,bb885=-11984 ,bb644=-11983 ,bb665=-11982 ,bb796=-11981 ,
bb910=-11980 ,bb737=-11979 ,bb858=-11978 ,bb862=-11977 ,bb610=-11976 ,
bb870=-11975 ,bb678=-11960 ,bb930=-11959 ,bb912=-11500 ,bb748=-11499 ,
bb868=-11498 ,bb808=-11497 ,bb709=-11496 ,bb770=-11495 ,bb802=-11494 ,
bb783=-11493 ,bb882=-11492 ,bb893=-11491 ,bb779=-11490 ,bb866=-11489 ,
bb692=-11488 ,bb859=-11487 ,bb884=-11486 ,bb645=-11485 ,bb656=-11484 ,
bb713=-11483 ,bb845=-11482 ,bb690=-11481 ,bb717=-11480 ,bb712=-11479 ,
bb720=-11478 ,bb735=-11477 ,bb853=-11476 ,bb843=-11475 ,bb873=-11474 ,
bb835=-11473 ,bb874=-11472 ,bb806=-11460 ,bb854=-11450 ,bb738=-11449 ,
bb722=-11448 ,bb747=-11447 ,bb871=-11446 ,bb676=-11445 ,bb723=-11444 ,
bb819=-11443 ,bb721=-11440 ,bb817=-11439 ,bb842=-11438 ,bb803=-11437 ,
bb691=-11436 ,bb684=-11435 ,bb903=-11420 ,bb548=-11419 ,bb588=-11418 ,
bb700=-11417 ,bb914=-11416 ,bb680=-11415 ,bb805=-11414 ,bb742=-11413 ,
bb888=-11412 ,bb865=-11411 ,bb693=-11410 ,bb927=-11409 ,bb905=-11408 ,
bb714=-11407 ,bb739=-11406 ,bb900=-11405 ,bb894=-11404 ,bb683=-11403 ,
bb766=-11402 ,bb774=-11401 ,bb773=-11400 ,bb890=-11399 ,bb716=-11398 ,
bb771=-11397 ,bb699=-11396 ,bb799=-11395 ,bb758=-11394 ,bb919=-11393 ,
bb834=-11392 ,bb920=-11391 ,bb895=-11390 ,bb741=-11389 ,bb667=-11388 ,
bb876=-11387 ,bb908=-11386 ,bb759=-11385 ,bb883=-11384 ,bb904=-11383 ,
bb846=-11382 ,bb658=-11381 ,bb838=-11380 ,bb790=-11379 ,bb928=-11378 ,
bb762=-11377 ,bb831=-11376 ,bb801=-11375 ,bb849=-11374 ,bb855=-11373 ,
bb702=-11372 ,bb906=-11371 ,bb653=-11370 ,bb784=-11369 ,bb867=-11368 ,
bb768=-11367 ,bb812=-11366 ,bb679=-11365 ,bb869=-11364 ,bb659=-11363 ,
bb401=-11350 ,bb892=bb401,bb729=-11349 ,bb701=-11348 ,bb787=-11347 ,bb740
=-11346 ,bb837=-11345 ,bb705=-11344 ,bb852=-11343 ,bb840=-11342 ,bb731=-
11341 ,bb734=-11340 ,bb907=-11339 ,bb406=-11338 ,bb821=-11337 ,bb688=bb406
,bb822=-11330 ,bb844=-11329 ,bb832=-11328 ,bb776=-11327 ,bb733=-11326 ,
bb661=-11325 ,bb918=-11324 ,bb725=-11320 ,bb931=-11319 ,bb763=-11318 ,
bb788=-11317 ,bb652=-11316 ,bb922=-11315 ,bb792=-11314 ,bb769=-11313 ,
bb782=-11312 ,bb789=-11300 ,bb778=-11299 ,bb807=-11298 ,bb719=-11297 ,
bb743=-11296 ,bb673=-11295 ,bb861=-11294 ,bb646=-11293 ,bb797=-11292 ,
bb901=-11291 ,bb857=-11290 ,bb649=-11289 ,bb785=-11288 ,bb879=-11287 ,
bb824=-11286 ,bb647=-11285 ,bb851=-11284 ,bb764=-11283 ,bb750=-11282 ,
bb695=-11281 ,bb833=-11280 ,bb829=-11279 ,bb751=-11250 ,bb798=-11249 ,
bb899=-11248 ,bb755=-11247 ,bb791=-11246 ,bb711=-11245 ,bb823=-11244 ,
bb715=-11243 ,bb925=-11242 ,bb860=-11240 ,bb662=-11239 ,bb745=-11238 ,
bb795=-11237 ,bb863=-11150 ,bb728=-11100 ,bb756=-11099 ,bb781=-11098 ,
bb726=-11097 ,bb730=-11096 ,bb800=-11095 ,bb923=-11094 ,bb875=-11093 ,
bb827=-11092 ,bb698=-11091 ,bb655=-11090 ,bb706=-11089 ,bb651=-11088 ,
bb926=-11087 ,bb878=-11086 ,bb836=-11085 ,bb696=-11050 ,bb753=-11049 ,
bb708=-10999 ,bb809=-10998 ,bb677=-10997 ,bb718=-10996 ,bb666=-10995 ,
bb697=-10994 ,bb710=-10993 ,bb650=-10992 ,bb777=-10991 ,bb668=-10990 ,
bb786=-10989 ,bb913=-10988 ,bb841=-10979 ,bb664=-10978 ,bb932=-10977 ,
bb887=-10976 ,bb760=-10975 ,bb724=-10974 ,};bba bbj bb469{bb3 bb76;bbd
bb129;bbd bb183;bbj bb469*bb99;}bby;bb7 bb489(bby*bb839,bbd bb933,bby
 *bb847,bbd bb877,bbd bb558);bb7 bb551(bby*bbi,bbd bb97,bbh bbb*bb98,
bbd bb48);bb7 bb597(bby*bbi,bbd bb97,bbb*bb132,bbd bb48);bbu bb813(
bby*bbi,bbd bb97,bbh bbb*bb98,bbd bb48);
#define bb977 bb56(0x0800)
#define bb1131 bb56(0x0806)
#define bb958 bb56(0x01f4)
#define bb975 bb56(0x1194)
#define bb1160 bb56(0x4000)
#define bb1134 bb56(0x2000)
#define bb1146 bb56(0x1FFF)
#define bb1088( bb10) (( bb10) & bb56(0x2000 | 0x1FFF))
#define bb1061( bb10) ((( bb197( bb10)) & 0x1FFF) << 3)
#define bb1013( bb10) ((( bb10) & bb56(0x1FFF)) == 0)
#define bb513( bb10) (( bb10) & bb56(0x2000))
#define bb1069( bb10) (!( bb513( bb10)))
#pragma pack(push, 1)
bba bbj{bbf bb376[6 ];bbf bb1044[6 ];bbk bb383;}bb374, *bb389;bba bbj{
bbf bb460[6 ];bbk bb383;}bb1117, *bb1126;bba bbj{bbf bb994:4 ;bbf bb1121
:4 ;bbf bb1085;bbk bb381;bbk bb909;bbk bb603;bbf bb1038;bbf bb291;bbk
bb634;bbd bb312;bbd bb256;}bb330, *bb324;bba bbj{bbk bb1071;bbk bb1078
;bbf bb1073;bbf bb1080;bbk bb1096;bbf bb1093[6 ];bbd bb1075;bbf bb1070
[6 ];bbd bb1098;}bb1090, *bb1115;
#pragma pack(pop)
bba bbj{bbk bb290;bbk bb439;bbk bb1043;bbk bb326;}bb431, *bb362;bba
bbj{bbk bb290;bbk bb621;bbd bb565;bbd bb948;bbf bb97;bbf bb171;bbk
bb159;bbk bb326;bbk bb1042;}bb508, *bb317;bba bbj{bbf bb1113;bbf
bb1103;bbf bb1092;bbf bb1109;bbd bb1097;bbk bb1107;bbk bb387;bbd
bb1127;bbd bb1116;bbd bb1100;bbd bb1095;bbf bb1114[16 ];bbf bb1084[64 ]
;bbf bb26[128 ];bbf bb1128[64 ];}bb1083, *bb1079;bba bbj{bbd bb312;bbd
bb256;bbf bb934;bbf bb291;bbk bb937;}bb627, *bb589;
#if defined( _WIN32)
#define bb56( bbc) (((( bbc) & 0XFF00) >> 8) | ((( bbc) & 0X00FF) <<  \
8))
#define bb197( bbc) ( bb56( bbc))
#define bb457( bbc) (((( bbc) & 0XFF000000) >> 24) | ((( bbc) &  \
0X00FF0000) >> 8) | ((( bbc) & 0X0000FF00) << 8) | ((( bbc) &  \
0X000000FF) << 24))
#define bb514( bbc) ( bb457( bbc))
#endif
bbk bb942(bbh bbb*bb304);bbk bb660(bbh bbb*bb528,bbe bb22);bb7 bb614(
bby*bb88,bbf bb104,bby*bb60);bb7 bb654(bby*bb88,bbu bb179,bbf*bb419);
bb7 bb982(bby*bb60,bbf*bb399);bb7 bb962(bbh bbf*bb399,bby*bb60);bb7
bb564(bby*bb53,bbf bb104,bbd*bb968);bb7 bb946(bby*bb88,bbf bb104,bbf
bb419,bby*bb60);bbd bb536(bby*bb53);bbk bb554(bby*bb53);bbb bb549(bbk
bb152,bby*bb53);bbb bb556(bby*bb53);bbb bb1009(bby*bb53,bbd*bb29);bbb
bb1035(bby*bb53,bbd*bb29);bbb bb1060(bby*bb53,bbd bb29);bbb bb954(bby
 *bb53,bbd bb29);bbb bb1016(bby*bb53);bbu bb1051(bbf*bb53);bba bbj
bb1025*bb1023;bba bbj bb1027*bb1064;bba bbj bb1026*bb1059;bba bbj
bb1067*bb1048;bba bbj bb1049*bb1033;bba bbj bb1024*bb1063;bba bb13{
bb579=0 ,bb607=1 ,bb609=2 ,bb814=3 ,bb612=4 ,bb601=5 ,bb586=6 ,bb593=7 ,bb604
=9 ,}bb438;bba bb13{bb630=0 ,bb1028,bb623,bb1046,bb955,bb941,bb947,
bb936,bb949,bb945,bb940,}bb537;bb7 bb2122(bby*bb302,bbd*bb106);bb7
bb2184(bby*bb88,bbu bb179,bbd bb490,bb537 bb298,bbh bbf*bb1349,bbf*
bb92,bb438 bb428,bbf*bb580,bbd bb106,bbd bb517,bby*bb60);bb7 bb2108(
bby*bb88,bbu bb179,bb537 bb298,bbh bbf*bb1349,bb438 bb428,bbf*bb580,
bbd*bb494,bbd*bb476,bbd*bb568,bby*bb60);bb7 bb2138(bby*bb302,bbd*
bb106);bb7 bb2156(bby*bb88,bbu bb179,bbd bb490,bb438 bb428,bbf*bb580,
bbd bb106,bbd bb517,bby*bb60);bb7 bb2128(bby*bb88,bbu bb179,bb438
bb428,bbf*bb580,bbd*bb494,bbd*bb476,bbd*bb568,bby*bb60);bb13{bb571=1 ,
};bbb*bb518(bbd bb1264,bbd bb387);bb7 bb474(bbb*bb1004);bba bbj bb2049
bb2031, *bb396;bba bb13{bb2058=0 ,bb1790=1 ,bb1807=2 }bb898;bb7 bb1859(
bb898 bb1923,bb396*bb369);bb7 bb2002(bb396 bb369,bbf*bb450,bbd bb434,
bbf*bb314,bbd bb295,bbd*bb453,bbd*bb310);bb7 bb1996(bb396 bb369,bbf*
bb314,bbd bb295,bbd*bb310,bbu*bb1007);bb7 bb2005(bb396 bb369,bbf*
bb450,bbd bb434,bbf*bb314,bbd bb295,bbd*bb453,bbd*bb310,bbu*bb997);
bb7 bb1869(bb396 bb369);bb7 bb2261(bby*bb88,bbu bb179,bbd bb490,bb898
bb1399,bby*bb60,bbu*bb2155);bb7 bb2167(bby*bb88,bbu bb179,bb898 bb1399
,bby*bb60);bbu bb2207(bby*bb302);bbu bb2266(bby*bb302);bb7 bb2127(bby
 *bb302,bbd*bb106);bb7 bb2074(bby*bb302,bbd*bb106);bb7 bb2015(bby*
bb88,bby*bb60,bbu bb1123,bbk bb2186,bbk bb1857);bb7 bb1911(bby*bb88,
bby*bb60,bbu bb1123);
#pragma pack(push, 8)
#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif
bba bbf bb180[4 ];bba bb13{bb1665=0 ,bb1489=1 ,}bb1412;bba bb13{bb1548=0
,bb1740=1 ,bb1683=2 ,bb1454=3 ,bb1421=4 ,bb1516=5 ,bb1652=6 ,bb1539=7 ,
bb1627=8 ,bb1542=9 ,bb1690=10 ,bb1530=11 ,bb1512=12 ,bb1579=13 ,bb1732=14 ,
bb1446=15 ,bb1476=16 ,bb1422=17 ,bb1622=18 ,bb1702=19 ,bb1660=20 ,bb1520=21
,bb1528=22 ,bb1496=23 ,bb1625=24 ,bb1620=25 ,bb1472=26 ,bb1555=27 ,bb1750=
28 ,bb1601=29 ,bb1704=30 ,bb1649=16300 ,bb1628=16301 ,bb1745=16384 ,bb1562=
24576 ,bb1482=24577 ,bb1460=24578 ,bb1501=34793 ,bb1758=40500 ,}bb780;bba
bb13{bb1484=0 ,bb1547=1 ,bb1477=2 ,bb1447=3 ,bb1718=4 ,bb1413=5 ,bb1498=6 ,
bb1497=7 ,bb1554=8 ,bb1423=9 ,bb1462=21 ,bb1513=22 ,bb1537=23 ,bb1464=24 ,
bb1566=25 ,bb1529=26 ,bb1485=27 ,bb1756=28 ,bb1495=29 ,bb1509=80 ,}bb828;
bba bb13{bb1653=0 ,bb1712=1 ,bb1709=2 ,bb1506=3 ,bb1541=4 ,}bb1643;bba bb13
{bb1703=0 ,bb1378=1 ,bb1203=2 ,bb1258=3 ,bb1402=4 ,bb1106=61440 ,bb1397=
61441 ,bb1153=61443 ,bb1341=61444 ,}bb498;bba bb13{bb1719=0 ,bb1518=1 ,
bb1584=2 ,}bb1697;bba bb13{bb1415=0 ,bb1744,bb1453,bb1473,bb1588,bb1519
,bb1654,bb1488,bb1626,bb1511,bb1414,bb1716,}bb736;bba bb13{bb1465=0 ,
bb1403=2 ,bb1366=3 ,bb1593=4 ,bb1361=9 ,bb1339=12 ,bb1400=13 ,bb1357=14 ,
bb1388=249 ,}bb929;bba bb13{bb1398=0 ,bb1342=1 ,bb1404=2 ,bb1445=3 ,bb1646
=4 ,bb1396=5 ,bb1381=12 ,bb1360=13 ,bb1355=14 ,bb1407=61440 ,}bb501;bba bb13
{bb1334=1 ,bb1347=2 ,bb1351=3 ,bb1564=4 ,bb1624=5 ,bb1468=6 ,bb1450=7 ,
bb1491=8 ,bb1471=9 ,bb1563=10 ,bb1345=11 ,bb413=12 ,bb1335=13 ,bb405=240 ,
bb1385=(128 <<16 )|bb405,bb1382=(192 <<16 )|bb405,bb1373=(256 <<16 )|bb405,
bb1344=(128 <<16 )|bb413,bb1337=(192 <<16 )|bb413,bb1371=(256 <<16 )|bb413,
}bb921;bba bb13{bb1336=0 ,bb1525=1 ,bb1405=2 ,bb1368=3 ,bb1480=4 ,}bb643;
bba bb13{bb1458=0 ,bb1597=1 ,bb1225=2 ,bb639=3 ,bb1280=4 ,}bb732;bba bb13{
bb1599=0 ,bb1551=1 ,bb1425=2 ,bb1684=5 ,bb1725=7 ,}bb492;bba bb13{bb1448=0
,bb1538=1 ,bb1623=2 ,bb1726=3 ,bb1494=4 ,bb1700=5 ,bb1657=6 ,bb409=7 ,bb1570
=65001 ,bb403=240 ,bb1508=(128 <<16 )|bb403,bb1526=(192 <<16 )|bb403,bb1533
=(256 <<16 )|bb403,bb1569=(128 <<16 )|bb409,bb1581=(192 <<16 )|bb409,bb1634
=(256 <<16 )|bb409,}bb818;bba bb13{bb1738=0 ,bb1478=1 ,bb1676=2 ,bb1596=3 ,
bb1493=4 ,bb1550=5 ,bb1589=6 ,bb1662=65001 ,}bb674;bba bb13{bb1642=0 ,
bb1549=1 ,bb1675=2 ,bb1576=3 ,bb1671=4 ,bb1633=5 ,bb1578=64221 ,bb1638=
64222 ,bb1674=64223 ,bb1688=64224 ,bb1730=65001 ,bb1699=65002 ,bb1573=
65003 ,bb1461=65004 ,bb1741=65005 ,bb1510=65006 ,bb1534=65007 ,bb1500=
65008 ,bb1723=65009 ,bb1499=65010 ,}bb703;bba bb13{bb1710=0 ,bb1439=1 ,
bb1455=2 ,}bb682;bba bb13{bb1434=0 ,bb1751=1 ,bb1502=2 ,bb1701=3 ,}bb815;
bba bb13{bb1613=0 ,bb1444=1 ,bb1457=2 ,bb1666=3 ,bb1619=4 ,bb1659=5 ,bb1515
=21 ,bb1592=6 ,bb1636=7 ,bb1558=8 ,bb1752=1000 ,}bb507;bba bb13{bb1435=0 ,
bb1606=1 ,bb1681=2 ,}bb746;bba bb13{bb1682=0 ,bb1426=1 ,bb1731=2 ,bb1459=3
,bb1492=4 ,}bb694;bba bb13{bb1557=0 ,bb1689=1 ,bb1717=1001 ,bb1733=1002 ,}
bb648;bba bb13{bb1583=0 ,bb1130=1 ,bb1108=2 ,bb1105=3 ,bb1169=4 ,bb1180=5 ,
bb1178=6 ,bb1713=100 ,bb1602=101 ,}bb504;bba bbj bb400{bb921 bb298;bb501
bb617;bb498 bb45;}bb400;bba bbj bb402{bb929 bb1384;bb501 bb617;bb498
bb45;}bb402;bba bbj bb408{bb643 bb1040;}bb408;bba bbj bb506{bb703
bb1647;bb674 bb428;bb818 bb298;bbu bb1507;bb492 bb663;}bb506;bba bbj
bb497{bbu bb638;bb400 bb316;bbu bb675;bb402 bb582;bbu bb810;bb408
bb632;bb492 bb663;}bb497;bba bbj bb470{bb180 bb984;bb180 bb1241;bb732
bb104;bb329{bbj{bb402 bb47;bbf bb581[64 ];bbf bb583[64 ];}bb582;bbj{
bb400 bb47;bbf bb1249[32 ];bbf bb1263[32 ];bbf bb581[64 ];bbf bb583[64 ];
bbf bb1232[16 ];}bb316;bbj{bb408 bb47;}bb632;}bb299;}bb470;bba bbj{bbd
bb889,bb620;bbf bb1173:1 ;bbf bb1208:1 ;bbf bb104;bbk bb455;}bb188;bba
bbj bb525{bbd bb11;bb188 bbc[64 *2 ];}bb525;
#ifdef UNDER_CE
bba bb43 bb394;
#else
bba bb85 bb394;
#endif
bba bbj bb174{bbj bb174*bb1487, *bb1760;bbd bb29;bbd bb1151;bb188
bb939[64 ];bb504 bb523;bbd bb1394;bbk bb1104;bbd bb574;bbd bb880;bbd
bb820;bbf bb499;bbf bb1375;bbf bb1142;bbd bb1068;bbd bb1759;bb394
bb600;bbk bb1295;bb470 bb417[3 ];bb394 bb1590;bbf bb1527[40 ];bbd bb613
;bbd bb1600;}bb174;
#ifdef CONFIG_COMPAT
#include"uncobf.h"
#include<linux/compat.h>
#include"cobf.h"
#define bb1369 ( bb12( bbj bb174  *  )  *  2 - bb12( bb500)  *  2)
#endif
bba bbj bb404{bbj bb404*bb1739;bb188 bb493;}bb404;bba bbj bb757{bbu
bb496;bbu bb499;bbd bb29;bbd bb613;bbf bb1535;bbk bb1615;bbf*bb1565;
bbd bb1443;bbf*bb1522;bbd bb1735;bbf*bb1754;bbd bb1433;bbu bb1663;bbu
bb1595;bb404*bb132;bbu bb1544;bb694 bb1545;bbd bb1614;bb682 bb1727;
bb504 bb523;bbk bb1747;bbd bb1553;bb648 bb1424;bbd bb1667;bbd bb1673;
bb736 bb1440;bbf*bb1427;bbd bb1430;bb507 bb924;bbd bb1670;bbd bb1641;
bbd bb1428;bbd bb1721;bbd bb1517;bb506*bb1552;bbd bb1481;bb497*bb1531
;bbd bb1420;bbd bb1556;bbd bb1668;}bb757;bba bbj bb816{bbu bb496;bbd
bb29;bb188 bb493;}bb816;bba bbj bb886{bb174*bb323;bbu bb1591;bbf*
bb1715;bbd bb1729;}bb886;bba bbj bb707{bbd bb29;bb188 bb493;bbf bb1456
;bbf bb1469;}bb707;bba bbj bb917{bbu bb496;bbu bb1155;bbd bb29;bbf*
bb1644;bbd bb1567;}bb917;bba bbj bb671{bbd bb29;bbk bb1714;bbk bb1748
;bbd bb152;bbf*bb51;}bb671;bba bbj bb749{bbu bb1609;bbd bb29;bbd bb574
;bbd bb880;bbd bb820;}bb749;bba bbj bb891{bb780 bb1514;bbd bb29;bb828
bb1362;bbu bb1580;}bb891;bba bbj bb856{bbf bb1505;bbf bb1416;bbf
bb1711;bbf bb1608;bbf bb1694;bbf bb1612;bbf bb951;bbf bb1479;bbf
bb1749;bbf bb1543;bbf bb1438;bbf bb981;bbf bb991;bbf bb956;bbf bb1692
;bbf bb987;bbf bb963;bbf bb1762;bbf bb1470;bbf bb529;bbf bb1574;bbf
bb1678;bbf bb1559;bbf bb1707;bbf bb1436;bbf bb1452;bbf bb1441;}bb856;
bba bbj bb752{bbu bb1677;bbd bb490;bbd bb1705;bb815 bb1449;bbk bb1651
;bbu bb1536;bbu bb1585;bbu bb1669;bbu bb1474;bbu bb1650;bbu bb1616;
bbu bb1417;bbl bb1640[128 ];bbl bb1685[128 ];bbl bb1611[128 ];bbl bb1687
[256 ];bbl bb1655[128 ];bbl bb1467[128 ];bbd bb1610;bbf bb1586[8 ];bbf
bb1432[8 ];}bb752;bba bbj bb669{bbd bb29;bbd bb1411;}bb669;bba bbj
bb848{bbd bb29;bbu bb499;}bb848;bba bbj bb915{bbu bb1734;bbd bb528;
bbd bb1201;}bb915;bba bbj bb765{bbd bb29;bb507 bb924;bb746 bb1621;bbf
 *bb1572;bbd bb1757;}bb765;bba bb13{bb1419=0 ,bb1577,bb1686,bb1763,
bb1635,bb1560,bb1617,bb1418,bb1561,bb1604,bb1546,bb1706,bb1724,bb1672
,bb1429,bb1607,bb1486,bb1431,bb1639,bb1658,}bb681;bba bbj bb1664 bb672
;bba bb7( *bb1575)(bb672*bb1753,bbb*bb1720,bb681 bb327,bbb*bb76);
#pragma pack(pop)
#ifdef _WIN32
#ifdef UNDER_CE
#define bb481 bb1722 bb636("1:")
#else
#define bb481 bb636("\\\\.\\IPSecTL")
#endif
#else
#define bb641 "ipsecdrvtl"
#define bb481 "/dev/" bb641
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
#include"uncobf.h"
#include<linux/ioctl.h>
#include"cobf.h"
bba bbj{bb3 bb602;bbd bb592;bb3 bb573;bbd bb545;bbd bb371;}bb1195;
#ifdef CONFIG_COMPAT
#include"uncobf.h"
#include<linux/compat.h>
#include"cobf.h"
bba bbj{bb500 bb602;bbd bb592;bb500 bb573;bbd bb545;bbd bb371;}bb1291
;
#endif
#define bb1323 1
#endif
#pragma pack(push, 8)
bb13{bb1395=3 ,bb1393,bb1392,bb1442,};bba bbj{bbf bb106[4 ];}bb1284;bba
bbj{bbf bb106[4 ];}bb1279;bba bbj{bbd bb973;bbd bb29;}bb1313;bba bbj{
bbd bb131;bbf bb1260[8 ];}bb420;bba bb13{bb1254=0 ,bb1276,bb1294,bb1328
,bb1746}bb1275;bba bbj{bbf bb1157;bbd bb1110;bbf bb1401;}bb503;
#pragma pack(pop)
#pragma pack(push, 8)
bb13{bb1163=-5000 ,bb1143=-4000 ,bb1032=-4999 ,bb1062=-4998 ,bb1050=-4997
,bb1015=-4996 ,bb1184=-4995 ,bb1120=-4994 ,bb1140=-4993 ,bb1052=-4992 ,
bb1125=-4991 };bb7 bb1164(bb7 bb1161,bbd bb1152,bbl*bb1136);bba bbj{
bb174 bb181;bbd bb1230;bbd bb1119;bbd bb1406;bbd bb1118;bbd bb1282;
bbd bb1321;bbd bb1305;bbd bb1283;bbd bb1290;bbd bb1274;bbd bb1324;bbu
bb1256;bb43 bb600,bb1193,bb1207;bbf bb376[6 ];}bb161;bba bbj bb491{bbj
bb491*bb99;bbf bb104;bbk bb1319;bbk bb1293;bbk bb1316;bbk bb1317;}
bb445;bba bbj bb794{bbj bb794*bb99;bbj bb491*bb1129;bbd bb29;bbf bb376
[6 ];}bb416;bba bb13{bb1179=0 ,bb1598,bb1112,bb1056,bb1047}bb236;bba bbj
{bbd bb395;bbd bb371;bbd bb534;bb420*bb944;bb95 bb1014;}bb311;bba bbj
{bb503*bb471;bb416*bb1168;bbd bb611;bb445*bb570;bb95 bb622;bbo bb1154
;bbo bb566;bb161*bb527;bbu bb1309;bbk bb1194;bbk bb1145;bb311 bb1081;
}bb35, *bb1630;
#pragma pack(pop)
bba bbj bb999 bb1409, *bb83;bba bbj bb830{bbj bb830*bb328;bb3 bb484;
bbo bb591;bbd bb29;bbk bb455;bbo bb97;bb3 bb318;bbo bb478;bb3 bb569;
bbo bb567;bb3 bb1521;bb103 bb1408;bbf bb1346[6 ];bb103 bb1020;bb103
bb1172;bb103 bb541;bb103 bb547;}bb175, *bb91;bba bbj bb670{bbj bb670*
bb99;bb175*bb328;bbd bb29;bbk bb557;bbk bb1490;bbo bb1466;bbo bb1605;
bbk bb1451;}bb1475, *bb472;bbu bb1310(bb35* *bb1243);bbb bb1327(bb35*
bbi);bb236 bb1311(bb35*bb114,bb389 bb465,bb324 bb140,bb362 bb427,
bb317 bb201);bb236 bb1289(bb35*bb114,bb389 bb465,bb324 bb140,bb362
bb427,bb317 bb201);bb236 bb1299(bb35*bb114,bb175*bb51,bb83 bb78);
bb236 bb1278(bb35*bb114,bb175*bb51,bb83 bb78);bb7 bb1288(bb35*bb114,
bb175*bb51,bbd*bb106);bb7 bb1191(bb83 bb78,bb35*bb114,bb175*bb51,
bb161*bb323,bbu bb606,bbu bb972);bbu bb1918(bbd bb296);bb161*bb1829(
bb35*bbi,bbd bb296,bbu bb606);bb161*bb1896(bb35*bbi,bbd bb296,bbd
bb106);bb161*bb1972(bb35*bbi,bb180 bb106);bbb bb1994(bb525*bb42);
bb161*bb1989(bb35*bbi,bb174*bb181);bbb bb1935(bb35*bbi,bb180 bb106);
bbb bb1910(bb35*bbi,bb180 bb106);bbb bb2040(bb35*bbi);bbb bb1836(bb35
 *bbi);bbb bb1963(bb35*bbi,bbd bb296,bbh bbf bb1219[6 ]);bbu bb1977(
bb35*bbi,bbd bb296,bb416*bb442);bbb bb2097(bb35*bbi);bbb bb2066(bb35*
bbi,bbd bb296,bbh bbf bb1219[6 ],bbf bb104,bbk bb425,bbk bb426);bbu
bb2077(bb35*bbi,bbd bb296,bbf bb104,bbk bb425,bbk bb426);bbu bb1904(
bb35*bbi,bbf bb104,bbk bb425,bbk bb426);bbb bb2047(bb35*bbi,bb445*
bb570,bbo bb611);bbu bb1920(bb311*bbi,bbo bb534);bbb bb2018(bb311*bbi
);bbb bb2098(bb311*bbi);bbu bb1828(bb311*bbi,bb420*bb826);bbu bb1984(
bb311*bbi,bb420*bb826);bbb bb1988(bb35*bbi,bb180 bb106);bbb bb1919(
bb35*bbi,bb180 bb106);bbb bb1864(bb35*bbi,bbd bb29,bbd bb973);bb7
bb1815(bb35*bbi,bb503*bb471);bbb bb2090(bb35*bbi);
#ifdef UNDER_CE
#define bb1970 64
#endif
bba bbj bb1942{bb123 bb1965;bb123 bb1974;bb35*bb1005;}bb1091, *bb1943
;bbr bb1091 bb976;bbj bb999{bb123 bb1937;bbo bb1945;bbd bb2008;bb91
bb1034;bb91 bb1986;bb91 bb1908;bb91 bb1946;bb91 bb1991;bb472 bb1907;
bb472 bb2006;bb472 bb1958;bb95 bb1174;bb103 bb1959;bb103 bb2001;bb103
bb1980;bb123 bb2004;bb123 bb1917;};bbr bb83 bb2011;bbr bb95 bb1968;
bbd bb1912(bbb*bb535,bbb*bb1931,bb160*bb1167);bb160 bb1999(bb123
bb2010,bb123 bb1956,bb81 bb569,bbo bb567,bb81 bb1150,bbo bb1139,bbo
bb1183);
#ifdef UNDER_CE
#define bb618 16
#define bb1176 32
#else
#define bb618 128
#define bb1176 256
#endif
#define bb1144 bb618  *2
#define bb595 ( bb1144  *  2)
#define bb1953 bb595  *  2
#define bb1909 bb595  *  2
bbr bbo bb974;bb160 bb1817(bb61 bb986,bbb*bb42,bbo bb1099,bb124 bb1695
);bb143 bb1976(IN bb83 bb78,IN bb123 bb2019,IN bb3 bb569,IN bbo bb567
,IN bb81 bb1150,IN bbo bb1139,IN bbo bb1183);bb143 bb1960(IN bb83 bb78
);bbd bb1952(bb81 bb535,bb123 bb1971,bb81 bb1964,bbo bb2012,bb81
bb1905,bbo bb1901,bbo bb1990,bb160*bb1167);bbb bb1273(bb83 bb78,bb91*
bb560,bb91 bb51);bb91 bb1307(bb83 bb78,bb91*bb560);bbu bb1814(bb83
bb78);bbb bb1833(bb83 bb78);bb91 bb1483(bb172 bb368,bb83 bb78);bb91
bb1874(bb172 bb368,bb83 bb78);bb91 bb1810(bb172 bb368,bb83 bb78);
bb143 bb1679(bb83 bb78,bb91 bb51);bb143 bb1839(bb83 bb78,bb91 bb51);
bb143 bb1895(bb83 bb78,bb91 bb51);bbb bb1928();bbb bb1930();bbb bb155
(bbh bbl*bb20,...);bbb bb2062(bb186 bbg);bbb bb2096(bbb*bb28,bbo bb11
);bbb bb2103(bbb*bbx,bbo bb5);bbb bb1852(bbb*bbx,bbo bb5);bbb bb1370(
bbb*bbx,bbo bb5);bbb bb1951();bbb bb1779();bbb bb2035(bb374*bb1973);
bbb bb1594(bb330*bb28);bbb bb1306(bb330*bb754,bb508*bb136);bbb bb1503
(bb330*bb754,bb431*bb1631);bbj bb2436{bbd bb1410;bbd bb97;bbd bb483;
bbe bb387;bbe bb557;bbf*bb42;bbj bb2436*bb99;};bbj bb2605{bbd bb428;
bbd bb483;bbd bb2701;bbj bb2605*bb2700;bbj bb2436*bb2699;bbf*bb42;};
#ifdef _WIN32
bbe bb2624(bbj bb1950*bb304);
#else
bbe bb2624(bbj bb2130*bb487);
#endif
bb2730("");bb2702("\x61\x68\x6f\x70\x65");bb1091 bb976={0 };bb40 bb1409
bb535={0 };bb2732 bb40 bb8 bb2448(bb1195*bb82,bbu bb2636){bbd bb986;
bbf*bb42=bb93;bbe bb2250=0 ;bbe bb24;bbm(bb82->bb545>2048 ){bb561("\x6d"
"\x2d\x3e\x6f\x75\x74\x53\x69\x7a\x65\x20\x3e\x20\x32\x30\x34\x38\x2c"
"\x20\x6d\x2d\x3e\x6f\x75\x74\x53\x69\x7a\x65\x3a\x20\x25\x64\n",bb82
->bb545);bb24=-bb2194;bb96 bb1022;}bbm(bb82->bb592!=4 ){bb561("\x6d"
"\x2d\x3e\x69\x6e\x53\x69\x7a\x65\x20\x21\x3d\x20\x34\n");bb24=-
bb2194;bb96 bb1022;}bb24=bb2713(bb986,(bb31)bb82->bb602);bbm(bb24!=0 ){
bb561("\x64\x65\x76\x5f\x69\x6f\x63\x74\x6c\x5f\x77\x69\x74\x68\x5f"
"\x6d\x2c\x20\x67\x65\x74\x5f\x75\x73\x65\x72\x2c\x20\x25\x64\n",bb24
);bb96 bb1022;}
#ifdef CONFIG_COMPAT
bbm(bb2636){bbm(bb986==0xFF010207 )bb2250=bb1369;}
#endif
bb42=bb127(bb2250+bb82->bb545);bbm(bb42==bb93){bb561("\x62\x75\x66"
"\x20\x3d\x3d\x20\x4e\x55\x4c\x4c\n");bb24=-bb2335;bb96 bb1022;}bb24=
bb2478(bb42+bb2250,bb82->bb573,bb82->bb545);bbm(bb24!=0 ){bb561("\x64"
"\x65\x76\x5f\x69\x6f\x63\x74\x6c\x5f\x77\x69\x74\x68\x5f\x6d\x2c\x20"
"\x63\x6f\x70\x79\x5f\x66\x72\x6f\x6d\x5f\x75\x73\x65\x72\x2c\x20\x25"
"\x64\n",bb24);bb96 bb1022;}bb135(&bb535.bb1174);
#ifdef _DEBUG
bbm(bb986==0xFF010212 ){bb155("\x62\x65\x66\x6f\x72\x65\x2c\x20\x4f"
"\x49\x44\x5f\x50\x47\x50\x5f\x53\x48\x55\x54\x44\x4f\x57\x4e");
bb1779();}
#endif
bbm(bb1817(bb986,bb42,bb2250+bb82->bb545,&bb82->bb371)!=0 ){bb133(&
bb535.bb1174);bb24=-bb2194;bb96 bb1022;}
#ifdef _DEBUG
bbm(bb986==0xFF010205 )bb1951();bbm(bb986==0xFF010207 )bb1779();bbm(
bb986==0xFF010212 ){bb155("\x61\x66\x74\x65\x72\x2c\x20\x4f\x49\x44"
"\x5f\x50\x47\x50\x5f\x53\x48\x55\x54\x44\x4f\x57\x4e");bb1779();}
#endif
bb133(&bb535.bb1174);bb24=bb2708(bb82->bb573,bb42,bb82->bb371);bbm(
bb24!=0 ){bb561("\x64\x65\x76\x5f\x69\x6f\x63\x74\x6c\x5f\x77\x69\x74"
"\x68\x5f\x6d\x2c\x20\x63\x6f\x70\x79\x5f\x74\x6f\x5f\x75\x73\x65\x72"
"\x2c\x20\x25\x64\n",bb24);bb96 bb1022;}bb1022:bbm(bb42)bb108(bb42);
bb4 bb24;}bb40 bb8 bb2556(bbj bb26*bb20,bbt bbe bbn,bbt bb8 bb2613){
bb1195 bb82, *bb2428=(bb1195* )bb2613;bbe bb24;
#ifdef _DEBUG
bb155("\x6e\x66\x2e\x63\x2c\x20\x62\x65\x67\x69\x6e\x20\x64\x65\x76"
"\x5f\x69\x6f\x63\x74\x6c\x5f\n");
#endif
bbm(bbn!=bb1323){bb561("\x63\x20\x21\x3d\x20\x49\x4f\x43\x54\x4c\x5f"
"\x56\x50\x4e\n");bb24=-bb2194;bb96 bb1022;}bb24=bb2478(&bb82,bb2428,
bb12(bb82));bbm(bb24!=0 ){bb561("\x64\x65\x76\x5f\x69\x6f\x63\x74\x6c"
"\x5f\x2c\x20\x63\x6f\x70\x79\x5f\x66\x72\x6f\x6d\x5f\x75\x73\x65\x72"
"\x2c\x20\x25\x64\n",bb24);bb96 bb1022;}bb24=bb2448(&bb82,0 );bbm(bb24
!=0 )bb96 bb1022;bb24=bb2609(bb82.bb371,&bb2428->bb371);bbm(bb24!=0 ){
bb561("\x64\x65\x76\x5f\x69\x6f\x63\x74\x6c\x5f\x2c\x20\x70\x75\x74"
"\x5f\x75\x73\x65\x72\x2c\x20\x25\x64\n",bb24);bb96 bb1022;}bb1022:
#ifdef _DEBUG
bb155("\x6e\x66\x2e\x63\x2c\x20\x65\x6e\x64\x20\x64\x65\x76\x5f\x69"
"\x6f\x63\x74\x6c\x5f\n");
#endif
bb4 bb24;}
#ifdef CONFIG_COMPAT
bb40 bb8 bb2630(bbj bb26*bb20,bbt bbe bbn,bbt bb8 bb2606){bb1291
bb2061, *bb2391=(bb1291* )bb2399(bb2606);bb1195 bb82;bbe bb24;
#ifdef _DEBUG
bb155("\x6e\x66\x2e\x63\x2c\x20\x62\x65\x67\x69\x6e\x20\x64\x65\x76"
"\x5f\x63\x6f\x6d\x70\x61\x74\x5f\x69\x6f\x63\x74\x6c\x5f\n");
#endif
bbm(bbn!=bb1323){bb561("\x63\x20\x21\x3d\x20\x49\x4f\x43\x54\x4c\x5f"
"\x56\x50\x4e\n");bb24=-bb2194;bb96 bb1022;}bb24=bb2478(&bb2061,
bb2391,bb12(bb2061));bbm(bb24!=0 ){bb561("\x64\x65\x76\x69\x63\x65\x5f"
"\x63\x6f\x6d\x70\x61\x74\x5f\x69\x6f\x63\x74\x6c\x2c\x20\x63\x6f\x70"
"\x79\x5f\x66\x72\x6f\x6d\x5f\x75\x73\x65\x72\x2c\x20\x25\x64\n",bb24
);bb96 bb1022;}bb82.bb602=bb2399(bb2061.bb602);bb82.bb592=bb2061.
bb592;bb82.bb573=bb2399(bb2061.bb573);bb82.bb545=bb2061.bb545;bb24=
bb2448(&bb82,1 );bbm(bb24!=0 )bb96 bb1022;bb24=bb2609(bb82.bb371,&
bb2391->bb371);bbm(bb24!=0 ){bb561("\x64\x65\x76\x5f\x63\x6f\x6d\x70"
"\x61\x74\x5f\x69\x6f\x63\x74\x6c\x5f\x2c\x20\x70\x75\x74\x5f\x75\x73"
"\x65\x72\x2c\x20\x25\x64\n",bb24);bb96 bb1022;}bb1022:
#ifdef _DEBUG
bb155("\x6e\x66\x2e\x63\x2c\x20\x65\x6e\x64\x20\x64\x65\x76\x5f\x63"
"\x6f\x6d\x70\x61\x74\x5f\x69\x6f\x63\x74\x6c\x5f\n");
#endif
bb4 bb24;}
#endif
bb40 bbe bb2595(bbj bb2305*bb2305,bbj bb26*bb20){bb4 0 ;}bb40 bbe
bb2598(bbj bb2305*bb2305,bbj bb26*bb20){bb4 0 ;}bb40 bbj bb2675 bb2648
={.bb2673=bb2556,
#ifdef CONFIG_COMPAT
.bb2688=bb2630,
#endif
.bb2712=bb2595,.bb2692=bb2598};bb40 bbj bb2669 bb2410={.bb2696=217 ,.
bb36=bb641,.bb2720=&bb2648,};bbt bbl*bb2474(bbj bb2130*bb487,bbt bbe
bb22){bbt bbl*bb2632=bb2657(bb487);bb487->bb2543+=bb22;bb487->bb22+=
bb22;bbm(bb2658(bb487->bb2543>bb487->bb456))bb155("\x73\x6b\x62\x5f"
"\x70\x75\x74\x2c\x20\x74\x61\x69\x6c\x20\x3e\x20\x65\x6e\x64\n");bb4
bb2632;}bba bbj{bb61 bb2344;bb61 bb1857;bb61 bb2377;}bb2623;bb40
bb2623 bb1876;bb40 bbo bb2527(bbo bb2330,bbj bb2130*bb487,bbh bbj
bb2271*bb2615,bbh bbj bb2271*bb452,bbe( *bb2553)(bbj bb2130* )){bbo
bb540=bb2422;bb236 bb1200;bbj bb1950*bb304;bb374 bb616={{0 },{0 },0 };
bb330 bb446;bb362 bb1300=bb93;bb317 bb1008=bb93;bb431 bb1287;bb508
bb1250;bb175*bb435=bb93;bb616.bb383=bb977;bb304=(bbj bb1950* )bb2131(
bb487);bbm(!bb304){bb540=bb1315;bb96 bb164;}bb74(&bb446,bb304,bb12(
bb446));bbm(bb446.bb291==2 ){bb540=bb1315;bb96 bb164;}bbm(bb446.bb291
==6 ){bbm(!(bb304+bb304->bb1410*4 )){bb540=bb1315;bb96 bb164;}bb74(&
bb1250,(bb3)bb304+bb304->bb1410*4 ,bb12(bb1250));bb1008=&bb1250;
#ifdef _DEBUG
bb155("\x69\x6e\x63\x6f\x6d\x69\x6e\x67\x2c\x20");bb1306(&bb446,
bb1008);
#endif
}bb50 bbm(bb446.bb291==17 ){bbm(!(bb304+bb304->bb1410*4 )){bb540=bb1315
;bb96 bb164;}bb74(&bb1287,(bb3)bb304+bb304->bb1410*4 ,bb12(bb1287));
bb1300=&bb1287;
#ifdef _DEBUG
bbm(1 ){bb155("\x69\x6e\x63\x6f\x6d\x69\x6e\x67\x2c\x20");bb1503(&
bb446,bb1300);}bb50{bbm(bb1287.bb290!=4500 &&bb1287.bb290!=bb56(4500 )){
bb155("\x6c\x6f\x63\x61\x6c\x20\x69\x6e\x2c\x20\x75\x64\x70\n");}}
#endif
}bb1200=bb1289(bb976.bb1005,&bb616,&bb446,bb1300,bb1008);
#ifdef _DEBUG
bb155("\x70\x6d\x73\x3a\x25\x64\x2c\x20\x69\x6e\x63\x6f\x6d\x69\x6e"
"\x67\x20\x69\x70\x2c\x20",bb1200);bb1594(&bb446);
#endif
bbm(bb1200==bb1047){bb540=bb1315;bb96 bb164;}bbm(bb1200!=bb1056)bb96
bb164;{bbo bb381;bb160 bbg;bbe bb1266;bb435=bb1483(&bbg,&bb535);bbm(!
bb435&&bb435->bb484&&bb435->bb318){bb155("\x63\x61\x6e\x27\x74\x20"
"\x61\x6c\x6c\x6f\x63\x20\x70\x61\x63\x6b\x65\x74\n");bb96 bb164;}
bb381=bb197(bb304->bb2590);bb74(bb435->bb484,&bb616,bb12(bb616));bb74
(bb435->bb484+bb12(bb616),bb304,bb381);bb74(bb435->bb318,&bb616,bb12(
bb616));bb435->bb591=bb12(bb616)+bb381;bb435->bb1020=1 ;bb435->bb29=
bb514(bb304->bb2661);bb435->bb455=bb1300?bb1300->bb439:0 ;bb1200=
bb1278(bb976.bb1005,bb435,&bb535);bbm(bb1200!=bb1179)bb96 bb164;
#ifdef _DEBUG
{bbj bb1950*bb1892;bb330 bb2013;bb362 bb1287=bb93;bb317 bb1250=bb93;
bb431 bb2326;bb508 bb2380;bb1892=(bbj bb1950* )(bb435->bb318+bb12(
bb616));bb74(&bb2013,bb1892,bb12(bb2013));bb155("\x69\x6e\x63\x6f\x6d"
"\x69\x6e\x67\x20\x78\x66\x6f\x72\x6d\x42\x6c\x6f\x63\x6b\x2c\x20");
bb1594(&bb2013);bbm(bb2013.bb291==6 ){bb74(&bb2380,(bb3)bb1892+bb1892
->bb1410*4 ,bb12(bb2380));bb1250=&bb2380;bb155("\x69\x6e\x63\x6f\x6d"
"\x69\x6e\x67\x20\x78\x66\x6f\x72\x6d\x42\x6c\x6f\x63\x6b\x20\x74\x63"
"\x70\x2c\x20");bb1306(&bb2013,bb1250);}bb50 bbm(bb2013.bb291==17 ){
bbm(!(bb304+bb304->bb1410*4 )){bb540=bb1315;bb96 bb164;}bb74(&bb2326,(
bb3)bb1892+bb1892->bb1410*4 ,bb12(bb2326));bb1287=&bb2326;bb155("\x69"
"\x6e\x63\x6f\x6d\x69\x6e\x67\x20\x78\x66\x6f\x72\x6d\x42\x6c\x6f\x63"
"\x6b\x20\x75\x64\x70\x2c\x20");bb1503(&bb2013,bb1287);}}
#endif
bb1266=bb435->bb478-bb435->bb591;bb381=bb435->bb478-bb12(bb616);bbm(
bb1266>0 ){bb27(bb2614(bb487)==0 );bbm(bb2616(bb487,0 ,bb1266,bb141)!=0 ){
bb155("\x63\x61\x6e\x27\x74\x20\x65\x78\x70\x61\x6e\x64\x20\x73\x6b"
"\x62\n");bb96 bb164;}bb2474(bb487,bb1266);}bb50{bbe bb2118=bb487->
bb22+bb1266;bbm(bb1266<0 )bb2525(bb487,bb2118);bbm(!bb2596(bb487,
bb2118)){bb155("\x63\x61\x6e\x27\x74\x20\x6d\x61\x6b\x65\x20\x73\x6b"
"\x62\x20\x77\x72\x69\x74\x61\x62\x6c\x65\n");bb96 bb164;}}bb74(
bb2131(bb487),bb435->bb318+bb12(bb616),bb381);bb304=(bbj bb1950* )bb2131
(bb487);bb74(&bb446,bb304,bb12(bb446));bbm(bb446.bb291==6 ){bb74(&
bb1250,(bb3)bb304+bb304->bb1410*4 ,bb12(bb1250));bb1008=&bb1250;bb1876
.bb2344=bb446.bb312;bb1876.bb1857=bb1008->bb621;bb1876.bb2377=bb1008
->bb948;}bbm(bb1088(bb2131(bb487)->bb2674)){bb155("\x66\x72\x61\x67"
"\x6d\x65\x6e\x74\n");bb2684(bb487);bb2665(bb487);bb2731(bb487);bb540
=bb2659;bb96 bb164;}}bb540=bb1315;bb164:bbm(bb435)bb1679(&bb535,bb435
);bb4 bb540;}bb40 bbo bb2600(bbo bb2330,bbj bb2130*bb487,bbh bbj
bb2271*bb2615,bbh bbj bb2271*bb452,bbe( *bb2553)(bbj bb2130* )){bbo
bb540=bb2422;bb236 bb1200;bbj bb1950*bb304;bbj bb2271*bb2220;bb374
bb616={{0 },{0 },0 };bb330 bb446;bb362 bb1300=bb93;bb317 bb1008=bb93;
bb431 bb1287;bb508 bb1250;bb175*bb435=bb93;bb616.bb383=bb977;bb304=(
bbj bb1950* )bb2131(bb487);bbm(!bb304){bb540=bb1315;bb96 bb164;}bb74(
&bb446,bb304,bb12(bb446));bb2220=bb2705(bb487)->bb2220;bbm(!bb2220||
bb2676(bb2220->bb36,"\x77\x6c\x61\x6e\x30")!=0 ){bb540=bb1315;bb96
bb164;}bbm(bb1918(bb304->bb2286)){bb4 bb2422;}bbm(bb446.bb291==2 ){
bb540=bb1315;bb96 bb164;}bbm(bb446.bb291==6 ){bbe bb2427=bb304->bb1410
 *4 ;bb317 bb2253=(bb317)((bb3)bb304+bb2427);bb74(&bb1250,bb2253,bb12(
bb1250));bb1008=&bb1250;
#ifdef _DEBUG
bb155("\x6f\x75\x74\x67\x6f\x69\x6e\x67\x2c\x20");bb1306(&bb446,
bb1008);
#endif
{bb35*bb114=bb976.bb1005;bbm(bb114->bb566>0 &&bb304->bb2286==bb114->
bb527[0 ].bb181.bb29&&bb1008->bb565==bb1876.bb2377&&bb1008->bb290==
bb1876.bb1857){bbo bb2214=bb197(bb446.bb381)-bb2427;bbf*bb42=bb127(
bb12(bb627)+bb2214);bb589 bb578=(bb589)bb42;bb1876.bb2377=0 ;bb578->
bb312=bb446.bb312;bb578->bb256=bb1876.bb2344;bb578->bb934=0 ;bb578->
bb291=6 ;bb578->bb937=bb56(bb2214);bb304->bb2286=bb1876.bb2344;bb1008
->bb326=0 ;bb74(&bb2253->bb326,&bb1008->bb326,2 );bb74(bb42+bb12(bb627),
bb2253,bb2214);bb1008->bb326=bb660(bb42,bb12(bb627)+bb2214);bb74(&
bb2253->bb326,&bb1008->bb326,2 );
#ifdef _DEBUG
bb155("\x6e\x65\x77\x20\x63\x68\x65\x63\x6b\x73\x75\x6d\x3a\x25\x30"
"\x34\x78\n",bb56(bb1008->bb326));
#endif
bb108(bb42);bb2729(bb304);bb74(&bb446,bb304,bb12(bb446));}}}bb50 bbm(
bb446.bb291==17 ){bbm(!(bb304+bb304->bb1410*4 )){bb540=bb1315;bb96 bb164
;}bb74(&bb1287,(bb3)bb304+bb304->bb1410*4 ,bb12(bb1287));bb1300=&
bb1287;
#ifdef _DEBUG
bb155("\x6f\x75\x74\x67\x6f\x69\x6e\x67\x2c\x20\n");bb1503(&bb446,
bb1300);
#endif
}bb1200=bb1311(bb976.bb1005,&bb616,&bb446,bb1300,bb1008);
#ifdef _DEBUG
bb155("\x70\x6d\x73\x3a\x25\x64\x2c\x20\x6f\x75\x74\x67\x6f\x69\x6e"
"\x67\x20\x69\x70\x2c\x20",bb1200);bb1594(&bb446);
#endif
bbm(bb1200==bb1047){bb540=bb1315;bb96 bb164;}bbm(bb1200!=bb1056)bb96
bb164;{bbo bb381;bb160 bbg;bbe bb1266;bb435=bb1483(&bbg,&bb535);bbm(!
bb435&&bb435->bb484&&bb435->bb318){bb155("\x63\x61\x6e\x27\x74\x20"
"\x61\x6c\x6c\x6f\x63\x20\x70\x61\x63\x6b\x65\x74\n");bb96 bb164;}
bb381=bb197(bb304->bb2590);bb74(bb435->bb484,&bb616,bb12(bb616));bb74
(bb435->bb484+bb12(bb616),bb304,bb381);bb74(bb435->bb318,&bb616,bb12(
bb616));
#ifdef _DEBUG
bbm(bb446.bb291==6 ){bb155("\x6f\x75\x74\x67\x6f\x69\x6e\x67\x20\x73"
"\x72\x63\x42\x6c\x6f\x63\x6b\x20\x74\x63\x70\x2c\x20");bb1306(&bb446
,bb1008);}bb50 bbm(bb446.bb291==17 ){bb155("\x6f\x75\x74\x67\x6f\x69"
"\x6e\x67\x20\x73\x72\x63\x42\x6c\x6f\x63\x6b\x20\x75\x64\x70\x2c\x20"
);bb1503(&bb446,bb1300);}
#endif
bb435->bb591=bb12(bb616)+bb381;bb435->bb1020=1 ;bb435->bb29=bb514(
bb304->bb2286);bb435->bb455=bb1300?bb1300->bb439:0 ;bb1200=bb1299(
bb976.bb1005,bb435,&bb535);bb155("\x50\x4d\x44\x6f\x54\x72\x61\x6e"
"\x73\x66\x6f\x72\x6d\x4f\x75\x74\x67\x6f\x69\x6e\x67\x2c\x20\x70\x6d"
"\x73\x3a\x25\x64\n",bb1200);bbm(bb1200!=bb1179)bb96 bb164;
#ifdef _DEBUG
{bb330 bb446;bb74(&bb446,bb435->bb318+bb12(bb616),bb12(bb446));bb155(""
"\x6f\x75\x74\x67\x6f\x69\x6e\x67\x20\x78\x66\x6f\x72\x6d\x42\x6c\x6f"
"\x63\x6b\x2c\x20");bb1594(&bb446);}
#endif
bb1266=bb435->bb478-bb435->bb591;bb381=bb435->bb478-bb12(bb616);bbm(
bb1266>0 ){bb27(bb2614(bb487)==0 );bbm(bb2616(bb487,0 ,bb1266,bb141)!=0 ){
bb155("\x63\x61\x6e\x27\x74\x20\x65\x78\x70\x61\x6e\x64\x20\x73\x6b"
"\x62\n");bb96 bb164;}bb2474(bb487,bb1266);}bb50{bbe bb2118=bb487->
bb22+bb1266;bbm(bb1266<0 )bb2525(bb487,bb2118);bbm(!bb2596(bb487,
bb2118)){bb155("\x63\x61\x6e\x27\x74\x20\x6d\x61\x6b\x65\x20\x73\x6b"
"\x62\x20\x77\x72\x69\x74\x61\x62\x6c\x65\n");bb96 bb164;}}bb74(
bb2131(bb487),bb435->bb318+bb12(bb616),bb381);
#ifdef _DEBUG
bb155("\x73\x6b\x62\x2d\x3e\x69\x70\x5f\x73\x75\x6d\x6d\x65\x64\x3a"
"\x25\x64\n",bb487->bb2635);
#endif
bb487->bb2635=bb2655;bb2707(bb487,bb2697);}bb540=bb1315;bb164:bbm(
bb435)bb1679(&bb535,bb435);bb4 bb540;}bb40 bbj bb2709 bb2517={.bb2534
=bb2527,.bb2594=bb2533,.bb2330=bb2670,.bb2627=bb2599},bb2489={.bb2534
=bb2600,.bb2594=bb2533,.bb2330=bb2727,.bb2627=bb2599};
#if bb2487( 3, 8, 0 ) <= LINUX_VERSION_CODE && LINUX_VERSION_CODE <  \
bb2487( 3, 9, 0 )
#define bb2393( bbc, bbp, bbn, bbs) bb2723( bbc, bbp, bbn, bbs)
#else
bb40 bbe bb2393(bbl*bb1039,bbl* *bb2135,bbl* *bb2205,bbe bb2426){bbj
bb2663*bb2209;bb2704 bb2445=(bb2426==bb2683)?bb141:bb2706;
#ifdef _DEBUG
bbl* *bb28=bb2135;bb561("\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72\x6d\x6f"
"\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x2c\x20\x25\x73",bb1039);bb109(
 *bb28){bb561("\x20\x25\x73", *bb28);bb28++;}bb561("\n");
#endif
#if LINUX_VERSION_CODE >= bb2487( 3, 10, 0 )
bb2209=bb2620(bb1039,bb2135,bb2205,bb2445,bb93,bb93,bb93);
#else
bb2209=bb2620(bb1039,bb2135,bb2205,bb2445);
#endif
bbm(bb2209==bb93){bb561("\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72\x6d\x6f"
"\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x2c\x20\x69\x6e\x66\x6f\x20\x3d"
"\x3d\x20\x4e\x55\x4c\x4c\n");bb4-bb2335;}bb4 bb2733(bb2209,bb2426);}
#endif
#if defined( PLAT_VER) && PLAT_VER >= 0x50000
#define bb2460 "/system/bin/mwlan_helper"
#else
#define bb2460 "/system/bin/toolbox"
#endif
bbe bb2721(){bbe bb24;
#ifdef _DEBUG
bb1928();
#endif
bb561("\x49\x2d\x57\x4c\x41\x4e\x2f\x69\x70\x73\x65\x63\x64\x72\x76"
"\x74\x6c\x2c\x20\x6d\x77\x6c\x61\x6e\x20\x6b\x6f\x20\x73\x72\x63\x2c"
"\x20\x32\x30\x31\x34\x2e\x31\x32\x2e\x31\x38\n");bb155("\x69\x70\x73"
"\x65\x63\x64\x72\x76\x74\x6c\x2c\x20\x62\x65\x67\x69\x6e\x20\x69\x6e"
"\x69\x74\x5f\x6d\x6f\x64\x75\x6c\x65\n");bb142(&bb535.bb1174);bbm(!
bb1310(&bb976.bb1005)){bb561("\x63\x61\x6e\x27\x74\x20\x69\x6e\x69"
"\x74\x20\x70\x6d\n");bb4-bb2335;}{bb503 bbn={0 };bbn.bb1157=1 ;bb1815(
bb976.bb1005,&bbn);}bb974=1480 ;bbm(!bb1814(&bb535)){bb561("\x63\x61"
"\x6e\x27\x74\x20\x61\x6c\x6c\x6f\x63\x20\x70\x6b\x74\x20\x70\x6f\x6f"
"\x6c\n");bb4-bb2335;}bb24=bb2668(&bb2410);bbm(bb24!=0 ){bb561("\x63"
"\x61\x6e\x27\x74\x20\x72\x65\x67\x20\x6d\x69\x73\x63\x20\x64\x65\x76"
"\x2c\x20\x20\x25\x64\n",bb24);bb4 bb24;}bb155("\x73\x75\x63\x63\x65"
"\x65\x64\x20\x69\x6e\x20\x72\x65\x67\x69\x73\x74\x65\x72\x69\x6e\x67"
"\x20\x6d\x69\x73\x63\n");bb24=bb2530(&bb2517);bbm(bb24!=0 ){bb561(""
"\x63\x61\x6e\x27\x74\x20\x72\x65\x67\x20\x68\x6f\x6f\x6b\x20\x69\x6e"
"\x2c\x20\x25\x64\n",bb24);bb4 bb24;}bb155("\x73\x75\x63\x63\x65\x65"
"\x64\x20\x69\x6e\x20\x72\x65\x67\x69\x73\x74\x65\x72\x69\x6e\x67\x20"
"\x74\x68\x65\x20\x68\x6f\x6f\x6b\x5f\x69\x6e\n");bb24=bb2530(&bb2489
);bbm(bb24!=0 ){bb561("\x63\x61\x6e\x27\x74\x20\x72\x65\x67\x20\x68"
"\x6f\x6f\x6b\x20\x6f\x75\x74\x2c\x20\x25\x64\n",bb24);bb4 bb24;}
bb155("\x73\x75\x63\x63\x65\x65\x64\x20\x69\x6e\x20\x72\x65\x67\x69"
"\x73\x74\x65\x72\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x6f\x6f\x6b\x5f"
"\x6f\x75\x74\n");{bbl*bb2135[]={"\x2f\x73\x79\x73\x74\x65\x6d\x2f"
"\x62\x69\x6e\x2f\x63\x68\x6f\x77\x6e","\x73\x79\x73\x74\x65\x6d\x2e"
"\x73\x79\x73\x74\x65\x6d",bb481,bb93};bb40 bbl*bb2205[]={"\x48\x4f"
"\x4d\x45\x3d\x2f","\x54\x45\x52\x4d\x3d\x6c\x69\x6e\x75\x78","\x50"
"\x41\x54\x48\x3d\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e",bb93};
bbe bb24=bb2393(bb2460,bb2135,bb2205,bb2672);bbm(bb24!=0 )bb561("\x69"
"\x6e\x69\x74\x5f\x6d\x6f\x64\x75\x6c\x65\x2c\x20\x63\x61\x6c\x6c\x5f"
"\x75\x73\x65\x72\x6d\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x20\x66"
"\x61\x69\x6c\x65\x64\x3a\x20\x25\x64\n",bb24);}bb155("\x69\x70\x73"
"\x65\x63\x64\x72\x76\x74\x6c\x2c\x20\x65\x6e\x64\x20\x69\x6e\x69\x74"
"\x5f\x6d\x6f\x64\x75\x6c\x65\n");bb4 0 ;}bbb bb2724(){bb155("\x69\x70"
"\x73\x65\x63\x64\x72\x76\x74\x6c\x2c\x20\x62\x65\x67\x69\x6e\x20\x63"
"\x6c\x65\x61\x6e\x75\x70\x5f\x6d\x6f\x64\x75\x6c\x65\n");bb2561(&
bb2517);bb2561(&bb2489);bb2695(&bb2410);bb1833(&bb535);bb1327(bb976.
bb1005);bb155("\x69\x70\x73\x65\x63\x64\x72\x76\x74\x6c\x2c\x20\x65"
"\x6e\x64\x20\x63\x6c\x65\x61\x6e\x75\x70\x5f\x6d\x6f\x64\x75\x6c\x65"
"\n");
#ifdef _DEBUG
bb1930();
#endif
}
