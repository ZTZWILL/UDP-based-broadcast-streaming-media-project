# UDP-based-broadcast-streaming-media-project
# 基于IPv4的流媒体广播系统
基于C语言和网络编程接口开发，实现了一个客户端/服务器模型（C/S）。该模型可以实现实现MP3格式音乐的组播，类似于广播电台的效果，客户端可以选择频道进行播放。服务器端采用多线程处理频道信息，实现媒体库（MP3）读取并进行流量控制（令牌桶），并通过UDP组播发送；客户端采用多进程，实现父进程接受来自网络的数据，通过进程间通信技术管道发送给子进程，子进程进行数据解码并输出到特定频道。

## 需求分析
- 实现一个音乐广播系统
- 考虑到流量控制
- 基于客户端/服务器模型，服务器设计成守护进程。
- 用户可选择音乐频道，同时考虑加入节目单频道

## 系统框架
<p align="center">
<img width = '1007' height = '1368' src = "https://github.com/ZTZWILL/UDP-based-broadcast-streaming-media-project/tree/main/images/server.jpg"/>
</p>

## 项目部署
### 编译
```powershell
git clone https://github.com/ZTZWILL/UDP-based-broadcast-streaming-media-project.git
cd clinet/
make
cd ../server
make
```
### 设置本地媒体库
|/var
<br/>|---media</br>
| --- | --- ch1
<br/>| --- | --- | --- desc.txt</br>
| --- | --- | --- 歌曲1.mp3
<br/>| --- | --- | --- 歌曲2.mp3</br>
| --- | --- | --- 歌曲3.mp3
<br/>| --- | --- ch2</br>
| --- | --- | --- desc.txt
<br/>| --- | --- | --- 歌曲4.mp3</br>
| --- | --- | --- 歌曲5.mp3
<br/>| --- | --- | --- 歌曲6.mp3</br>
| --- | --- ch3
<br/>| --- | --- | --- desc.txt</br>
| --- | --- | --- 歌曲7.mp3
<br/>| --- | --- ...</br>
**注**：频道目录chn（ch1, ch2, ch3...）位于 **/var/media** ，仅作为参考，用户可自行选择音乐库位置。频道描述由用户自己给出，该描述用于阐明该频道歌曲风格，以及其他信息，以供听众参考



## 后期优化的方向：
-  (1) 参考TCP的可靠传输，实现UDP的可靠传输，可参考实时流式传输协议RTSP。
-  (2) 客户端界面
-  (3) 使用内存池来创建节目单信息的缓存结构体，减少内存分配的开销
