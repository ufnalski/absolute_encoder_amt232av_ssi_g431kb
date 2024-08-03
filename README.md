# Absolute encoder with SSI interface  (STM32G431KB)
An STM32 HAL example of communicating with an absolute encoder over the SSI interface. A single-turn 14-bit encoder from CUI Devices is taken as an example. The relevant evaluation kit is [AMT232B-V](https://www.cuidevices.com/product/motion-and-control/rotary-encoders/absolute/modular/amt23-series). Both blocking and non-blocking (_IT) modes are implemented.

![CUI Devices AMT23 in action](/Assets/Images/cui_devices_ssi_in_action.jpg)

The main motivation behind this submission is to encourage you to get familiar with an SSI interface. And the "an" here is to point out that SSI is kind of a loosely defined standard and many modified versions exist. You should then pay a special attention to timing waveforms provided by a manufacturer of a device you are going to deal with. The differences may be in the physical layer (differential vs. single-ended lines) as well as in the presence of the CS line or its lack (as in the initial specification). You may also encounter some naming inconsistencies. The SSI without the CS line uses the first SCK transition from high to low to start the communication with the slave and the consecutive transition from low to high to inform the slave that it is time to push the first bit of information. The bits should be then sampled at the CLK falling edges. The SSI with the CS line may still keep the original "additional" clock cycle or skip it, as the CS line can now serve its purpose, turning into the regular SPI, yet still called in some datasheets an SSI.

For example, to communicate with the absolute encoder AMT23, we have to reproduce the following waveforms:

![CUI Devices AMT23 timing waveform](/Assets/Images/ssi_to_be_read_by_spi.JPG)

Source: [CUI Devices AMT23 datasheet](https://www.cuidevices.com/product/resource/amt23.pdf)

We are going to use a regular SPI peripheral to do that. Note that the SPI uses CS to initialize pushing the bits. The SPI then samples the MISO line also at the red falling edge what is needless here. Also note that the STM32 SPI peripheral can be configured to read from 4 to 16 bits. We will then use HAL_SPI_TransmitReceive() to read 16 logical states (the spare red one and the 15 green bits of information). Then we will read the last bit of information (the blue one) using HAL_GPIO_ReadPin() before pulling up the CS line.

# Missing files?
Don't worry :slightly_smiling_face: Just hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Libraries
* OLED: [stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306) (MIT license)

# Hardware
* [Bidirectional 4-channel logic level shifter](https://sklep.msalamon.pl/produkt/konwerter-poziomow-logicznych-33v-5v-4-kanalowy/)
* [OLED display 1.3" (SH1106 or SSD1306)](https://sklep.msalamon.pl/produkt/wyswietlacz-oled-13-i2c-bialy/)
* [AMT Cable AMT-06C-1-036](https://www.mouser.pl/ProductDetail/CUI-Devices/AMT-06C-1-036?qs=fAHHVMwC%252BbhzTLN9MYRSPg%3D%3D)

# Tools
* [DSLogic Plus](https://www.dreamsourcelab.com/product/dslogic-series/) or any other logic analyzer capable of sampling at 16+ MHz.
* [YAT - Yet Another Terminal :: Serial Communication :: Engineer/Test/Debug](https://sourceforge.net/projects/y-a-terminal/)
* [Tabby - a terminal for the modern age](https://tabby.sh/)

![DSLogic Plus in action](/Assets/Images/ssi_logic_analyzer.JPG)

# Tips and tricks
* [List of ANSI color escape sequences](https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences)
* [Box-drawing characters](https://en.wikipedia.org/wiki/Box-drawing_characters)

![Tabby in action](/Assets/Images/ssi_raw_data_tabby.JPG)

# Readings

## SSI
* [SSI Interface Description](https://cdn.sick.com/media/docs/9/79/079/technical_information_ssi_interface_description_en_im0100079.pdf) (from the original developers of SSI)
* [SSI Encoders Overview](https://www.dynapar.com/technology/encoder_basics/ssi_encoders/)
* [How SSI Encoders Work](https://www.roboteq.com/applications/all-blogs/15-how-ssi-encoders-work)
* [Synchronous Serial Interface](https://www.posital.com/pl/produkty/interfaces/ssi/ssi-encoder.php)
* [Integrating Absolute Encoders - An Overview of SPI, RS-485, and SSI Protocols](https://www.cuidevices.com/blog/integrating-absolute-encoders-an-overview-of-spi-rs-485-and-ssi-protocols)
* [AR35-T25E/S Series](https://docs.broadcom.com/doc/AR35-ThroughHole-Series-DS) - Figure 9 suggests to be an example of SPI denoted as SSI (haven't put my hands on that particular encoder, so correct me if I'm wrong)
* [AEAT-6010/6012 Magnetic Encoder](https://docs.broadcom.com/doc/AV02-0188EN) - identical waveforms as in our case
* [BiSS and SSI - An Overview](https://www.celeramotion.com/inductive-sensors/support/technical-papers/biss-and-ssi-an-overview/)

## SPI
* [Serial Peripheral Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)
* [AN-1248: SPI Interface](https://www.analog.com/en/resources/app-notes/an-1248.html)
* [Introduction to SPI Interface](https://www.analog.com/en/resources/analog-dialogue/articles/introduction-to-spi-interface.html)

## BiSS (hardware compatible to SSI but offers additional features and options)
* [About BiSS](https://biss-interface.com/about-biss/)
* [BiSS interface](https://en.wikipedia.org/wiki/BiSS_interface)
* [BiSS C Protocol Description](http://biss-interface.com/download/biss-c-protocol-description-english/)

# Call for action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2024_dzien_otwarty_we/Dzien_Otwarty_WE_2024_Control_Engineering_for_Hobbyists.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Control in power electronics and drives - do try this at home :exclamation:

190+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned!
