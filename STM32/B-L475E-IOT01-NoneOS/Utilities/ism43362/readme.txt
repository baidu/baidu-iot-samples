readme.txt
----------


In order to update the Inventek ISM43362 Wifi module firmware, there are several steps:
This update will fully erase the chip.

Prior to running this, 
1.	You must have the ST-LINK Utility installed in so that the ST-LINK_CLI.exe path is "C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" (default install directory).
http://www.st.com/content/st_com/en/products/embedded-software/development-tool-software/stsw-link004.html

2.	Update the ST-LINK firmware on the discovery board with STM32 ST-LINK Utility
    2.1 In the 'Target' tab, select 'Connect'
	2.2 In the 'ST-LINK' tab, select 'Firmware update'
	2.3 Click on 'Device Connect' button then 'Yes' button
	2.4 In the 'Target' tab, select 'Disconnect'
	
3.	You must have the STMFlashLoader demonstration tool installed so that the STMFlashLoader.exe path is  "C:\Program Files (x86)\STMicroelectronics\Software\Flash Loader Demo\STMFlashLoader.exe" (default install directory).
http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/flasher-stm32.html

4.	You must make sure you have no other ST-LINKs connected to the PC other than the Discovery Kit.

From the IotNodeDiscoveryKit\Utilities\ism43362\bin directory, run update_Wifi_FW_9dBm_C3.5.2.3.BETA9.bat and it will go through 2 steps:
1.	Update the Discovery Kit firmware to InventekBootloaderPassthrough.bin using the ST-LINK command line interface.
2.	Update the Inventek WiFi firmware to ISM43362_M3G_L44_SPI_9dBm_C3.5.2.3.BETA9.bin using the STMFlashLoader command line interface.

If you want to upload another firmware image to the Inventek module, you’ll have to modify line 10 of the batch file to point to your file rather than ISM43362_M3G_L44_SPI_9dBm_C3.5.2.3.BETA9.bin 
Also, if you need to modify the path for the tools (ST-LINK or STMFlashLoader), you can modify it as necessary in the batch file.

