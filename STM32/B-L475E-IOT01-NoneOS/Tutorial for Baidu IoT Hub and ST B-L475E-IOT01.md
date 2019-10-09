### 教程概述
*** 
- ST和百度云天工基于ST新近推出的一款物联网开发套件B-L475E-IOT01A，提供了连接百度云天工的软件扩展包，内含连接到百度云天工物接入（IoT Hub），进行消息订阅和发布的应用程序。
- 用户基于本教程，使用ST的B-L475E-IOT01A可以快速上手搭建自己的百度云物联网应用程序。

### 硬件概述
*** 
- B-L475E-IOT01A是ST新近推出一款物联网套件，支持WIFI，蓝牙，SubG, NFC等多种无线连接方式。 而且还支持Arduino接口，所以使用者也可以很方便的通过Arduino接口来扩展其他的无线连接模块，比如GSM模块。
- 这块开发板使用的是ST低功耗MCU系列中的STM32L475这款芯片，拥有1Mbytes的Flash和128Kbytes的SRAM。板上还集成了64Mbits的Quad-SPI Flash。而且板上搭载了多种传感器，比如温湿度传感器，高性能3轴磁力计，加速度传感器，陀螺仪，接近传感器和压力传感器等。便于开发者用来进行多种应用的演示。
- 板子上还有两个数字全角度麦克风，所以用这块开发板，还可以实现语音控制的功能。
- 配合这个套件，ST和百度共同推出了连接百度天工的软件扩展包。提供了连接到百度IoT hub, 进行消息定阅和发布的应用例程，客户可以基于这个例程快速的上手搭建自己的应用程序。
- 从[ST的官网](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l475e-iot01a.html
)可以下载到关于该开发套件的详细介绍文档和硬件设计资料。
### 硬件环境
*** 
- B-L475E-IOT01A板子上已经集成了WIFI模块和ST-LINK，所以不需要额外的模块和调试工具，只需要一根micro的USB线给板子提供电源就可以开始使用了。当然，如果需要更新程序的话，还需要一台带USB接口的电脑。
    - 一块B-L475E-IOT01A开发板（集成了WIFI模块和ST-LINK）
    - 一根micro接口的 USB线（给板子供电，提供调试和程序下载接口）
    - 一个WIFI热点（WIFI路由器或者手机热点）
    - 一台电脑（编译和烧录程序，配置WIFI密码）

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/1.png)

### 软件环境
***
需要的软件包括：
- 免费的SW4STM32 IDE（Windows, Linux, macOS）
- 串口调试工具Tera Term
- 百度云天工物接入资源
- 基于B-L475E-IOT01A板子的源代码。[下载地址](https://github.com/baidu/iot-edge-sdk-samples)

通过免费的SW4STM32，可以修改，编译，下载和调试程序。链接是SW4STM32的[下载地址](http://www.openstm32.org/Downloading%2Bthe%2BSystem%2BWorkbench%2Bfor%2BSTM32%2Binstaller)，不过需要先注册登录后才能下载。

还有更多的[文档资料](http://www.openstm32.org/Documentation)。

通过Tera Term可以向板子配置WIFI用户名和密码，并且显示程序运行时的信息。

下面将具体介绍，如何使用SW4STM32来编译，下载程序到开发板。以及Tera Term的使用方法。
### SW4STM32使用说明
***
##### SW4STM32下载安装

SW4STM32包含一系列的Eclipse插件，可以安装在现有的Eclipse上，也可以通过下载独立的安装程序来安装。

从SW4STM32官网下载安装程序，官网上提供了Windows, Mac OS/X和Linux的版本。选择合适的版本下载安装。

这里以Windows版本为例进行介绍。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/2.png)

双击下载的安装程序，按照提示进行安装。详细的安装说明见[官网链接](http://www.openstm32.org/Installing%2BSystem%2BWorkbench%2Bfor%2BSTM32)。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/3.png)

安装完成后，在开始菜单里找到SystemWorkbench for STM32，打开。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/4.png)

### 导入工程
***
打开SW4STM32后，首先会弹出下面窗口,让你选择一个Workspace。如果是第一次使用，可以通过“Browse”按钮设置workspace的位置。如果之前已经设置过，就可以在下拉菜单中选择一个。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/5.png)

点击“OK”，就可以看到SW4STM32的欢迎界面了。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/6.png)

下面通过File-->Import菜单导入已有的工程。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/7.png)

在弹出的窗口里，选择“Existing Projects into Workspace”，然后点击“Next”。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/8.png)

接下来，选择源文件所在的位置。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/9.png)

在Projects的窗口会显示已存在的工程，选择该工程。点击“Finish”按钮。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/10.png)

工程就已经被导入进来了，关闭Welcome欢迎窗口或者点击左上角的“Restore”按钮，就可以看到整个工程的界面了。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/11.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/12.png)

### 创建百度云物接入资源
***
在使用物接入服务前，您需要创建一个百度云账号，请按照下述步骤进行注册和登录。
1.	注册并登录百度云平台，请参考注册和登录。
2.	如果未进行实名认证，请参考实名认证操作方法完成认证。
3.	登录成功后，导航栏选择“产品服务>物接入IOT Hub”，即可开始创建实例。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/13.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/14.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/15.png)

### 创建百度云物接入实例
***
1.	点击“创建实例”，填写实例名称，点击“确定”完成实例创建。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/16.png)

2.	返回实例列表，查看已经创建的物接入实例：

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/17.png)

物接入默认提供三种地址，选择不同的地址，意味着您可以通过不同的方式连接到百度云物接入服务。每类地址的具体用法如下：
- tcp://yourendpoint.mqtt.iot.gz.baiduce.com:1883，端口1883，不支持传输数据加密。
- ssl://yourendpoint.mqtt.iot.gz.baiduce.com:1884，端口1884，支持SSL/TLS加密传输。可使用MQTT.fx客户端连接，参考配置MQTT客户端。
- wss://yourendpoint.mqtt.iot.gz.baidubce.com:8884，端口8884，支持Websockets浏览器方式连接，同样包含ssl加密，参考Websockets Client。

### 创建百度云物接入设备
***
1.	点击实例名称，进入物接入实例页面。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/18.png)

2.	点击“创建设备”，在弹出窗口中输入设备名称，并点击“下一步”。
“实例名称/设备名称”将作为实体设备连接云端的用户名。
3.	为设备绑定身份。此处可以直接从下拉菜单中选择身份名称，如果没有可用身份，也可以选择“+创建”，创建新身份。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/19.png)

4.	输入身份名称，并点击“下一步”。
5.	为身份绑定策略。此处可以直接从下拉菜单中选择策略名称，如果没有可用策略，也可以选择“+创建”，创建新策略。
6.	输入策略名称、主题及主题的权限，点击“下一步”完成设备创建。
说明：
每个策略下只能配置100个主题。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/20.png)

每个策略可以创建多个主题，在创建策略弹框右侧，点击“+”可以绑定更多的主题。添加主题时，可以使用“#”或“+”作为通配符，关于通配符的介绍，请参看关于[通配符的使用方法](https://cloud.baidu.com/doc/IOT/GettingStarted.html#.FD.1C.4A.A9.8A.2B.E2.A9.9C.D0.74.14.51.E8.AA.8A)。

创建新设备后，系统将自动初始化改身份对应的密钥。该密钥将用于后续实体设备登录，请妥善保管。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/21.png)

您也可以使用IoT Hub CLI命令创建各项参数，详细过程请参考[IoT Hub CLI](https://cloud.baidu.com/doc/IOT/IoTCLI.html#.E5.88.9B.E5.BB.BAIoT.20Hub.E6.9C.8D.E5.8A.A1)。

这样，就有了百度云物接入的endpoint、username、password、topic.

### 修改源码内Baidu IoT Hub连接信息
***
创建好百度云IOT Hub的服务，之前将示例工程导入到SW4STM32后，现在可以进行程序的修改，编译和下载了。

例程中提供的MQTT连接例程主要在iothub_mqtt_client_sample.c这个文件中实现。现在，将其中关于MQTT服务器地址，用户名，密码还有主题（endpoint、username、password、topic
）修改成之前的步骤中在云端创建的相关内容。而后进行编译。

将获得的MQTT服务器地址，用户名，密码还有主题填写到下面代码对应的位置。主题的属性，可以在云端设置为可以发布和订阅消息。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/22.png)

### 编译下载程序
***
接下来，就可以点击对应的按钮进行编译和下载了。本例程已经做好配置，直接点击对应的按钮就可以执行。关于编译和调试的配置说明，可以参考[官网文档](http://www.openstm32.org/User%2BGuide)。

##### 编译程序

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/23.png)

弹出窗口会显示编译进度。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/24.png)

##### 下载程序
下载程序之前，请确保已经用USB线将B-L475E-IOT01A与电脑相连。

如果下载出现问题，请确认ST-LINK的驱动已经正确安装。

如果ST-LINK的驱动正确安装，可以在电脑的设备管理器中看到如下设备：

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/25.png)

一般情况下当插入板子时，会自动安装驱动。如果没有看到这两个设备，请重新安装驱动。驱动[下载地址](http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-utilities/stsw-link009.html)。

通过“Run”按钮开始下载程序，操作见下图

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/26.png)

下方的Console窗口可以看到下载过程信息提示。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/27.png)

##### 调试程序
通过Debug窗口启动调试。见下图：

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/28.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/29.png)

### Tera Term使用说明
***
程序下载到开发板后，复位开始运行。通过TeraTerm可以看到程序执行过程中的信息。并且WIFI的用户名和密码也是通过Tera Term进行配置的。

第一次使用Tera Term时，请参考下面的步骤进行配置。
1. 终端设置


![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/30.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/31.png)

2. 串口通信设置

串口配置为：115200波特率，8位数据位，无奇偶校验，1位停止位。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/32.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/33.png)

3. 保存设置

不要忘记将前面的设置保存。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/34.png)

接下来可以开始运行程序啦。

### 运行程序
***
将B-L475E-IOT01A开发板通过USB插到电脑，打开Tera Term（选择对应的COM口）。按黑色的按键复位开发板。

在看到提示“Press the User button (Blue) within the next 5 seconds if you want to update the configuration”后5秒内按下板子上蓝色的按键。（如果之前没有输入过wifi信息，会直接提示输入SSID等内容）

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/35.png)

提示是否要更新WIFI的设置。输入“y”，然后依照提示输入wifi的用户名，加密方式和密码。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/36.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/37.png)

接下来，板子会连接到WIFI路由器，然后自动连接到之前在程序中设定好的MQTT服务器，向设定好的主题订阅消息/发布消息。

发布的消息就是板子上集成的传感器数据。

可以通过百度云端的客户端或者MQTT.fx订阅同样的主题来查看消息，或者发送消息给B-L475E-IOT01A开发板。

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/38.png)

![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/39.png)

#### 相关参考文档
***
1.	UM2153-Discovery kit for IoT node, multi-channel communication with STM32L4
2.	[百度云物接入IoT Hub产品文档](https://cloud.baidu.com/doc/IOT/ProductDescription.html#.E7.89.A9.E6.8E.A5.E5.85.A5.E4.BB.8B.E7.BB.8D)
