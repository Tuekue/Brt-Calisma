EESchema Schematic File Version 4
LIBS:Lamba-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "sam. 04 avril 2015"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 10350 1500 1    60   ~ 0
RAW
Text Label 9550 1000 1    60   ~ 0
DTR
Text Label 9750 1000 1    60   ~ 0
0(Rx)
Text Label 9000 1600 0    60   ~ 0
1(Tx)
Text Label 9000 1700 0    60   ~ 0
0(Rx)
Text Label 9000 1800 0    60   ~ 0
Reset
Text Label 10500 1800 0    60   ~ 0
Reset
Text Label 9000 2000 0    60   ~ 0
2
Text Label 9000 2100 0    60   ~ 0
3(**)
Text Label 9000 2200 0    60   ~ 0
4
Text Label 9000 2300 0    60   ~ 0
5(**)
Text Label 9000 2400 0    60   ~ 0
6(**)
Text Label 9000 2500 0    60   ~ 0
7
Text Label 9000 2600 0    60   ~ 0
8
Text Label 9000 2700 0    60   ~ 0
9(**)
Text Label 10500 2700 0    60   ~ 0
10(**/SS)
Text Label 10500 2600 0    60   ~ 0
11(**/MOSI)
Text Label 10500 2500 0    60   ~ 0
12(MISO)
Text Label 10500 2400 0    60   ~ 0
13(SCK)
Text Label 10500 2300 0    60   ~ 0
A0
Text Label 10500 2200 0    60   ~ 0
A1
Text Label 10500 2100 0    60   ~ 0
A2
Text Label 10500 2000 0    60   ~ 0
A3
Text Label 9650 3200 3    60   ~ 0
A4
Text Label 9550 3200 3    60   ~ 0
A5
Text Notes 8550 575  0    60   ~ 0
Shield Arduino Pro Mini \n
Wire Notes Line
	8525 650  9700 650 
Wire Notes Line
	9700 650  9700 475 
Text Label 9950 3200 3    60   ~ 0
A7
Text Label 10050 3200 3    60   ~ 0
A6
$Comp
L Connector_Generic:Conn_01x02 P3
U 1 1 56D74FB3
P 9550 3000
F 0 "P3" H 9550 3150 50  0000 C CNN
F 1 "ADC" V 9650 3000 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x02" H 9550 3000 50  0001 C CNN
F 3 "" H 9550 3000 50  0000 C CNN
	1    9550 3000
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 P5
U 1 1 56D7505C
P 9950 3000
F 0 "P5" H 9950 3150 50  0000 C CNN
F 1 "ADC" V 10050 3000 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x02" H 9950 3000 50  0001 C CNN
F 3 "" H 9950 3000 50  0000 C CNN
	1    9950 3000
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x06 P2
U 1 1 56D75238
P 9750 1200
F 0 "P2" H 9750 1550 50  0000 C CNN
F 1 "COM" V 9850 1200 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x06" H 9750 1200 50  0001 C CNN
F 3 "" H 9750 1200 50  0000 C CNN
	1    9750 1200
	0    -1   1    0   
$EndComp
Text Label 9650 1000 1    60   ~ 0
1(Tx)
$Comp
L power:GND #PWR01
U 1 1 56D7539A
P 10200 1000
F 0 "#PWR01" H 10200 750 50  0001 C CNN
F 1 "GND" H 10200 850 50  0000 C CNN
F 2 "" H 10200 1000 50  0000 C CNN
F 3 "" H 10200 1000 50  0000 C CNN
	1    10200 1000
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR02
U 1 1 56D753B8
P 9850 1000
F 0 "#PWR02" H 9850 850 50  0001 C CNN
F 1 "VCC" H 9850 1150 50  0000 C CNN
F 2 "" H 9850 1000 50  0000 C CNN
F 3 "" H 9850 1000 50  0000 C CNN
	1    9850 1000
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x12 P1
U 1 1 56D754D1
P 9550 2100
F 0 "P1" H 9550 2750 50  0000 C CNN
F 1 "Digital" V 9650 2100 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x12" H 9550 2100 50  0001 C CNN
F 3 "" H 9550 2100 50  0000 C CNN
	1    9550 2100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x12 P4
U 1 1 56D755F3
P 10050 2100
F 0 "P4" H 10050 2750 50  0000 C CNN
F 1 "Analog" V 10150 2100 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x12" H 10050 2100 50  0001 C CNN
F 3 "" H 10050 2100 50  0000 C CNN
	1    10050 2100
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 56D756B8
P 9250 2800
F 0 "#PWR03" H 9250 2550 50  0001 C CNN
F 1 "GND" H 9250 2650 50  0000 C CNN
F 2 "" H 9250 2800 50  0000 C CNN
F 3 "" H 9250 2800 50  0000 C CNN
	1    9250 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	9350 1900 9250 1900
Wire Wire Line
	9250 1900 9250 2800
Wire Wire Line
	9350 1600 9000 1600
Wire Wire Line
	9000 1700 9350 1700
Wire Wire Line
	9350 1800 9000 1800
Wire Wire Line
	9000 2000 9350 2000
Wire Wire Line
	9000 2500 9350 2500
Wire Wire Line
	9350 2600 9000 2600
Wire Wire Line
	10350 1500 10350 1600
Wire Wire Line
	10350 1600 10250 1600
Wire Wire Line
	10250 1700 10350 1700
Wire Wire Line
	10350 1700 10350 2800
$Comp
L power:GND #PWR04
U 1 1 56D75A03
P 10350 2800
F 0 "#PWR04" H 10350 2550 50  0001 C CNN
F 1 "GND" H 10350 2650 50  0000 C CNN
F 2 "" H 10350 2800 50  0000 C CNN
F 3 "" H 10350 2800 50  0000 C CNN
	1    10350 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	10250 1900 10450 1900
Wire Wire Line
	10450 1900 10450 1500
$Comp
L power:VCC #PWR05
U 1 1 56D75A9D
P 10450 1500
F 0 "#PWR05" H 10450 1350 50  0001 C CNN
F 1 "VCC" H 10450 1650 50  0000 C CNN
F 2 "" H 10450 1500 50  0000 C CNN
F 3 "" H 10450 1500 50  0000 C CNN
	1    10450 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	10250 1800 10500 1800
Wire Wire Line
	10500 2000 10250 2000
Wire Wire Line
	10250 2100 10500 2100
Wire Wire Line
	10500 2200 10250 2200
Wire Wire Line
	10250 2300 10500 2300
Wire Notes Line
	11200 3400 8500 3400
Wire Notes Line
	8500 3400 8500 500 
Text Notes 9650 1600 0    60   ~ 0
1
Wire Wire Line
	9950 1000 9950 950 
Wire Wire Line
	9950 950  10050 950 
Wire Wire Line
	10050 950  10050 1000
Wire Wire Line
	10050 950  10200 950 
Wire Wire Line
	10200 950  10200 1000
Connection ~ 10050 950 
$Comp
L pspice:DIODE D1
U 1 1 5C1003B6
P 1150 3500
F 0 "D1" V 1196 3372 50  0000 R CNN
F 1 "DIODE" V 1105 3372 50  0000 R CNN
F 2 "" H 1150 3500 50  0001 C CNN
F 3 "~" H 1150 3500 50  0001 C CNN
	1    1150 3500
	-1   0    0    1   
$EndComp
$Comp
L Transistor_BJT:2N2219 Q1
U 1 1 5C1004F8
P 1800 3500
F 0 "Q1" H 1991 3546 50  0000 L CNN
F 1 "2N2219" H 1991 3455 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-39-3" H 2000 3425 50  0001 L CIN
F 3 "http://www.onsemi.com/pub_link/Collateral/2N2219-D.PDF" H 1800 3500 50  0001 L CNN
	1    1800 3500
	0    -1   1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 5C10061D
P 1800 3100
F 0 "R1" V 1593 3100 50  0000 C CNN
F 1 "4.7K" V 1684 3100 50  0000 C CNN
F 2 "" V 1730 3100 50  0001 C CNN
F 3 "~" H 1800 3100 50  0001 C CNN
	1    1800 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  4100 750  4100
Wire Wire Line
	750  4100 750  3500
Wire Wire Line
	750  3500 950  3500
Wire Wire Line
	1350 3500 1500 3500
Wire Wire Line
	1500 3500 1500 4100
Wire Wire Line
	1800 3250 1800 3300
Wire Wire Line
	1600 3600 1600 4100
Wire Wire Line
	1450 4100 1500 4100
Connection ~ 1500 4100
Wire Wire Line
	1500 4100 1600 4100
$Comp
L Relay:DIPxx-1Axx-11x K1
U 1 1 5C103E68
P 1150 4300
F 0 "K1" V 1600 4300 50  0000 L CNN
F 1 "DIP01-1A01-LAMP" V 1500 3950 50  0000 L CNN
F 2 "Relay_THT:Relay_StandexMeder_DIP_LowProfile" H 1500 4250 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 1150 4300 50  0001 C CNN
	1    1150 4300
	0    -1   -1   0   
$EndComp
$Comp
L power:+24V #PWR0101
U 1 1 5C103ED6
P 600 5300
F 0 "#PWR0101" H 600 5150 50  0001 C CNN
F 1 "+24V" H 615 5473 50  0000 C CNN
F 2 "" H 600 5300 50  0001 C CNN
F 3 "" H 600 5300 50  0001 C CNN
	1    600  5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	600  5500 850  5500
Wire Wire Line
	850  5500 850  4500
Wire Wire Line
	1600 4500 1600 6900
$Comp
L pspice:DIODE D2
U 1 1 5C10A940
P 2500 3500
F 0 "D2" V 2546 3372 50  0000 R CNN
F 1 "DIODE" V 2455 3372 50  0000 R CNN
F 2 "" H 2500 3500 50  0001 C CNN
F 3 "~" H 2500 3500 50  0001 C CNN
	1    2500 3500
	-1   0    0    1   
$EndComp
$Comp
L Transistor_BJT:2N2219 Q2
U 1 1 5C10A947
P 3150 3500
F 0 "Q2" H 3341 3546 50  0000 L CNN
F 1 "2N2219" H 3341 3455 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-39-3" H 3350 3425 50  0001 L CIN
F 3 "http://www.onsemi.com/pub_link/Collateral/2N2219-D.PDF" H 3150 3500 50  0001 L CNN
	1    3150 3500
	0    -1   1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5C10A94E
P 3150 3100
F 0 "R2" V 2943 3100 50  0000 C CNN
F 1 "4.7K" V 3034 3100 50  0000 C CNN
F 2 "" V 3080 3100 50  0001 C CNN
F 3 "~" H 3150 3100 50  0001 C CNN
	1    3150 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 4100 2100 4100
Wire Wire Line
	2100 4100 2100 3500
Wire Wire Line
	2100 3500 2300 3500
Wire Wire Line
	2700 3500 2850 3500
Wire Wire Line
	2850 3500 2850 4100
Wire Wire Line
	3150 3250 3150 3300
Wire Wire Line
	2950 3600 2950 4100
Wire Wire Line
	2800 4100 2850 4100
Connection ~ 2850 4100
Wire Wire Line
	2850 4100 2950 4100
$Comp
L Relay:DIPxx-1Axx-11x K2
U 1 1 5C10A960
P 2500 4300
F 0 "K2" V 2950 4300 50  0000 L CNN
F 1 "DIPx01-1A02-FAN" V 2850 3950 50  0000 L CNN
F 2 "Relay_THT:Relay_StandexMeder_DIP_LowProfile" H 2850 4250 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 2500 4300 50  0001 C CNN
	1    2500 4300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2200 5500 2200 4500
Wire Wire Line
	2950 4500 2950 6900
Wire Wire Line
	850  5500 2200 5500
Connection ~ 850  5500
Wire Wire Line
	3350 3600 3350 5050
Wire Wire Line
	2000 3600 2000 5050
Wire Wire Line
	2000 5050 3350 5050
Wire Wire Line
	9350 2100 1800 2100
Wire Wire Line
	1800 2100 1800 2950
Wire Wire Line
	9350 2200 3150 2200
Wire Wire Line
	3150 2200 3150 2950
$Comp
L RF:NRF24L01_Breakout U1
U 1 1 5C178728
P 9450 4650
F 0 "U1" V 9972 4581 50  0000 C CNN
F 1 "NRF24L01_Breakout" V 10063 4581 50  0000 C CNN
F 2 "RF_Module:nRF24L01_Breakout" H 9600 5250 50  0001 L CIN
F 3 "http://www.nordicsemi.com/eng/content/download/2730/34105/file/nRF24L01_Product_Specification_v2_0.pdf" H 9450 4550 50  0001 C CNN
	1    9450 4650
	0    1    1    0   
$EndComp
Wire Wire Line
	10650 4650 10050 4650
Wire Wire Line
	9750 4150 9750 3900
Wire Wire Line
	9750 3900 11000 3900
Wire Wire Line
	11000 3900 11000 2600
Wire Wire Line
	10250 2600 11000 2600
Wire Wire Line
	9650 3800 10900 3800
Wire Wire Line
	10900 3800 10900 2500
Wire Wire Line
	10250 2500 10900 2500
Wire Wire Line
	9650 3800 9650 4150
Wire Wire Line
	9550 3700 10800 3700
Wire Wire Line
	10800 3700 10800 2400
Wire Wire Line
	10250 2400 10800 2400
Wire Wire Line
	9550 3700 9550 4150
Wire Wire Line
	9450 4150 9450 3600
Wire Wire Line
	9450 3600 10700 3600
Wire Wire Line
	10700 3600 10700 2700
Wire Wire Line
	10250 2700 10700 2700
Wire Wire Line
	9250 4150 9250 3600
Wire Wire Line
	9250 3600 8900 3600
Wire Wire Line
	8900 3600 8900 2700
Wire Wire Line
	8900 2700 9350 2700
$Comp
L MCU_Module:CJMCU-VL53L0X U2
U 1 1 5C1356E8
P 6900 3600
F 0 "U2" H 7377 3146 50  0000 L CNN
F 1 "CJMCU-VL53L0X" H 7377 3055 50  0000 L CNN
F 2 "CJMCU-530 VL53L0X" H 7100 3600 50  0001 C CNN
F 3 "" H 7100 3600 50  0001 C CNN
	1    6900 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 3950 6400 3950
Wire Wire Line
	6400 3950 6400 4900
Wire Wire Line
	6400 4900 8500 4900
Wire Wire Line
	8500 4900 8500 3500
Wire Wire Line
	8500 3500 9650 3500
Wire Wire Line
	9650 3500 9650 3200
Wire Wire Line
	6800 4050 6500 4050
Wire Wire Line
	6500 4050 6500 4800
Wire Wire Line
	6500 4800 8400 4800
Wire Wire Line
	8400 4800 8400 3300
Wire Wire Line
	8400 3300 9550 3300
Wire Wire Line
	9550 3300 9550 3200
$Comp
L power:GND #PWR0103
U 1 1 5C140049
P 7350 5700
F 0 "#PWR0103" H 7350 5450 50  0001 C CNN
F 1 "GND" H 7355 5527 50  0000 C CNN
F 2 "" H 7350 5700 50  0001 C CNN
F 3 "" H 7350 5700 50  0001 C CNN
	1    7350 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 4650 8700 5700
Wire Wire Line
	6800 3850 6300 3850
Wire Wire Line
	6300 3850 6300 4250
Wire Wire Line
	6300 5300 7350 5300
Wire Wire Line
	7350 5300 7350 5700
Connection ~ 7350 5700
Wire Wire Line
	6800 4250 6300 4250
Connection ~ 6300 4250
Wire Wire Line
	6300 4250 6300 5300
$Comp
L power:VCC #PWR0104
U 1 1 5C14ABD4
P 10650 4550
F 0 "#PWR0104" H 10650 4400 50  0001 C CNN
F 1 "VCC" H 10667 4723 50  0000 C CNN
F 2 "" H 10650 4550 50  0001 C CNN
F 3 "" H 10650 4550 50  0001 C CNN
	1    10650 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	600  5500 600  5300
Wire Wire Line
	1600 4500 1450 4500
Wire Wire Line
	2950 4500 2800 4500
Wire Wire Line
	8700 5700 7350 5700
Wire Wire Line
	8850 4650 8700 4650
Wire Wire Line
	10650 4650 10650 4550
$Comp
L power:+5V #PWR0102
U 1 1 5C17215F
P 6250 3200
F 0 "#PWR0102" H 6250 3050 50  0001 C CNN
F 1 "+5V" H 6265 3373 50  0000 C CNN
F 2 "" H 6250 3200 50  0001 C CNN
F 3 "" H 6250 3200 50  0001 C CNN
	1    6250 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 3200 6250 3750
Wire Wire Line
	6250 3750 6800 3750
$EndSCHEMATC