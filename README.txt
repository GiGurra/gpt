    
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

 - On both game and slave pcs: Install a java jvm, such as the oracle jre (at least java 8).
 - On the game pc: extract the contents of gpt zip to anywhere you like.
 - On the slave pc: extract the contents of gpt zip to anywhere you like.
 - Edit each component's corresponding cfg file (...._cfg.json) in notepad or your editor of choice
		Specifically check that IP addresses/hostnames are correct
 - Set the following in your "Falcon BMS 4.33\User\Config\falcon bms.cfg":
		set g_bExportRTTTextures 1
		set g_nRTTExportBatchSize 1
    

C. Compatibility

This GPT release (0.3) is not guaranteed to be compatible with older versions in any way.


D. Help/Instructions/Howtos

The best way to get help is to post in the official GPT thread in the BMS forums at:
http://www.benchmarksims.org/forum/showthread.php?10677-Beta-Release-GPT-(cockpit-texture-extraction-remote-cockpit-control-shm-mirror)
