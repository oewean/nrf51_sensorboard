nrf51_sensorboard
=================

![nrf51 dongle with sensor board](https://raw.githubusercontent.com/vegarwe/nrf51_sensorboard/master/doc/IMG_20141130_220524.jpg)

Sensor devices

| Device name  | Device type     | Addr bits | Addr | Data sheet                                                                                                            |
| ------------ | --------------- | --------- | ---- | --------------------------------------------------------------------------------------------------------------------- |
| PCA9535A     | IO-extender     | 0100111   | 0x27 | [PCA9535A.pdf](http://www.nxp.com/documents/data_sheet/PCA9535A.pdf)                                                  |
| LIS3DH       | Accelerometer   | 0011000   | 0x18 | [CD00274221](http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/CD00274221.pdf)       |
| MPU-9150     | Motion tracking | 1101000   | 0x68 | <div>[PS-MPU-9150A-00v4_3.pdf](http://www.invensense.com/mems/gyro/documents/PS-MPU-9150A-00v4_3.pdf)</div><div>[RM-MPU-9150A-00v4_2.pdf](http://www.invensense.com/mems/gyro/documents/RM-MPU-9150A-00v4_2.pdf)</div> |
| MCP9808      | Temp            | 0011011   | 0x1b | [25095A.pdf](http://ww1.microchip.com/downloads/en/DeviceDoc/25095A.pdf)                                              |
| MAX44009EDT+ | Ambient light   | 1001010   | 0x4a | [MAX44009.pd](http://datasheets.maximintegrated.com/en/ds/MAX44009.pdf)                                               |
| LPS25H       | Pressure        | 1011100   | 0x5c | [DM00066332.pdf](http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00066332.pdf)   |
