    
    README: Gurra's Pit Tools v. 0.3
    
 --------------------------------------

GPT is a toolkit enabling you to build home cockpits 
for Falcon BMS that have software components distributed 
on different computers. This document VERY briefly explains 
what GPT includes and how to install it.


A. Contents

GPT consists of the following components:  
 - The displays transmitter (game pc)
 - The shm transmitter (game pc)
 - The key receiver (game pc)
 - The displays receiver (slave pc)
 - The shm receiver (slave pc)
 - The key transmitter (slave pc)


B. Installation
    
GPT is released as a simple zip file (gpt.zip) with two directories inside
 - components
 - dependencies
 
 0. On both game and slave pcs: Install a java jvm, such as the oracle jre (at least java 8).
 1. On both game and slave pcs: Run the installers in the dependencies folder.
 2. On the game pc: copy the contents of the components folder to anywhere you like.
 3. On the slave pc: copy the contents of the components folder to anywhere you like.
 4. Edit each component's corresponding cfg file (...._cfg.json) in notepad or your editor of choice
 5. Create custom shortcuts as desired.
 6. Put the following in your "Falcon BMS 4.33\User\Config\falcon bms.cfg":
		set g_bExportRTTTextures 1
		set g_nRTTExportBatchSize 1
    

C. Compatibility

This new GPT release (0.3) is not compatible with older versions in any way.


D. Help/Instructions/Howtos

The best way to get help is to post in the official GPT thread in the BMS forums at:
http://www.benchmarksims.org/forum/showthread.php?10677-Beta-Release-GPT-(cockpit-texture-extraction-remote-cockpit-control-shm-mirror)
