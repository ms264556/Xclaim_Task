// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

;# Instructions:  Edit the "my" variables below as desired and then run tpp.
;#                For example:
;#                   tpp master.vec.tpp > master.vec
;# 
;my $MemBase = 0x5fb80000;      # Match the desired memory's base address
;my $BusByteWidth = 64;         # IRAM/IROM may not exceed 64
;my $DoSubWord = 1;             # Set to 0 for IRAM/IROM
;my $DoRCW = 0;                 # PIF only
;my $DoBlock = 0;               # PIF only
;my $DoAddressError = 0;        # PIF only
;#
;sub hx { sprintf("0x%08x", shift); }

// Set MASTER to be the sc_module name of your xtsc_master
#define MASTER=master

// Increase wait if the downstream device is slow/far-away, to ensure
// that the writes complete before the peeks occur that check on them
#define WAIT 1

info &grep End-Of-Script

info  
info  **************************************************************
info  *                    Testing POKE's                          *
info  **************************************************************
info  

/*
delay POKE  address    sz byt0 byt1 byt2 byt3 . . .  */
now   POKE  ` hx($MemBase + 0x0000)` 1  0x00
now   POKE  ` hx($MemBase + 0x0001)` 1  0x01
now   POKE  ` hx($MemBase + 0x0002)` 1  0x02
now   POKE  ` hx($MemBase + 0x0003)` 1  0x03
now   POKE  ` hx($MemBase + 0x0004)` 2  0x04 0x05
now   POKE  ` hx($MemBase + 0x0006)` 2  0x06 0x07
now   POKE  ` hx($MemBase + 0x0008)` 1  0x08
now   POKE  ` hx($MemBase + 0x0009)` 2  0x09 0x0a
now   POKE  ` hx($MemBase + 0x000b)` 2  0x0b 0x0c
now   POKE  ` hx($MemBase + 0x000d)` 3  0x0d 0x0e 0x0f
now   POKE  ` hx($MemBase + 0x0010)` 3  0x10 0x11 0x12
now   POKE  ` hx($MemBase + 0x0013)` 3  0x13 0x14 0x15
now   POKE  ` hx($MemBase + 0x0016)` 3  0x16 0x17 0x18
now   POKE  ` hx($MemBase + 0x0019)` 3  0x19 0x1a 0x1b
now   POKE  ` hx($MemBase + 0x001c)` 4  0x1c 0x1d 0x1e 0x1f
now   POKE  ` hx($MemBase + 0x0020)` 5  0x20 0x21 0x22 0x23 0x24
now   POKE  ` hx($MemBase + 0x0025)` 5  0x25 0x26 0x27 0x28 0x29
now   POKE  ` hx($MemBase + 0x002a)` 5  0x2a 0x2b 0x2c 0x2d 0x2e
now   POKE  ` hx($MemBase + 0x002f)` 5  0x2f 0x30 0x31 0x32 0x33
now   POKE  ` hx($MemBase + 0x0034)` 8  0x34 0x35 0x36 0x37 0x38 0x39 0x3a 0x3b
now   POKE  ` hx($MemBase + 0x003c)` 16 0x3c 0x3d 0x3e 0x3f 0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4a 0x4b
now   POKE  ` hx($MemBase + 0x004c)` 32 0x4c 0x4d 0x4e 0x4f 0x50 0x51 0x52 0x53 0x54 0x55 0x56 0x57 0x58 0x59 0x5a 0x5b 0x5c 0x5d 0x5e 0x5f 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67 0x68 0x69 0x6a 0x6b
now   POKE  ` hx($MemBase + 0x006c)` 64 0x6c 0x6d 0x6e 0x6f 0x70 0x71 0x72 0x73 0x74 0x75 0x76 0x77 0x78 0x79 0x7a 0x7b 0x7c 0x7d 0x7e 0x7f 0x80 0x81 0x82 0x83 0x84 0x85 0x86 0x87 0x88 0x89 0x8a 0x8b 0x8c 0x8d 0x8e 0x8f 0x90 0x91 0x92 0x93 0x94 0x95 0x96 0x97 0x98 0x99 0x9a 0x9b 0x9c 0x9d 0x9e 0x9f 0xa0 0xa1 0xa2 0xa3 0xa4 0xa5 0xa6 0xa7 0xa8 0xa9 0xaa 0xab

/*
delay PEEK  address    sz  */
info  Self-description:  &grep peek ` hx($MemBase + 0x0000)`: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f << Self-description
now   PEEK  ` hx($MemBase + 0x0000)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0010)`: 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f << Self-description
now   PEEK  ` hx($MemBase + 0x0010)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0020)`: 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f << Self-description
now   PEEK  ` hx($MemBase + 0x0020)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0030)`: 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f << Self-description
now   PEEK  ` hx($MemBase + 0x0030)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0040)`: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f << Self-description
now   PEEK  ` hx($MemBase + 0x0040)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0050)`: 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f << Self-description
now   PEEK  ` hx($MemBase + 0x0050)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0060)`: 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f << Self-description
now   PEEK  ` hx($MemBase + 0x0060)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0070)`: 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f << Self-description
now   PEEK  ` hx($MemBase + 0x0070)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0080)`: 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f << Self-description
now   PEEK  ` hx($MemBase + 0x0080)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x0090)`: 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f << Self-description
now   PEEK  ` hx($MemBase + 0x0090)` 16
info  Self-description:  &grep peek ` hx($MemBase + 0x00a0)`: a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab << Self-description
now   PEEK  ` hx($MemBase + 0x00a0)` 12
info  Self-description:  


info  
info  **************************************************************
info  *                    Testing PEEK's                          *
info  **************************************************************
info  

/*
delay PEEK  address    sz */
info  Self-description:  &grep peek ` hx($MemBase + 0x0000)`: 00 << Self-description
now   PEEK  ` hx($MemBase + 0x0000)` 1
info  Self-description:  &grep peek ` hx($MemBase + 0x0001)`: 01 << Self-description
now   PEEK  ` hx($MemBase + 0x0001)` 1
info  Self-description:  &grep peek ` hx($MemBase + 0x0002)`: 02 << Self-description
now   PEEK  ` hx($MemBase + 0x0002)` 1
info  Self-description:  &grep peek ` hx($MemBase + 0x0003)`: 03 << Self-description
now   PEEK  ` hx($MemBase + 0x0003)` 1
info  Self-description:  &grep peek ` hx($MemBase + 0x0004)`: 04 05 << Self-description
now   PEEK  ` hx($MemBase + 0x0004)` 2
info  Self-description:  &grep peek ` hx($MemBase + 0x0006)`: 06 07 << Self-description
now   PEEK  ` hx($MemBase + 0x0006)` 2
info  Self-description:  &grep peek ` hx($MemBase + 0x0008)`: 08 << Self-description
now   PEEK  ` hx($MemBase + 0x0008)` 1
info  Self-description:  &grep peek ` hx($MemBase + 0x0009)`: 09 0a << Self-description
now   PEEK  ` hx($MemBase + 0x0009)` 2
info  Self-description:  &grep peek ` hx($MemBase + 0x000b)`: 0b 0c << Self-description
now   PEEK  ` hx($MemBase + 0x000b)` 2
info  Self-description:  &grep peek ` hx($MemBase + 0x000d)`: 0d 0e 0f << Self-description
now   PEEK  ` hx($MemBase + 0x000d)` 3
info  Self-description:  &grep peek ` hx($MemBase + 0x0010)`: 10 11 12 << Self-description
now   PEEK  ` hx($MemBase + 0x0010)` 3
info  Self-description:  &grep peek ` hx($MemBase + 0x0013)`: 13 14 15 << Self-description
now   PEEK  ` hx($MemBase + 0x0013)` 3
info  Self-description:  &grep peek ` hx($MemBase + 0x0016)`: 16 17 18 << Self-description
now   PEEK  ` hx($MemBase + 0x0016)` 3
info  Self-description:  &grep peek ` hx($MemBase + 0x0019)`: 19 1a 1b << Self-description
now   PEEK  ` hx($MemBase + 0x0019)` 3
info  Self-description:  &grep peek ` hx($MemBase + 0x001c)`: 1c 1d 1e 1f << Self-description
now   PEEK  ` hx($MemBase + 0x001c)` 4
info  Self-description:  &grep peek ` hx($MemBase + 0x0020)`: 20 21 22 23 24 << Self-description
now   PEEK  ` hx($MemBase + 0x0020)` 5
info  Self-description:  &grep peek ` hx($MemBase + 0x0025)`: 25 26 27 28 29 << Self-description
now   PEEK  ` hx($MemBase + 0x0025)` 5
info  Self-description:  &grep peek ` hx($MemBase + 0x002a)`: 2a 2b 2c 2d 2e << Self-description
now   PEEK  ` hx($MemBase + 0x002a)` 5
info  Self-description:  &grep peek ` hx($MemBase + 0x002f)`: 2f 30 31 32 33 << Self-description
now   PEEK  ` hx($MemBase + 0x002f)` 5
info  Self-description:  &grep peek ` hx($MemBase + 0x0001)`: 01 02 03 04 05 06 07 << Self-description
now   PEEK  ` hx($MemBase + 0x0001)` 7
info  Self-description:  &grep peek ` hx($MemBase + 0x0034)`: 34 35 36 37 38 39 3a 3b << Self-description
now   PEEK  ` hx($MemBase + 0x0034)` 8  
info  Self-description:  &grep peek ` hx($MemBase + 0x003c)`: 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b << Self-description
now   PEEK  ` hx($MemBase + 0x003c)` 16 
info  Self-description:  &grep peek ` hx($MemBase + 0x004c)`: 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b << Self-description
now   PEEK  ` hx($MemBase + 0x004c)` 32 
info  Self-description:  &grep peek ` hx($MemBase + 0x006c)`: 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab << Self-description
now   PEEK  ` hx($MemBase + 0x006c)` 64 
info  Self-description:  


info  
info  **************************************************************
info  *                    Testing WRITE's                         *
info  **************************************************************
info  

// Clear contents
now   POKE  ` hx($MemBase + 0x0000)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0010)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0020)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0030)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0040)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0050)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0060)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0070)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0080)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x0090)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00a0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00b0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00c0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00d0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00e0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
now   POKE  ` hx($MemBase + 0x00f0)` 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

/*
delay WRITE address    sz rt id pr pc BE     byt0 byt1 byt2 byt3 */

; if ($DoSubWord) {
1     WRITE ` hx($MemBase + 0x0000)` 1  0  0  0  0  0x000F 0x00
wait  rsp
1     WRITE ` hx($MemBase + 0x0001)` 1  0  0  0  0  0x000F 0x01
wait  rsp
1     WRITE ` hx($MemBase + 0x0002)` 2  0  0  0  0  0x000F 0x02 0x03
wait  rsp
; } else {
1     WRITE ` hx($MemBase + 0x0000)` 4  0  0  0  0  0x000F 0x00 0x01 0x02 0x03
wait  rsp
; }
1     WRITE ` hx($MemBase + 0x0004)` 4  0  0  0  0  0x000F 0x04 0x05 0x06 0x07
wait  rsp
1     WRITE ` hx($MemBase + 0x0008)` 4  0  0  0  0  0x000F 0x08 0x09 0x0a 0x0b
wait  rsp
1     WRITE ` hx($MemBase + 0x000c)` 4  0  0  0  0  0x000F 0x0c 0x0d 0x0e 0x0f
wait  rsp

wait $(WAIT)
info  &grep peek ` hx($MemBase + 0x0000)`: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f << end-grep
now   PEEK  ` hx($MemBase + 0x0000)` 16
info  end-grep

;if ($BusByteWidth >= 8) {
1     WRITE ` hx($MemBase + 0x0010)` 8  0  0  0  0  0x00FF 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17
wait  rsp
1     WRITE ` hx($MemBase + 0x0018)` 8  0  0  0  0  0x00FF 0x18 0x19 0x1a 0x1b 0x1c 0x1d 0x1e 0x1f
wait  rsp

wait $(WAIT)
info  &grep peek ` hx($MemBase + 0x0010)`: 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f << end-grep
now   PEEK  ` hx($MemBase + 0x0010)` 16
info  end-grep

; }
; if ($BusByteWidth >= 16) {
1     WRITE ` hx($MemBase + 0x0020)` 16 0  0  0  0  0xFFFF 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28 0x29 0x2a 0x2b 0x2c 0x2d 0x2e 0x2f
wait  rsp
1     WRITE ` hx($MemBase + 0x0030)` 16 0  0  0  0  0xFFFF 0x30 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39 0x3a 0x3b 0x3c 0x3d 0x3e 0x3f
wait  rsp

wait $(WAIT)
info  &grep peek ` hx($MemBase + 0x0020)`: 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f << end-grep
now   PEEK  ` hx($MemBase + 0x0020)` 32
info  end-grep

; }
; if ($BusByteWidth >= 32) {
1     WRITE ` hx($MemBase + 0x0040)` 32 0  0  0  0  0xFFFFFFFF 0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4a 0x4b 0x4c 0x4d 0x4e 0x4f 0x50 0x51 0x52 0x53 0x54 0x55 0x56 0x57 0x58 0x59 0x5a 0x5b 0x5c 0x5d 0x5e 0x5f 
wait  rsp
1     WRITE ` hx($MemBase + 0x0060)` 32 0  0  0  0  0xFFFFFFFF 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67 0x68 0x69 0x6a 0x6b 0x6c 0x6d 0x6e 0x6f 0x70 0x71 0x72 0x73 0x74 0x75 0x76 0x77 0x78 0x79 0x7a 0x7b 0x7c 0x7d 0x7e 0x7f 
wait  rsp

wait $(WAIT)
info  &grep peek ` hx($MemBase + 0x0040)`: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f << end-grep
now   PEEK  ` hx($MemBase + 0x0040)` 64
info  end-grep

; }


; if ($BusByteWidth >= 64) {
1     WRITE ` hx($MemBase + 0x0080)` 64 0  0  0  0  0xFFFFFFFFFFFFFFFF 0x80 0x81 0x82 0x83 0x84 0x85 0x86 0x87 0x88 0x89 0x8a 0x8b 0x8c 0x8d 0x8e 0x8f 0x90 0x91 0x92 0x93 0x94 0x95 0x96 0x97 0x98 0x99 0x9a 0x9b 0x9c 0x9d 0x9e 0x9f 0xa0 0xa1 0xa2 0xa3 0xa4 0xa5 0xa6 0xa7 0xa8 0xa9 0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 0xb7 0xb8 0xb9 0xba 0xbb 0xbc 0xbd 0xbe 0xbf
wait  rsp
1     WRITE ` hx($MemBase + 0x00c0)` 64 0  0  0  0  0xFFFFFFFFFFFFFFFF 0xc0 0xc1 0xc2 0xc3 0xc4 0xc5 0xc6 0xc7 0xc8 0xc9 0xca 0xcb 0xcc 0xcd 0xce 0xcf 0xd0 0xd1 0xd2 0xd3 0xd4 0xd5 0xd6 0xd7 0xd8 0xd9 0xda 0xdb 0xdc 0xdd 0xde 0xdf 0xe0 0xe1 0xe2 0xe3 0xe4 0xe5 0xe6 0xe7 0xe8 0xe9 0xea 0xeb 0xec 0xed 0xee 0xef 0xf0 0xf1 0xf2 0xf3 0xf4 0xf5 0xf6 0xf7 0xf8 0xf9 0xfa 0xfb 0xfc 0xfd 0xfe 0xff
wait  rsp

wait $(WAIT)
info  &grep peek ` hx($MemBase + 0x0080)`: 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff << end-grep
now   PEEK  ` hx($MemBase + 0x0080)` 128
info  end-grep

; }


; if ($DoAddressError) {
info  &grep RSP_ADDRESS_ERROR << Self-description
// This should fail 
1     WRITE ` hx($MemBase + 0x0009)`  4 0  0  0  0  0x00FF 0x08 0x09 0x0a 0x0b 
wait  rsp

wait $(WAIT)
info  Self-description:  &grep 00 01 02 03 04 05 06 07 00 00 *$ << Self-description
now   PEEK  ` hx($MemBase + 0x0000)` 10
info  Self-description:  



; }
info  
info  **************************************************************
info  *                    Testing READ's                          *
info  **************************************************************
info  

/*
delay READ  address    sz rt id pr pc  */

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 00 *$ << Self-description
1     READ  ` hx($MemBase + 0x0000)` 1  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 01 *$ << Self-description
1     READ  ` hx($MemBase + 0x0001)` 1  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 02 03 *$ << Self-description
1     READ  ` hx($MemBase + 0x0002)` 2  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 04 05 06 07 *$ << Self-description
1     READ  ` hx($MemBase + 0x0004)` 4  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 08 09 0a 0b *$ << Self-description
1     READ  ` hx($MemBase + 0x0008)` 4  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 0c 0d 0e 0f *$ << Self-description
1     READ  ` hx($MemBase + 0x000c)` 4  0  0  0  0  
wait  rsp

;if ($BusByteWidth >= 8) {
info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 10 11 12 13 14 15 16 17 *$ << Self-description
1     READ  ` hx($MemBase + 0x0010)` 8  0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 18 19 1a 1b 1c 1d 1e 1f *$ << Self-description
1     READ  ` hx($MemBase + 0x0018)` 8  0  0  0  0  
wait  rsp

; }
; if ($BusByteWidth >= 16) {
info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f *$ << Self-description
1     READ  ` hx($MemBase + 0x0020)` 16 0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f *$ << Self-description
1     READ  ` hx($MemBase + 0x0030)` 16 0  0  0  0  
wait  rsp

; }
; if ($BusByteWidth >= 32) {
info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f *$ << Self-description
1     READ  ` hx($MemBase + 0x0040)` 32 0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f *$ << Self-description
1     READ  ` hx($MemBase + 0x0060)` 32 0  0  0  0  
wait  rsp

; }
; if ($BusByteWidth >= 64) {
info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf *$ << Self-description
1     READ  ` hx($MemBase + 0x0080)` 64 0  0  0  0  
wait  rsp

info  Self-description:  &grep INFO.*$(MASTER).*RSP_OK.*: c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff *$ << Self-description
1     READ  ` hx($MemBase + 0x00c0)` 64 0  0  0  0  
wait  rsp

; }
info  Self-description



; if ($DoAddressError) {
info  Self-description:  &grep RSP_ADDRESS_ERROR << Self-description
// This should fail 
1     READ  ` hx($MemBase + 0x0009)` 4  0  0  0  0  
wait  rsp
info  Self-description:  



; }
; if ($DoRCW) {
info  
info  **************************************************************
info  *                    Testing RCW's                           *
info  **************************************************************
info  

/*
delay RCW         address    sz rt id pr pc num last byt0 byt1 byt2 byt3 */
10    RCW         ` hx($MemBase + 0x2004)`  4 0  0  0  0  2   0    0xba 0xbe 0xfa 0xce
1     RCW         ` hx($MemBase + 0x2004)`  4 0  0  0  0  2   1    0xca 0xfe 0xf0 0x0d
wait  rsp

wait $(WAIT)
info  Self-description:  &grep! ca fe f0 0d << Self-description
now   PEEK  ` hx($MemBase + 0x2004)` 4
info  Self-description:  

now   POKE  ` hx($MemBase + 0x2004)` 4 0xba 0xbe 0xfa 0xce

/*
delay RCW         address    sz rt id pr pc num last byt0 byt1 byt2 byt3 */
10    RCW         ` hx($MemBase + 0x2004)`  4 0  0  0  0  2   0    0xba 0xbe 0xfa 0xce
1     RCW         ` hx($MemBase + 0x2004)`  4 0  0  0  0  2   1    0xca 0xfe 0xf0 0x0d
wait  rsp

wait $(WAIT)
info  Self-description:  &grep ca fe f0 0d *$ << Self-description
now   PEEK  ` hx($MemBase + 0x2004)` 4
info  Self-description:  




; }
; if ($DoBlock) {
info  
info  **************************************************************
info  *                Testing BLOCK_WRITE's                       *
info  **************************************************************
info  

;if ($BusByteWidth == 4) {
/*
delay BLOCK_WRITE address    sz rt id pr pc num last first  b0 b1 b2 b3 */

10    BLOCK_WRITE ` hx($MemBase + 0x3000)`  4 0  0  0  0  4   0    1       0  1  2  3
10    BLOCK_WRITE ` hx($MemBase + 0x3004)`  4 0  0  0  0  4   0    0       4  5  6  7
10    BLOCK_WRITE ` hx($MemBase + 0x3008)`  4 0  0  0  0  4   0    0       8  9 10 11
10    BLOCK_WRITE ` hx($MemBase + 0x300C)`  4 0  0  0  0  4   1    0      12 13 14 15
wait  rsp

wait $(WAIT)
info  Self-description:  &grep 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f *$ << Self-description
now   PEEK  ` hx($MemBase + 0x3000)` 16
info  Self-description:  

; } elsif ($BusByteWidth == 8) {
/*
delay BLOCK_WRITE address    sz rt id pr pc num last first  b0 b1 b2 b3 b4 b5 b6 b7 b8 */

10    BLOCK_WRITE ` hx($MemBase + 0x3000)`  8 0  0  0  0  4   0    1       0  1  2  3  4  5  6  7
10    BLOCK_WRITE ` hx($MemBase + 0x3008)`  8 0  0  0  0  4   0    0       8  9 10 11 12 13 14 15
10    BLOCK_WRITE ` hx($MemBase + 0x3010)`  8 0  0  0  0  4   0    0      16 17 18 19 20 21 22 23
10    BLOCK_WRITE ` hx($MemBase + 0x3018)`  8 0  0  0  0  4   1    0      24 25 26 27 28 29 30 31
wait  rsp

wait $(WAIT)
info  Self-description:  &grep 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f *$ << Self-description
now   PEEK  ` hx($MemBase + 0x3000)` 32
info  Self-description:  

; } else {
/*
delay BLOCK_WRITE address    sz rt id pr pc num last first  b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 . . .*/

10    BLOCK_WRITE ` hx($MemBase + 0x3000)` 16 0  0  0  0  4   0    1       0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
10    BLOCK_WRITE ` hx($MemBase + 0x3010)` 16 0  0  0  0  4   0    0      16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
10    BLOCK_WRITE ` hx($MemBase + 0x3020)` 16 0  0  0  0  4   0    0      32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
10    BLOCK_WRITE ` hx($MemBase + 0x3030)` 16 0  0  0  0  4   1    0      48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
wait  rsp

wait $(WAIT)
info  Self-description:  &grep 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f *$ << Self-description
now   PEEK  ` hx($MemBase + 0x3000)` 64
info  Self-description:  
info  ---------------------------------------------------------------------------------

; }

info  
info  **************************************************************
info  *                Testing BLOCK_READ's                        *
info  **************************************************************
info  

/*
delay BLOCK_READ  address    sz rt id pr pc num */
;if ($BusByteWidth == 4) {
10    BLOCK_READ  ` hx($MemBase + 0x3000)`  4 0  0  0  0  4   
info  Self-description:  &grep 00 01 02 03 *$ << Self-description
wait  rsp
info  Self-description:  &grep 04 05 06 07 *$ << Self-description
wait  rsp
info  Self-description:  &grep 08 09 0a 0b *$ << Self-description
wait  rsp
info  Self-description:  &grep 0c 0d 0e 0f *$ << Self-description
wait  rsp
info  Self-description:
; } elsif ($BusByteWidth == 8) {
10    BLOCK_READ  ` hx($MemBase + 0x3000)`  8 0  0  0  0  4   
info  Self-description:  &grep 00 01 02 03 04 05 06 07 *$ << Self-description
wait  rsp
info  Self-description:  &grep 08 09 0a 0b 0c 0d 0e 0f *$ << Self-description
wait  rsp
info  Self-description:  &grep 10 11 12 13 14 15 16 17 *$ << Self-description
wait  rsp
info  Self-description:  &grep 18 19 1a 1b 1c 1d 1e 1f *$ << Self-description
wait  rsp
info  Self-description:
; } else {
10    BLOCK_READ  ` hx($MemBase + 0x3000)` 16 0  0  0  0  4   
info  Self-description:  &grep 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f *$ << Self-description
wait  rsp
info  Self-description:  &grep 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f *$ << Self-description
wait  rsp
info  Self-description:  &grep 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f *$ << Self-description
wait  rsp
info  Self-description:  &grep 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f *$ << Self-description
wait  rsp
info  Self-description:
; }



; }
wait 10
info End-Of-Script
now stop

