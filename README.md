# CH32V305FB_UAC2实现UAC2.0
&emsp;&emsp;CH32V305FB 是基于青稞RISC-V 内核设计的工业级通用微控制器。CH32V30x 系列基于青稞V4F微处理器设计，支持单精度浮点指令和快速中断响应，支持144MHz主频零等待运行，提供2组串口、一个I2S，内置USB2.0高速PHY收发器（480Mbps）。  

&emsp;&emsp;V305FB基本是拥有USHS接口最便宜的方案，其I2S外设支持音频采样频率支持范围8kHz-562.2kHz。我们期望设计一个最大采样384kHz 32bits 2Channel的USB转I2S小板，USB使用UAC2.0协议。UAC2.0直接使用CheeryUSB协议栈，可以减少开发周期。感谢大佬的开源！
  
