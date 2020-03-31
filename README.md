# IBM-dials-retrofit

This repo contains the firmware and hardware instructions to retrofit your *IBM Dials* with a full-speed USB interface, cabable of CDC, MIDI and HID. The mod uses an OtterPill, a STM32F072 devboard featuring USB-PD. This feature however is not needed and the PD-phy should be replaced with two 5.1k-ohm resistors. Additionally an IBM Dials and a 3D printed Otter-Pill mount is needed. 

The firmware on the OtterPill can be customized and used for a variety of applications, e.g. [KiCAD](https://twitter.com/JanHenrikH/status/1245113168621449217).

**Documentation is WIP**

![](images/1.jpeg)

## Building instructions

