/**
 * @file roomba.h
 * @author Richard B. Wagner
 * @date 2016-04-04
 */

/*******************************************************************************
 * iRobot® Create® 2 Open Interface (OI)
 * Specification based on the iRobot® Roomba® 600
 ******************************************************************************/

#ifndef ROOMBA_H_
#define ROOMBA_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief OP_CODE enum
 *
 * Operation codes for roomba
 */
typedef enum _op_code {
  /**
   * Reset | Opcode: 7 | Data Bytes: 0
   * This command resets the robot, as if you had removed and reinserted the
   * battery.
   *   - Serial sequence: [7].
   *   - Available in modes: Always available.
   *   - Changes mode to: Off. You will have to send [128] again to re-enter
   *     Open Interface mode.
   */
  RESET = 7,

  /**
   * Start | Opcode: 128 | Data Bytes: 0
   * This command starts the OI. You must always send the Start command before
   * sending any other commands to the OI.
   *   - Serial sequence: [128].
   *   - Available in modes: Passive, Safe, or Full
   *   - Changes mode to: Passive. Roomba beeps once to acknowledge it is
   *     starting * from “off” mode.
   */
  START = 128,

  /**
   * Baud | Opcode: 129 | Data Bytes: 1
   * This command sets the baud rate in bits per second (bps) at which OI
   * commands and data are sent according to the baud code sent in the data
   * byte. The default baud rate at power up is 115200 bps, but the starting
   * baud rate can be changed to 19200 by following the method outlined on page
   * 4. Once the baud rate is changed, it persists until Roomba is power cycled
   * by pressing the power button or removing the battery, or when the battery
   * voltage falls below the minimum required for processor operation. You
   * must wait 100ms after sending this command before sending additional
   * commands at the new baud rate.
   *   - Serial sequence: [129][Baud Code]
   *   - Available in modes: Passive, Safe, or Full
   *   - Changes mode to: No Change
   *   - Baud data byte 1: Baud Code (0 - 11)
   */
  BAUD = 129,

  /**
  This command sets the baud rate in bits per second (bps) at which OI commands and data are sent
  according to the baud code sent in the data byte. The default baud rate at power up is 115200 bps, but
  the starting baud rate can be changed to 19200 by following the method outlined on page 4. Once the
  baud rate is changed, it persists until Roomba is power cycled by pressing the power button or removing
  the battery, or when the battery voltage falls below the minimum required for processor operation. You
  must wait 100ms after sending this command before sending additional commands at the new baud rate.
   Serial sequence: [129][Baud Code]
   Available in modes: Passive, Safe, or Full
   Changes mode to: No Change
   Baud data byte 1: Baud Code (0 - 11)

  Baud Code Baud Rate in BPS
0 300
1 600
2 1200
3 2400
4 4800
5 9600
6 14400
7 19200
8 28800
9 38400
10 57600
11 115200
  */
  CONTROL,

  /**
  Safe Opcode: 131 Data Bytes: 0
  This command puts the OI into Safe mode, enabling user control of Roomba. It turns off all LEDs. The OI
  can be in Passive, Safe, or Full mode to accept this command. If a safety condition occurs (see above)
  Roomba reverts automatically to Passive mode.
   Serial sequence: [131]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Safe
  Note: The effect and usage of the Control command (130) are identical to the Safe command (131).
  */
  SAFE,

  /**
  Full Opcode: 132 Data Bytes: 0
  This command gives you complete control over Roomba by putting the OI into Full mode, and turning off
  the cliff, wheel-drop and internal charger safety features. That is, in Full mode, Roomba executes any
  command that you send it, even if the internal charger is plugged in, or command triggers a cliff or wheel
  drop condition.
   Serial sequence: [132]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Full
  Note: Use the Start command (128) to change the mode to Passive
  */
  FULL,

  /**
  Power Opcode: 133 Data Bytes: 0
  This command powers down Roomba. The OI can be in Passive, Safe, or Full mode to accept this
  command.
   Serial sequence: [133]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Passive
  */
  POWER,

  /**
  Spot Opcode: 134 Data Bytes: 0
  This command starts the Spot cleaning mode. This is the same as pressing Roomba’s Spot button, and
  will pause a cleaning cycle if one is already in progress.
   Serial sequence: [134]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Passive
  */
  SPOT,

  /**
  Clean Opcode: 135 Data Bytes: 0
  This command starts the default cleaning mode. This is the same as pressing Roomba’s Clean button,
  and will pause a cleaning cycle if one is already in progress.
   Serial sequence: [135]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Passive
  */
  CLEAN,

  /**
  Max Opcode: 136 Data Bytes: 0
  This command starts the Max cleaning mode, which will clean until the battery is dead. This command
  will pause a cleaning cycle if one is already in progress.
   Serial sequence: [136]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Passive
  */
  MAX,

  /**
  Drive Opcode: 137 Data Bytes: 4
  This command controls Roomba’s drive wheels. It takes four data bytes, interpreted as two 16-bit signed
  values using two’s complement. (http://en.wikipedia.org/wiki/Two%27s_complement) The first two bytes
  specify the average velocity of the drive wheels in millimeters per second (mm/s), with the high byte
  being sent first. The next two bytes specify the radius in millimeters at which Roomba will turn. The
  longer radii make Roomba drive straighter, while the shorter radii make Roomba turn more. The radius is
  measured from the center of the turning circle to the center of Roomba. A Drive command with a
  positive velocity and a positive radius makes Roomba drive forward while turning toward the left. A
  negative radius makes Roomba turn toward the right. Special cases for the radius make Roomba turn in
  place or drive straight, as specified below. A negative velocity makes Roomba drive backward.
  NOTE:
  Internal and environmental restrictions may prevent Roomba from accurately carrying out some drive
  commands. For example, it may not be possible for Roomba to drive at full speed in an arc with a large
  radius of curvature.
   Serial sequence: [137] [Velocity high byte] [Velocity low byte] [Radius high byte] [Radius low byte]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Velocity (-500 – 500 mm/s)
   Radius (-2000 – 2000 mm)
  Special cases:
  Straight = 32768 or 32767 = 0x8000 or 0x7FFF
  Turn in place clockwise = -1 = 0xFFFF
  Turn in place counter-clockwise = 1 = 0x0001
  Example:
  To drive in reverse at a velocity of -200 mm/s while turning at a radius of 500mm, send the following
  serial byte sequence:
  [137] [255] [56] [1] [244]
  Explanation:
  Desired value  two’s complement and convert to hex  split into 2 bytes  convert to decimal
  Velocity = -200 = 0xFF38 = [0xFF] [0x38] = [255] [56]
  Radius = 500 = 0x01F4 = [0x01] [0xF4] = [1] [244]
  */
  DRIVE,

  /**
  Motors Opcode: 138 Data Bytes: 1
  This command lets you control the forward and backward motion of Roomba’s main brush, side brush,
  and vacuum independently. Motor velocity cannot be controlled with this command, all motors will run at
  maximum speed when enabled. The main brush and side brush can be run in either direction. The
  vacuum only runs forward.
  Serial sequence: [138] [Motors]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Bits 0-2: 0 = off, 1 = on at 100% pwm duty cycle
   Bits 3 & 4: 0 = motor’s default direction, 1 = motor’s opposite direction. Default direction for the side
  brush is counterclockwise. Default direction for the main brush/flapper is inward.
  Bit 7 6 5 4 3 2 1 0
  Value Reserved Main
  Brush
  Direction
  Side
  Brush
  Clockwise?
  Main
  Brush
  Vacuum Side
  Brush
  Example:
  To turn on the main brush inward and the side brush clockwise, send: [138] [13]
  */
  MOTORS,

  /**
  LEDs Opcode: 139 Data Bytes: 3
  This command controls the LEDs common to all models of Roomba 600. The power LED is specified by
  two data bytes: one for the color and the other for the intensity.
   Serial sequence: [139] [LED Bits] [Power Color] [Power Intensity]
   Available in modes: Safe or Full
   Changes mode to: No Change
   LED Bits (0 – 255)
  Home and Spot use green LEDs: 0 = off, 1 = on
  Check Robot uses an orange LED.
  Debris uses a blue LED.
  Power uses a bicolor (red/green) LED. The intensity and color of this LED can be controlled with 8-bit
  resolution.
  LED Bits (0-255)
  Bit 7 6 5 4 3 2 1 0
  Value Reserved Check
  Robot
  Dock Spot Debris
  Power LED Color (0 – 255)
  0 = green, 255 = red. Intermediate values are intermediate colors (orange, yellow, etc).
  Power LED Intensity (0 – 255)
  0 = off, 255 = full intensity. Intermediate values are intermediate intensities.
  Example:
  To turn on the Home LED and light the Power LED green at half intensity, send the serial byte sequence
  [139] [4] [0] [128].
  */
  LEDS,

  /**
  Song Opcode: 140 Data Bytes: 2N+2,
   where N is the number of notes in the song
  This command lets you specify up to four songs to the OI that you can play at a later time. Each song is
  associated with a song number. The Play command uses the song number to identify your song selection.
  Each song can contain up to sixteen notes. Each note is associated with a note number that uses MIDI
  note definitions and a duration that is specified in fractions of a second. The number of data bytes varies,
  depending on the length of the song specified. A one note song is specified by four data bytes. For each
  additional note within a song, add two data bytes.
   Serial sequence: [140] [Song Number] [Song Length] [Note Number 1] [Note Duration 1] [Note
  Number 2] [Note Duration 2], etc.
   Available in modes: Passive, Safe, or Full
   Changes mode to: No Change
   Song Number (0 – 4)
  The song number associated with the specific song. If you send a second Song command, using the
  same song number, the old song is overwritten.
   Song Length (1 – 16)
  The length of the song, according to the number of musical notes within the song.
   Song data bytes 3, 5, 7, etc.: Note Number (31 – 127)
  The pitch of the musical note Roomba will play, according to the MIDI note numbering scheme. The
  lowest musical note that Roomba will play is Note #31. Roomba considers all musical notes outside
  the range of 31 – 127 as rest notes, and will make no sound during the duration of those notes.
   Song data bytes 4, 6, 8, etc.: Note Duration (0 – 255)
  The duration of a musical note, in increments of 1/64th of a second. Example: a half-second long
  musical note has a duration value of 32.

  Number Note Frequency Number Note Frequency Number Note Frequency
31 G 49.0 58 A# 233.1 85 C# 1108.8
32 G# 51.9 59 B 246.9 86 D 1174.7
33 A 55.0 60 C 261.6 87 D# 1244.5
34 A# 58.3 61 C# 277.2 88 E 1318.5
35 B 61.7 62 D 293.7 89 F 1396.9
36 C 65.4 63 D# 311.1 90 F# 1480.0
37 C# 69.3 64 E 329.6 91 G 1568.0
38 D 73.4 65 F 349.2 92 G# 1661.3
39 D# 77.8 66 F# 370.0 93 A 1760.0
40 E 82.4 67 G 392.0 94 A# 1864.7
41 F 87.3 68 G# 415.3 95 B 1975.6
42 F# 92.5 69 A 440.0 96 C 2093.1
43 G 98.0 70 A# 466.2 97 C# 2217.5
44 G# 103.8 71 B 493.9 98 D 2349.4
45 A 110.0 72 C 523.3 99 D# 2489.1
46 A# 116.5 73 C# 554.4 100 E 2637.1
47 B 123.5 74 D 587.3 101 F 2793.9
48 C 130.8 75 D# 622.3 102 F# 2960.0
49 C# 138.6 76 E 659.3 103 G 3136.0
50 D 146.8 77 F 698.5 104 G# 3322.5
51 D# 155.6 78 F# 740.0 105 A 3520.1
52 E 164.8 79 G 784.0 106 A# 3729.4
53 F 174.6 80 G# 830.6 107 B 3951.2
54 F# 185.0 81 A 880.0
55 G 196.0 82 A# 932.4
56 G# 207.7 83 B 987.8
57 A 220.0 84 C 1046.5
  */
  SONG,

  /**
  Play Opcode: 141 Data Bytes: 1
  This command lets you select a song to play from the songs added to Roomba using the Song command.
  You must add one or more songs to Roomba using the Song command in order for the Play command to
  work.
   Serial sequence: [141] [Song Number]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Song Number (0 – 4)
  The number of the song Roomba is to play.
  */
  PLAY,

  /**
  Sensors Opcode: 142 Data Bytes: 1
  This command requests the OI to send a packet of sensor data bytes. There are 58 different sensor data
  packets. Each provides a value of a specific sensor or group of sensors.
  For more information on sensor packets, refer to the next section, “Roomba Open Interface Sensors
  Packets”.
   Serial sequence: [142] [Packet ID]
   Available in modes: Passive, Safe, or Full
   Changes mode to: No Change
  Identifies which of the 58 sensor data packets should be sent back by the OI. A value of 6 indicates
  a packet with all of the sensor data. Values of 0 through 5 indicate specific subgroups of the sensor
  data.
  */
  SENSORS,

  /**
  Seek Dock Opcode: 143 Data Bytes: 0
  This command directs Roomba to drive onto the dock the next time it encounters the docking beams.
  This is the same as pressing Roomba’s Dock button, and will pause a cleaning cycle if one is already in
  progress.
   Serial sequence: [143]
   Available in modes: Passive, Safe, or Full
   Changes mode to: Passive
  */
  SEEK_DOCK,

  /**
  PWM Motors Opcode: 144 Data Bytes: 3
  This command lets you control the speed of Roomba’s main brush, side brush, and vacuum
  independently. With each data byte, you specify the duty cycle for the low side driver (max 128). For
  example, if you want to control a motor with 25% of battery voltage, choose a duty cycle of 128 * 25%
  = 32. The main brush and side brush can be run in either direction. The vacuum only runs forward.
  Positive speeds turn the motor in its default (cleaning) direction. Default direction for the side brush is
  counterclockwise. Default direction for the main brush/flapper is inward.
  Serial sequence: [144] [Main Brush PWM] [Side Brush PWM] [Vacuum PWM]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Main Brush and Side Brush duty cycle (-127 – 127)
   Vacuum duty cycle (0 – 127)
  */
  PWM_MOTORS,

  /**
  Drive Direct Opcode: 145 Data Bytes: 4
  This command lets you control the forward and backward motion of Roomba’s drive wheels
  independently. It takes four data bytes, which are interpreted as two 16-bit signed values using two’s
  complement. The first two bytes specify the velocity of the right wheel in millimeters per second (mm/s),
  with the high byte sent first. The next two bytes specify the velocity of the left wheel, in the same
  format. A positive velocity makes that wheel drive forward, while a negative velocity makes it drive
  backward.
   Serial sequence: [145] [Right velocity high byte] [Right velocity low byte] [Left velocity high byte]
  [Left velocity low byte]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Right wheel velocity (-500 – 500 mm/s)
   Left wheel velocity (-500 – 500 mm/s)
  */
  DRIVE_DIRECT,

  /**
  Drive PWM Opcode: 146 Data Bytes: 4
  This command lets you control the raw forward and backward motion of Roomba’s drive wheels
  independently. It takes four data bytes, which are interpreted as two 16-bit signed values using two’s
  complement. The first two bytes specify the PWM of the right wheel, with the high byte sent first. The
  next two bytes specify the PWM of the left wheel, in the same format. A positive PWM makes that wheel
  drive forward, while a negative PWM makes it drive backward.
   Serial sequence: [146] [Right PWM high byte] [Right PWM low byte] [Left PWM high byte] [Left PWM
  low byte]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Right wheel PWM (-255 – 255)
   Left wheel PWM (-255 – 255)
  */
  DRIVE_PWM,

  /**

  */
  // Digital Outputs

  /**
  Stream Opcode: 148 Data Bytes: N + 1,
   where N is the number of packets requested.
  This command starts a stream of data packets. The list of packets requested is sent every 15 ms, which
  is the rate Roomba uses to update data.
  This method of requesting sensor data is best if you are controlling Roomba over a wireless network
  (which has poor real-time characteristics) with software running on a desktop computer.
   Serial sequence: [148] [Number of packets] [Packet ID 1] [Packet ID 2] [Packet ID 3] etc.
   Available in modes: Passive, Safe, or Full
   Changes mode to: No Change
  The format of the data returned is:
[19][N-bytes][Packet ID 1][Packet 1 data…][Packet ID 2][Packet 2 data…][Checksum]
N-bytes is the number of bytes between the n-bytes byte and the checksum.
The checksum is a 1-byte value. It is the 8-bit complement of all of the bytes in the packet, excluding
the checksum itself. That is, if you add all of the bytes in the packet, including the checksum, the low
byte of the result will be 0.
Example:
To get data from Roomba's left cliff signal (packet 29) and virtual wall sensor (packet 13), send the
following command string to Roomba:
[148] [2] [29] [13]
NOTE:
The left cliff signal is a 2-byte packet and the virtual wall is a 1-byte packet.
Roomba starts streaming data that looks like this:
 19 5 29 2 25 13 0 163
header n-bytes packet ID 1 Packet data 1 (2 bytes) packet ID 2 packet data 2 (1 byte) Checksum
NOTE:
Checksum computation: (19 + 5 + 29 + 2 + 25 + 13 + 0 + 163) = 256 and (256 & 0xFF) = 0.
In the above stream segment, Roomba’s left cliff signal value was 549 (0x0225) and there was no virtual
wall signal.
It is up to you not to request more data than can be sent at the current baud rate in the 15 ms time slot.
For example, at 115200 baud, a maximum of 172 bytes can be sent in 15 ms:
15 ms / 10 bits (8 data + start + stop) * 115200 = 172.8
If more data is requested, the data stream will eventually become corrupted. This can be confirmed by
checking the checksum.
The header byte and checksum can be used to align your receiving program with the data. All data
chunks start with 19 and end with the 1-byte checksum.
  */
  STREAM = 148,

  /**
  Query List Opcode: 149 Data Bytes: N + 1,
   where N is the number of packets requested.
  This command lets you ask for a list of sensor packets. The result is returned once, as in the Sensors
  command. The robot returns the packets in the order you specify.
   Serial sequence: [149][Number of Packets][Packet ID 1][Packet ID 2]...[Packet ID N]
   Available in modes: Passive, Safe, or Full
   Changes modes to: No Change
  Example:
  To get the state of the bumpers and the virtual wall sensor, send the following sequence:
  [149] [2] [7] [13]
  */
  QUERY_LIST,

  /**
  Pause/Resume Stream Opcode: 150 Data Bytes: 1
  This command lets you stop and restart the steam without clearing the list of requested packets.
   Serial sequence: [150][Stream State]
   Available in modes: Passive, Safe, or Full
   Changes modes to: No Change
   Range: 0-1
  An argument of 0 stops the stream without clearing the list of requested packets. An argument of 1
  starts the stream using the list of packets last requested.
  */
  PAUSE_RESUME_STREAM,

  //Send IR
  //Script
  //Play Script
  //Show Script
  //Wait Time
  //Wait Distance
  //Wait Angle
  //Wait Event

  /**
  Scheduling LEDS Opcode: 162 Data Bytes: 2
  This command controls the state of the scheduling LEDs present on the Roomba 560 and 570.
   Serial sequence: [162] [Weekday LED Bits][Scheduling LED Bits]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Weekday LED Bits (0 – 255)
   Scheduling LED Bits (0 – 255)
   All use red LEDs: 0 = off, 1 = on
  Weekday LED Bits
  Bit 7 6 5 4 3 2 1 0
  Value Reserved Sat Fri Thu Wed Tue Mon Sun
  Scheduling LED Bits
  Bit 7 6 5 4 3 2 1 0
  Value Reserved Schedule Clock AM PM Colon (:)
  */
  SCHEDULING_LEDS = 162,

  /**
  Digit LEDs Raw Opcode: 163 Data Bytes: 4
  This command controls the four 7 segment
  displays on the Roomba 560 and 570.
   Serial sequence: [163] [Digit 3 Bits] [Digit 2
  Bits] [Digit 1 Bits] [Digit 0 Bits]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Digit N Bits (0 – 255)
   All use red LEDs: 0 = off, 1 = on. Digits are
  ordered from left to right on the robot
  3,2,1,0.
  Digit N Bits
  Bit 7 6 5 4 3 2 1 0
  Value Reserved G F E D C B A
  */
  DIGIT_LEDS_RAW,

  /**
  Digit LEDs ASCII Opcode: 164 Data Bytes: 4
  This command controls the four 7 segment displays on the Roomba 560 and 570 using ASCII character
  codes. Because a 7 segment display is not sufficient to display alphabetic characters properly, all
  characters are an approximation, and not all ASCII codes are implemented.
   Serial sequence: [164] [Digit 3 ASCII] [Digit 2 ASCII] [Digit 1 ASCII] [Digit 0 ASCII]
   Available in modes: Safe or Full
   Changes mode to: No Change
   Digit N ASCII (32 – 126)
   All use red LEDs. Digits are ordered from left to right on the robot 3,2,1,0.
  Example:
  To write ABCD to the display, send the serial byte sequence: [164] [65] [66] [67] [68]
  Table of ASCII codes:
  Code Display Code Display Code Display Code Display
  32 53 5 70, 102 F 86, 118 V
  33 ! 54 6 71, 103 G 87, 119 W
  34 ” 55 7 72, 104 H 88, 120 X
  35 # 56 8 73, 105 I 89, 121 Y
  37 % 57 9 74, 106 J 90, 122 Z
  38 & 58 : 75, 107 K 91, 40 [
  39 ’ 59 ; 76, 108 L 92 \
  44 , 60 ¡ 77, 109 M 93, 41 ]
  45 - 61 = 78, 110 N 94 ∧
  46 . 62 ¿ 79, 111 O 95
  47 / 63 ? 80, 112 P 96 ‘
  48 0 65, 97 A 81, 113 Q 123 {
  49 1 66, 98 B 82, 114 R 124 —
  50 2 67, 99 C 83, 36, 115 S 125 }
  51 3 68, 100 D 84, 116 T 126 ∼
  52 4 69, 101 E 85, 117 U
  */
  DIGIT_LEDS_ASCII,

  /**
  Buttons Opcode: 165 Data Bytes: 1
  This command lets you push Roomba’s buttons. The buttons will automatically release after 1/6th of a
  second.
   Serial sequence: [165] [Buttons]
   Available in modes: Passive, Safe, or Full
   Changes mode to: No Change
   Buttons (0-255) 1 = Push Button, 0 = Release Button
  Buttons
  Bit 7 6 5 4 3 2 1 0
  Value Clock Schedule Day Hour Minute Dock Spot Clean
  */
  BUTTONS,

  /**
  This command sends Roomba a new schedule. To disable scheduled cleaning, send all 0s.
   Serial sequence: [167] [Days] [Sun Hour] [Sun Minute] [Mon Hour] [Mon Minute] [Tue Hour] [Tue
  Minute] [Wed Hour] [Wed Minute] [Thu Hour] [Thu Minute] [Fri Hour] [Fri Minute] [Sat Hour] [Sat
  Minute]
   Available in modes: Passive, Safe, or Full.
   If Roomba’s schedule or clock button is pressed, this command will be ignored.
   Changes mode to: No change
   Times are sent in 24 hour format. Hour (0-23) Minute (0-59)
  Days
  Bit 7 6 5 4 3 2 1 0
  Value Reserved Sat Fri Thu Wed Tue Mon Sun
  Example:
  To schedule the robot to clean at 3:00 PM on Wednesdays and 10:36 AM on Fridays, send: [167] [40] [0]
  [0] [0] [0] [0] [0] [15] [0] [0] [0] [10] [36] [0] [0]
  To disable scheduled cleaning, send: [167] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0]
  */
  SCHEDULE = 167,

  /**
  Set Day/Time Opcode: 168 Data Bytes: 3
  This command sets Roomba’s clock.
   Serial sequence: [168] [Day] [Hour] [Minute]
   Available in modes: Passive, Safe, or Full.
   If Roomba’s schedule or clock button is pressed, this command will be ignored.
   Changes mode to: No change
   Time is sent in 24 hour format. Hour (0-23) Minute (0-59)
  Code Day
  0 Sunday
  1 Monday
  2 Tuesday
  3 Wednesday
  4 Thursday
  5 Friday
  6 Saturday
  */
  SET_DAY_TIME,

  /**
  Stop Opcode: 173 Data Bytes: 0
  This command stops the OI. All streams will stop and the robot will no longer respond to commands.
  Use this command when you are finished working with the robot.
   Serial sequence: [173].
   Available in modes: Passive, Safe, or Full
   Changes mode to: Off. Roomba plays a song to acknowledge it is exiting the OI.
  */
  STOP = 173,
} ROOMBA_OP_CODE;

/*
typedef enum {
129 Left = 129,
130 Forward,
131 Right,
132 Spot,
133 Max,
134 Small,
135 Medium,
136 Large_Clean,
137 Stop,
138 Power,
139 Arc Left,
140 Arc Right,
141 Stop,
142 Download,
143 Seek Dock,

240 Reserved
248 Red Buoy
244 Green Buoy
242 Force Field
252 Red Buoy and Green
Buoy
250 Red Buoy and Force
Field
246 Green Buoy and Force
Field
254 Red Buoy, Green Buoy
and Force Field
Roomba 600
Drive-on
Charger
160 Reserved
161 Force Field
164 Green Buoy
165 Green Buoy and Force
Field
168 Red Buoy
169 Red Buoy and Force
Field
172 Red Buoy and Green
Buoy
173 Red Buoy, Green Buoy
and Force Field
Roomba 600
Virtual Wall
162 Virtual Wall
Roomba 600
Auto-on Virtual
Wall
0LLLL0BB LLLL = Auto-on virtual
wall ID (assigned
automatically by
Roomba 660 robots)
1-10: Valid ID
11: Unbound
12-15: Reserved
} ROOMBA_PACKET_ID;
*/

typedef enum {
	PASSIVE_MODE,
	SAFE_MODE,
	FULL_MODE,
} ROOMBA_STATE;


#endif /* ROOMBA_H_ */
