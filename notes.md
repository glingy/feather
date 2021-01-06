## Feather M0 Game Notes

#### LCD
 - Backlight can remain disconnected. Use register brightness settings instead.
   - Persist across reset, system-level using another reserved byte in ram or sd card file... yup sd card status file? -- todo
 - Reset can be kept low on startup for +5ms, then raised and the extra 120ms in the datasheet can be ignored
   - I have found no indication that the extra time is really necessary.
   - Reset pin for some reason slowly drops power when M0 reset is held low, so at some variable point display does reset, but it seems ok so long as CS falls after it resets.
 - I'm using 4-wire serial mode with a separate line for DC and no read capability.  
  


#### SD Card
 - For now, MBR partitioning scheme with single FAT32 partition. 
 - Programs should be stored in the root directory to be found and their extension should be .BIN (I think it's caps-insensitive, so .bin *should* work fine as well.)
 - Check CS status? Can toggling chip select fix invalid command due to interruption?
 - Add a pin to reset line? I can set it high when I don't want reset to be triggered as a failsafe so the sd card isn't interrupted except for power button?

 - SD Block is 512 bytes, which is also usually the size of a FAT cluster, which is often broken up into 16 individual sectors.
 - I rely on 512 byte clusters and seem to ignore the sector size?


#### Battery
 - Low power warning screen needed at 2.9V or less-ish. It seems to die around 2.8
 - Display flickers and looks dim at low battery



#### Work Log

Several months of work before...

##### 1/1/21
 - LCD's backlight would turn on, but the display seemed to be failing to reset. 
  - CS must *fall* after reset before the serial interface will activate because the serial interface initializes when it is high and is operational when it is low.
  - I had forgotten this important fact, then forgot I changed some of the code

 - Strange note... I think it did fall afterwards... I think there's some timing involved with the cs toggle and reset, but the datasheet is incorrect about reset timing and is unclear about cs timing. CS does work with the shortest high possible after reset rises. (36ns high ~0.2ms after RST high)
 
##### 1/2/21
 - Rewired the joystick so gnd and vcc are swapped so the switch is connected to vcc like the rest of the buttons.
 - SD card was having an error when reading an empty directory and hanging. It had been reading from the fat using a uint8_t offset instead of uint32_t for entries, so was reading 0xFFF0FFFF instead of 0x0FFFFFFF and was getting confused (reading for the next in chain for cluster 2, hence the offset by 2 bytes)
 - Added early exit for end-of-dir condition: if the first character of the name is null, then that is end of dir.

 - Stuff works! Cleaning up code now
 - Switching USB to namespace instead of a class for simplicity and removing unnecessary includes.
 - I need to write a user serial interface test because I have a setup here for it, but it's not intuitive to use.

##### 1/3/21
 - Working on reformatting SD code into separate FS code for normal use.
 