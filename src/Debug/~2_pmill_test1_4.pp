0 12 0 -1 -1 LOADU/port.u
1 52 0 0 -1 MACHIN/pworks,0     $$ Declare default Postprocessor
2 9 0 1 -1 multax/on
3 35 0 2 -1 invis/sf1,sf4,sf5,sf7,sf8,sf6, sf39
4 22 0 3 -1 CV3=SPLINE/PSF1,3,0.45
5 33 0 4 -1 CV4=SPLINE/OFFSET,CV3,ZS,1.000000
6 26 0 5 -1 PT1=POINT/YSMALL,ENDPT,CV3
7 18 0 6 -1 CU/0.2,0.1,0.2,-89
8 12 0 7 -1 OB/PT1,X,Y,Z
9 24 0 8 -1 PT2=POINT/X+0.25,Y-0.5,Z
10 6 0 9 -1 th/0.1
11 16 0 10 -1 goto/X+3,Y-0.5,Z
12 26 0 11 -1 PT3=POINT/YSMALL,ENDPT,CV4
13 18 0 12 -1 CU/0.2,0.1,0.2,-89
14 12 0 13 -1 OB/PT1,X,Y,Z
15 23 0 14 -1 PL1=PL/(PV/ON,CV4,0.10)
16 23 0 15 -1 PL2=PL/(PV/ON,CV4,0.30)
17 8 0 16 -1 goto/PT3
18 11 0 17 -1 TA/THRU,PT3
19 38 0 18 -1 PMILL/PSF1,PL1,PL2,STEP,0.1,LINEAR,CLW
20 38 0 19 -1 PMILL/PSF1,PL1,PL2,STEP,0.1,LINEAR,CLW
