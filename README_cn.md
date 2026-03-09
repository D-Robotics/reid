[English](./README.md) | 简体中文

# 功能介绍

人体Reid跟随算法示例, 是基于[Reid](https://github.com/KaiyangZhou/deep-person-reid.git)算法的量化部署方案, 通过对人体特征进行提取, 匹配, 完成对人体跟随的id输出。

# 开发环境

- 编程语言: C/C++
- 开发平台: X5/S100/S600
- 系统版本：Ubuntu 22.04/Ubuntu 24.04
- 编译工具链: Linux GCC 11.4.0/Linux GCC 13.3.0

# 编译

- X5版本：支持在 X5 Ubuntu 22.04 系统上编译和在PC上使用docker交叉编译两种方式。

- S100版本：支持在 S100 Ubuntu 22.04 系统上编译和在PC上使用docker交叉编译两种方式。

- S600版本：支持在 S600 Ubuntu 24.04 系统上编译和在PC上使用docker交叉编译两种方式。

同时支持通过编译选项控制编译pkg的依赖和pkg的功能。

## 依赖库

- opencv:3.4.5

ros package：

- dnn node
- cv_bridge
- sensor_msgs
- hbm_img_msgs
- ai_msgs

hbm_img_msgs为自定义的图片消息格式, 用于shared mem场景下的图片传输, hbm_img_msgs pkg定义在hobot_msgs中, 因此如果使用shared mem进行图片传输, 需要依赖此pkg。


## 编译选项

1、SHARED_MEM

- shared mem（共享内存传输）使能开关, 默认打开（ON）, 编译时使用-DSHARED_MEM=OFF命令关闭。
- 如果打开, 编译和运行会依赖hbm_img_msgs pkg, 并且需要使用tros进行编译。
- 如果关闭, 编译和运行不依赖hbm_img_msgs pkg, 支持使用原生ros和tros进行编译。
- 对于shared mem通信方式, 当前只支持订阅nv12格式图片。

## RDK Ubuntu系统上编译

1、编译环境确认

- 板端已安装X5 Ubuntu系统。
- 当前编译终端已设置TogetherROS环境变量：`source PATH/setup.bash`。其中PATH为TogetherROS的安装路径。
- 已安装ROS2编译工具colcon。安装的ROS不包含编译工具colcon, 需要手动安装colcon。colcon安装命令：`pip install -U colcon-common-extensions`
- 已编译dnn node package

2、编译

- 编译命令：`colcon build --packages-select reid`

## docker交叉编译

1、编译环境确认

- 在docker中编译, 并且docker中已经安装好TogetherROS。docker安装、交叉编译说明、TogetherROS编译和部署说明详见机器人开发平台robot_dev_config repo中的README.md。
- 已编译dnn node package
- 已编译hbm_img_msgs package（编译方法见Dependency部分）

2、编译

- 编译命令：

  ```shell
  # RDK X5
  bash robot_dev_config/build.sh -p X5 -s reid
  
  # RDK S100
  bash robot_dev_config/build.sh -p S100 -s reid

  # RDK S600
  bash robot_dev_config/build.sh -p S600 -s reid
  ```

- 编译选项中默认打开了shared mem通信方式。

## 注意事项


# 使用介绍

## 依赖

- mipi_cam package：发布图片msg
- usb_cam package：发布图片msg
- websocket package：渲染图片和ai感知msg

## 参数

| 参数名             | 解释                                  | 是否必须             | 数值类型 | 默认值                 |
| ------------------ | ------------------------------------- | -------------------- | ------------------- | ----------------------------------------------------------------------- |
| feed_type           | int         | 本地/订阅推理模式。0：加载本地图片；1：订阅图片话题                                                                         | 否       | 0/1                  | 0                            |
| is_sync_mode           | int         | 同步/异步推理模式。0：异步模式；1：同步模式                                                                         | 否       | 0/1                  | 0                            |
| model_file_name        | std::string | 推理使用的模型文件                                                                                                  | 否       | 根据实际模型路径配置 | config/reid.bin          |
| is_shared_mem_sub      | int         | 是否使用shared mem通信方式订阅图片消息。打开和关闭shared mem通信方式订阅图片的topic名分别为/hbmem_img和/image_raw。 | 0/1      | 0/1                  | 0                            |
| ai_msg_pub_topic_name  | std::string | 发布包含人体跟随ID结果的消息topic名                                                                         | 否       | 根据实际部署环境配置 | /perception/detection/reid    |
| ai_msg_sub_topic_name | std::string | 订阅包含人体框检测结果的消息topic名                                                                             | 否       | 根据实际部署环境配置 | /hobot_mono2d_body_detection |
| ros_img_topic_name | std::string | 订阅Ros图片话题消息topic名                                                                             | 否       | 根据实际部署环境配置 | /image_raw |


## 运行

- reid 使用到的模型在安装包'config'路径下。

- 编译成功后, 将生成的install路径拷贝到地平线RDK上（如果是在RDK上编译, 忽略拷贝步骤）, 并执行如下命令运行。

## RDK Ubuntu系统上运行

运行方式1, 使用可执行文件启动：
```shell
export COLCON_CURRENT_PREFIX=./install
source /opt/ros/humble/setup.bash
source /opt/tros/humble/setup.bash
source ./install/setup.bash
# config中为示例使用的模型, 回灌使用的本地图片
# 根据实际安装路径进行拷贝（docker中的安装路径为install/lib/reid/config/, 拷贝命令为cp -r install/lib/reid/config/ .）。
cp -r install/lib/reid/config/ .

# 运行模式1：
# 使用本地nv12格式图片进行回灌预测
ros2 run reid reid --ros-args -p feed_type:=0 -p dump_render_img:=1

# 运行模式2：
# 使用shared mem通信方式(topic为/hbmem_img)进行预测, 设置ai订阅话题名(/hobot_mono2d_body_detection)为并设置log级别为warn。同时在另一个窗口发送ai msg话题(topic为/hobot_mono2d_body_detection) 变更检测框
ros2 run reid reid --ros-args -p feed_type:=1 --ros-args --log-level warn -p ai_msg_sub_topic_name:="/hobot_mono2d_body_detection"

# 同时使用
ros2 launch mono2d_body_detection mono2d_body_detection.launch.py
```

运行方式2, 使用launch文件启动：
```shell
export COLCON_CURRENT_PREFIX=./install
source /opt/ros/humble/setup.bash
source /opt/tros/humble/setup.bash
source ./install/setup.bash
# config中为示例使用的模型, 根据实际安装路径进行拷贝
# 如果是板端编译（无--merge-install编译选项）, 拷贝命令为cp -r install/PKG_NAME/lib/PKG_NAME/config/ ., 其中PKG_NAME为具体的package名。
cp -r install/lib/reid/config/ .

# 配置MIPI摄像头
export CAM_TYPE=mipi

# 运行模式：启动launch文件, 单独启动 reid 节点
ros2 launch reid reid.launch.py
```

## Linux 系统上运行

```shell
export ROS_LOG_DIR=/userdata/
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./install/lib/

# config中为示例使用的模型, 回灌使用的本地图片
cp -r install/lib/reid/config/ .

# 运行模式1：
# 使用本地nv12格式图片进行回灌预测, 输入自定义类别
./install/lib/reid/reid --ros-args -p feed_type:=0 -p dump_render_img:=1

# 运行模式2：
# 使用订阅到的image msg(topic为/image)进行预测, 设置ai订阅话题名(/hobot_mono2d_body_detection)为并设置log级别为warn。同时在另一个窗口发送ai msg话题(topic为/hobot_mono2d_body_detection) 变更检测框
./install/lib/reid/reid --ros-args -p feed_type:=1 -p dump_render_img:=1 --ros-args --log-level warn -p ai_msg_sub_topic_name:="/hobot_mono2d_body_detection"
```

# 结果分析

## X5结果展示

log：

运行命令：`ros2 run reid reid --ros-args -p feed_type:=0 -p dump_render_img:=1`

```shell
[WARN] [1752818868.744934448] [reid_node]: Parameter:
 feed_type(0:local, 1:sub): 0
 db_file: reid.db
 model_file_name: config/reid.bin
 dump_render_img: 1
 is_sync_mode_: 0
 is_shared_mem_sub: 1
 threshold: 0.7
 ai_msg_pub_topic_name: /perception/detection/reid
 ai_msg_sub_topic_name: /hobot_mono2d_body_detection
 ros_img_topic_name: /image_raw
 sharedmem_img_topic_name: /hbmem_img
[INFO] [1752818868.745566409] [dnn]: Node init.
[INFO] [1752818868.745613659] [reid_node]: Set node para.
[INFO] [1752818868.745699326] [dnn]: Model init.
[BPU_PLAT]BPU Platform Version(1.3.6)!
[HBRT] set log level as 0. version = 3.15.55.0
[DNN] Runtime version = 1.24.5_(3.15.55 HBRT)
[A][DNN][packed_model.cpp:247][Model](2025-07-18,14:07:48.873.858) [HorizonRT] The model builder version = 1.24.4
[INFO] [1752818868.961298645] [dnn]: The model input 0 width is 128 and height is 256
[INFO] [1752818868.961517687] [dnn]:
Model Info:
name: reid.
[input]
 - (0) Layout: NCHW, Shape: [1, 3, 256, 128], Type: HB_DNN_IMG_TYPE_NV12.
[output]
 - (0) Layout: NCHW, Shape: [1, 512, 1, 1], Type: HB_DNN_TENSOR_TYPE_F32.

[INFO] [1752818868.961622021] [dnn]: Task init.
[INFO] [1752818868.963938447] [dnn]: Set task_num [4]
[INFO] [1752818868.964011406] [reid_node]: The model input width is 128 and height is 256
[INFO] [1752818868.994250860] [reid_node]: inputs.size(): 1, rois->size(): 1
[WARN] [1752818869.062804883] [reid_fet_manage]:
[WARN] [1752818869.159041351] [reid_fet_manage]: Query failed, storage: 1.
```