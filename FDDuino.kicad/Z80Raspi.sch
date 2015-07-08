EESchema Schematic File Version 2
LIBS:meesokim
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:00N8VEM
LIBS:Zilog
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RASPI_V2 P?
U 1 1 559D4081
P 2200 3650
F 0 "P?" H 2100 4750 50  0000 C CNN
F 1 "RASPI_V2" V 2350 3350 50  0000 C CNN
F 2 "" H 2200 2700 60  0000 C CNN
F 3 "" H 2200 2700 60  0000 C CNN
	1    2200 3650
	1    0    0    -1  
$EndComp
$Comp
L Z80CPU U?
U 1 1 559D4282
P 6900 4050
F 0 "U?" H 6350 5450 50  0000 L CNN
F 1 "Z80CPU" H 7150 5450 50  0000 L CNN
F 2 "" H 6900 4450 60  0000 C CNN
F 3 "" H 6900 4450 60  0000 C CNN
	1    6900 4050
	1    0    0    -1  
$EndComp
$Comp
L 74F676 U?
U 1 1 559D42F5
P 4500 3600
F 0 "U?" H 4050 4400 50  0000 L CNN
F 1 "74F676" H 4650 4400 50  0000 L CNN
F 2 "" H 4500 3600 60  0000 C CNN
F 3 "" H 4500 3600 60  0000 C CNN
	1    4500 3600
	1    0    0    -1  
$EndComp
Wire Bus Line
	5400 2100 5400 4650
Wire Bus Line
	5400 2100 7900 2100
Wire Bus Line
	7900 2100 7900 4500
Wire Wire Line
	5100 3000 5300 3000
Wire Wire Line
	5100 3100 5300 3100
Wire Wire Line
	5100 3200 5300 3200
Wire Wire Line
	5100 3300 5300 3300
Wire Wire Line
	5100 3400 5300 3400
Wire Wire Line
	5100 3500 5300 3500
Wire Wire Line
	5100 3600 5300 3600
Wire Wire Line
	5100 3700 5300 3700
Wire Wire Line
	5100 3800 5300 3800
Wire Wire Line
	5100 3900 5300 3900
Wire Wire Line
	5100 4000 5300 4000
Wire Wire Line
	5100 4100 5300 4100
Wire Wire Line
	5100 4200 5300 4200
Wire Wire Line
	5100 4300 5300 4300
Wire Wire Line
	5100 4400 5300 4400
Wire Wire Line
	5100 4500 5300 4500
Text Label 5150 3000 0    60   ~ 0
A0
Text Label 5150 3100 0    60   ~ 0
A1
Text Label 5150 3200 0    60   ~ 0
A2
Text Label 5150 3300 0    60   ~ 0
A3
Text Label 5150 3400 0    60   ~ 0
A4
Text Label 5150 3500 0    60   ~ 0
A5
Text Label 5150 3600 0    60   ~ 0
A6
Text Label 5150 3700 0    60   ~ 0
A7
Text Label 5150 3800 0    60   ~ 0
A8
Text Label 5150 3900 0    60   ~ 0
A9
Text Label 5150 4000 0    60   ~ 0
A10
Text Label 5150 4100 0    60   ~ 0
A11
Text Label 5150 4200 0    60   ~ 0
A12
Text Label 5150 4300 0    60   ~ 0
A13
Text Label 5150 4400 0    60   ~ 0
A14
Text Label 5150 4500 0    60   ~ 0
A15
Entry Wire Line
	5300 3000 5400 3100
Entry Wire Line
	5300 3100 5400 3200
Entry Wire Line
	5300 3200 5400 3300
Entry Wire Line
	5300 3300 5400 3400
Entry Wire Line
	5300 3400 5400 3500
Entry Wire Line
	5300 3500 5400 3600
Entry Wire Line
	5300 3600 5400 3700
Entry Wire Line
	5300 3700 5400 3800
Entry Wire Line
	5300 3800 5400 3900
Entry Wire Line
	5300 3900 5400 4000
Entry Wire Line
	5300 4000 5400 4100
Entry Wire Line
	5300 4100 5400 4200
Entry Wire Line
	5300 4200 5400 4300
Entry Wire Line
	5300 4300 5400 4400
Entry Wire Line
	5300 4400 5400 4500
Entry Wire Line
	5300 4500 5400 4600
Wire Wire Line
	7600 2850 7800 2850
Wire Wire Line
	7600 2950 7800 2950
Wire Wire Line
	7600 3050 7800 3050
Wire Wire Line
	7600 3150 7800 3150
Wire Wire Line
	7600 3250 7800 3250
Wire Wire Line
	7600 3350 7800 3350
Wire Wire Line
	7600 3450 7800 3450
Wire Wire Line
	7600 3550 7800 3550
Wire Wire Line
	7600 3650 7800 3650
Wire Wire Line
	7600 3750 7800 3750
Wire Wire Line
	7600 3850 7800 3850
Wire Wire Line
	7600 3950 7800 3950
Wire Wire Line
	7600 4050 7800 4050
Wire Wire Line
	7600 4150 7800 4150
Wire Wire Line
	7600 4250 7800 4250
Wire Wire Line
	7600 4350 7800 4350
Text Label 7650 2850 0    60   ~ 0
A0
Text Label 7650 2950 0    60   ~ 0
A1
Text Label 7650 3050 0    60   ~ 0
A2
Text Label 7650 3150 0    60   ~ 0
A3
Text Label 7650 3250 0    60   ~ 0
A4
Text Label 7650 3350 0    60   ~ 0
A5
Text Label 7650 3450 0    60   ~ 0
A6
Text Label 7650 3550 0    60   ~ 0
A7
Text Label 7650 3650 0    60   ~ 0
A8
Text Label 7650 3750 0    60   ~ 0
A9
Text Label 7650 3850 0    60   ~ 0
A10
Text Label 7650 3950 0    60   ~ 0
A11
Text Label 7650 4050 0    60   ~ 0
A12
Text Label 7650 4150 0    60   ~ 0
A13
Text Label 7650 4250 0    60   ~ 0
A14
Text Label 7650 4350 0    60   ~ 0
A15
Entry Wire Line
	7800 2850 7900 2950
Entry Wire Line
	7800 2950 7900 3050
Entry Wire Line
	7800 3050 7900 3150
Entry Wire Line
	7800 3150 7900 3250
Entry Wire Line
	7800 3250 7900 3350
Entry Wire Line
	7800 3350 7900 3450
Entry Wire Line
	7800 3450 7900 3550
Entry Wire Line
	7800 3550 7900 3650
Entry Wire Line
	7800 3650 7900 3750
Entry Wire Line
	7800 3750 7900 3850
Entry Wire Line
	7800 3850 7900 3950
Entry Wire Line
	7800 3950 7900 4050
Entry Wire Line
	7800 4050 7900 4150
Entry Wire Line
	7800 4150 7900 4250
Entry Wire Line
	7800 4250 7900 4350
Entry Wire Line
	7800 4350 7900 4450
Wire Wire Line
	3000 2700 3200 2700
Wire Wire Line
	3000 2800 3200 2800
Wire Wire Line
	3000 2900 3200 2900
Wire Wire Line
	3000 3000 3200 3000
Wire Wire Line
	3000 3100 3200 3100
Wire Wire Line
	3000 3200 3200 3200
Wire Wire Line
	3000 3300 3200 3300
Wire Wire Line
	3000 3400 3200 3400
Wire Bus Line
	3300 3550 3300 1900
Wire Bus Line
	3300 1900 8050 1900
Wire Bus Line
	8050 1900 8050 5450
Entry Wire Line
	3200 2700 3300 2800
Entry Wire Line
	3200 2800 3300 2900
Entry Wire Line
	3200 2900 3300 3000
Entry Wire Line
	3200 3000 3300 3100
Entry Wire Line
	3200 3100 3300 3200
Entry Wire Line
	3200 3200 3300 3300
Entry Wire Line
	3200 3300 3300 3400
Entry Wire Line
	3200 3400 3300 3500
Text Label 3050 2700 0    60   ~ 0
D0
Text Label 3050 2800 0    60   ~ 0
D1
Text Label 3050 2900 0    60   ~ 0
D2
Text Label 3050 3000 0    60   ~ 0
D3
Text Label 3050 3100 0    60   ~ 0
D4
Text Label 3050 3200 0    60   ~ 0
D5
Text Label 3050 3300 0    60   ~ 0
D6
Text Label 3050 3400 0    60   ~ 0
D7
Wire Wire Line
	7600 4550 7950 4550
Wire Wire Line
	7600 4650 7950 4650
Wire Wire Line
	7600 4750 7950 4750
Wire Wire Line
	7600 4850 7950 4850
Wire Wire Line
	7600 4950 7950 4950
Wire Wire Line
	7600 5050 7950 5050
Wire Wire Line
	7600 5150 7950 5150
Wire Wire Line
	7600 5250 7950 5250
Entry Wire Line
	7950 4550 8050 4650
Entry Wire Line
	7950 4650 8050 4750
Entry Wire Line
	7950 4750 8050 4850
Entry Wire Line
	7950 4850 8050 4950
Entry Wire Line
	7950 4950 8050 5050
Entry Wire Line
	7950 5050 8050 5150
Entry Wire Line
	7950 5150 8050 5250
Entry Wire Line
	7950 5250 8050 5350
Text Label 7650 4550 0    60   ~ 0
D0
Text Label 7650 4650 0    60   ~ 0
D1
Text Label 7650 4750 0    60   ~ 0
D2
Text Label 7650 4850 0    60   ~ 0
D3
Text Label 7650 4950 0    60   ~ 0
D4
Text Label 7650 5050 0    60   ~ 0
D5
Text Label 7650 5150 0    60   ~ 0
D6
Text Label 7650 5250 0    60   ~ 0
D7
Wire Wire Line
	3000 3500 3700 3500
Wire Wire Line
	3700 3500 3700 3000
Wire Wire Line
	3700 3000 3900 3000
Wire Wire Line
	3000 3600 3750 3600
Wire Wire Line
	3750 3600 3750 3200
Wire Wire Line
	3750 3200 3900 3200
Wire Wire Line
	3000 3700 3800 3700
Wire Wire Line
	3800 3700 3800 3600
Wire Wire Line
	3800 3600 3900 3600
Wire Wire Line
	3000 3800 3900 3800
Text GLabel 6200 2850 0    60   Input ~ 0
RESET
Text GLabel 6200 3150 0    60   Input ~ 0
CLK
Text GLabel 6200 3450 0    60   Input ~ 0
NMI
Text GLabel 6200 3550 0    60   Input ~ 0
INT
Text GLabel 6200 3850 0    60   Input ~ 0
M1
Text GLabel 6200 4050 0    60   Input ~ 0
WAIT
Text GLabel 6200 4150 0    60   Input ~ 0
HALT
Text GLabel 6200 4550 0    60   Input ~ 0
RD
Text GLabel 6200 4650 0    60   Input ~ 0
WR
Text GLabel 6200 4750 0    60   Input ~ 0
MREQ
Text GLabel 6200 4850 0    60   Input ~ 0
IORQ
Text GLabel 6200 5150 0    60   Input ~ 0
BUSRQ
Text GLabel 6200 5250 0    60   Input ~ 0
BUSACK
Text GLabel 3000 3900 2    60   Input ~ 0
RESET
Text GLabel 3000 4000 2    60   Input ~ 0
CLK
Text GLabel 3000 4100 2    60   Input ~ 0
NMI
Text GLabel 3000 4200 2    60   Input ~ 0
INT
Text GLabel 3000 4300 2    60   Input ~ 0
M1
Text GLabel 3000 4400 2    60   Input ~ 0
WAIT
Text GLabel 3000 4500 2    60   Input ~ 0
HALT
Text GLabel 3000 4600 2    60   Input ~ 0
RD
Text GLabel 3000 4700 2    60   Input ~ 0
WR
Text GLabel 3000 4800 2    60   Input ~ 0
MREQ
Text GLabel 3000 4900 2    60   Input ~ 0
IORQ
Text GLabel 3000 5000 2    60   Input ~ 0
BUSRQ
Text GLabel 3000 5100 2    60   Input ~ 0
BUSACK
$Comp
L GND #PWR?
U 1 1 559D517F
P 2400 6050
F 0 "#PWR?" H 2400 5800 50  0001 C CNN
F 1 "GND" H 2400 5900 50  0000 C CNN
F 2 "" H 2400 6050 60  0000 C CNN
F 3 "" H 2400 6050 60  0000 C CNN
	1    2400 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 5650 2050 6050
Wire Wire Line
	2050 6050 2750 6050
Wire Wire Line
	2150 5650 2150 6050
Connection ~ 2150 6050
Wire Wire Line
	2250 5650 2250 6050
Connection ~ 2250 6050
Wire Wire Line
	2350 5650 2350 6050
Connection ~ 2350 6050
Wire Wire Line
	2450 6050 2450 5650
Connection ~ 2400 6050
Wire Wire Line
	2550 6050 2550 5650
Connection ~ 2450 6050
Wire Wire Line
	2650 6050 2650 5650
Connection ~ 2550 6050
Wire Wire Line
	2750 6050 2750 5650
Connection ~ 2650 6050
$Comp
L GND #PWR?
U 1 1 559D54C0
P 6900 5550
F 0 "#PWR?" H 6900 5300 50  0001 C CNN
F 1 "GND" H 6900 5400 50  0000 C CNN
F 2 "" H 6900 5550 60  0000 C CNN
F 3 "" H 6900 5550 60  0000 C CNN
	1    6900 5550
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 559D5584
P 2250 2050
F 0 "#PWR?" H 2250 1900 50  0001 C CNN
F 1 "+3.3V" H 2250 2190 50  0000 C CNN
F 2 "" H 2250 2050 60  0000 C CNN
F 3 "" H 2250 2050 60  0000 C CNN
	1    2250 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 2450 2350 2100
Wire Wire Line
	2350 2100 2250 2100
Connection ~ 2250 2100
Wire Wire Line
	2250 2100 2250 2450
$Comp
L +3.3V #PWR?
U 1 1 559D57EF
P 6900 2500
F 0 "#PWR?" H 6900 2350 50  0001 C CNN
F 1 "+3.3V" H 6900 2640 50  0000 C CNN
F 2 "" H 6900 2500 60  0000 C CNN
F 3 "" H 6900 2500 60  0000 C CNN
	1    6900 2500
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 559D5800
P 4500 2700
F 0 "#PWR?" H 4500 2550 50  0001 C CNN
F 1 "+3.3V" H 4500 2840 50  0000 C CNN
F 2 "" H 4500 2700 60  0000 C CNN
F 3 "" H 4500 2700 60  0000 C CNN
	1    4500 2700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 559D5811
P 4500 4750
F 0 "#PWR?" H 4500 4500 50  0001 C CNN
F 1 "GND" H 4500 4600 50  0000 C CNN
F 2 "" H 4500 4750 60  0000 C CNN
F 3 "" H 4500 4750 60  0000 C CNN
	1    4500 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 4000 3800 4000
NoConn ~ 3800 4000
$EndSCHEMATC
