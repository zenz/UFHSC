准备开源一个东西，用来控制地暖系统的。
市场上有类似的产品，但都是闭源的，这个开源，爱折腾爱研究爱写程序的，可以通过这个东西来设计和测试自己的地暖控制逻辑。

包含的接口有：
1、两路温度传感器接口（用B=3950，100K 精度1% NTC电阻温度传感器）
2、一路壁挂炉采暖启停控制接口
3、一路循环泵启停控制接口
4、一路负载式温控面板接口
5、一路温控面板直接控制的热电阀接口
6、一个UART通讯接口，用于通过USB2TTL接线实现与电脑串口通讯
7、一个ICSP接口，用于给单片机刷写Bootloader
8、一个ESP-05款无线WIFI模块接口，用于实现IoT监控
9、一个USB/AC供电跳针接口，用于选择单片机电源供给方式，调试用
10、一个预留的开关判断接口（目前在代码中用于是否连接循环泵的判断）

整个设备，全部采用直插式原件，方便爱好者自己焊接制作。所有用到的零件，都可以在网上（淘宝）购买到。
温度传感器精度可通过选择精度高的NTC电阻以及分压电阻(4.7K)来调节。
目前选用的NTC精度1%，分压电阻精度5%，测试下来在测试物温度恒定时，波动温度不超过0.2度（个人认为对地暖来说这个精度够了，波动更大的原因是开关电源的稳定性）。

单片机采用ATMEL公司的ATMEGA328P-PU，8位处理芯片，FLASH内存32K，动态内存2K，刷开源固件Arduino。
原始控制代码也全部开源，用Arduino编写，在Arduino IDE 1.0.5 以上版本编译通过并可通过USB2TTL上传到单片机。

开源的目的是什么？
个人一直是开源（软硬件）的支持者，认同开源可以使得在开放式讨论环境中，目标理论和算法能够得到更充分的探讨和完善。而且能够在使用环境适应性上有更加灵活的应对方法来解决实际的问题。FDM（熔融堆积成型）三维打印机这几年的发展就是一个例子。还有桌面CNC铣床GBRL，微型纺织机FirePicker等等。Github上随便搜搜都能找到一堆的开源软硬件项目。

开源会不会对企业带来经济损失？
个人认为这是一个伪命题。一个企业的研发能力是有限的，特别是中小型企业，要能够有人员和资金持续地，不断投入一个项目的研究还不钻牛角尖是很难的。因此，很多的企业，甚至是大企业，都采取部分开源的做法，来吸引更多对此问题关注的人参与进来，从而在获得的研究结果中提取有利于企业本身的数据和技术，来不断完善企业自己生产和销售的产品。产品做好了，买来使用的人其实占据了绝大多数。

开源需要的是什么？
开源需要的是真诚的交流和主动改变的心态，在这一点上，国外的开发者似乎比国内的做的要好一些。但这几年，国内的开源项目也多起来了。主要还是心态的慢慢转变吧。

接下来会怎么做？
我会把这个项目放到Github上，所有的设计文件，代码，让有想法又愿意分享并贡献的人可以跟进交流。希望这个开源项目能获得更多人的关注，从而使得对地面辐射采暖节能控制更多的了解和应用。这样我就成功了。如果最后也没有什么进步，那么我也至少努力过了。

OK，我要说的说完了。你约不约？