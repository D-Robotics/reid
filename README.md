English| [简体中文](./README_cn.md)

Getting Started with reid
=======

# Feature Introduction

The reid package is a usage example based on [reid](https://github.com/KaiyangZhou/deep-person-reid.git) quantification deployment. The image data comes from local image feedback and subscribed image msg. The [reid](https://github.com/KaiyangZhou/deep-person-reid.git) relies on the input of the person's detection box, and extract the feature compared with the dataset, finally get the track id. 


# Development Environment

- Programming Language: C/C++
- Development Platform: X5
- System Version: Ubuntu 22.04
- Compilation Toolchain: Linaro GCC 11.4.0

# Compilation

- X5 Version: Supports compilation on the X5 Ubuntu system and cross-compilation using Docker on a PC.

It also supports controlling the dependencies and functionality of the compiled pkg through compilation options.

## Dependency Libraries

- OpenCV: 3.4.5

ROS Packages:

- dnn node
- cv_bridge
- sensor_msgs
- hbm_img_msgs
- ai_msgs

hbm_img_msgs is a custom image message format used for image transmission in shared memory scenarios. The hbm_img_msgs pkg is defined in hobot_msgs; therefore, if shared memory is used for image transmission, this pkg is required.

## Compilation Options

1. SHARED_MEM

- Shared memory transmission switch, enabled by default (ON), can be turned off during compilation using the -DSHARED_MEM=OFF command.
- When enabled, compilation and execution depend on the hbm_img_msgs pkg and require the use of tros for compilation.
- When disabled, compilation and execution do not depend on the hbm_img_msgs pkg, supporting compilation using native ROS and tros.
- For shared memory communication, only subscription to nv12 format images is currently supported.## Compile on X3/Rdkultra Ubuntu System

1. Compilation Environment Verification

- The X5 Ubuntu system is installed on the board.
- The current compilation terminal has set up the TogetherROS environment variable: `source PATH/setup.bash`. Where PATH is the installation path of TogetherROS.
- The ROS2 compilation tool colcon is installed. If the installed ROS does not include the compilation tool colcon, it needs to be installed manually. Installation command for colcon: `pip install -U colcon-common-extensions`.
- The dnn node package has been compiled.

2. Compilation

- Compilation command: `colcon build --packages-select reid`

## Docker Cross-Compilation for X5 Version

1. Compilation Environment Verification

- Compilation within docker, and TogetherROS has been installed in the docker environment. For instructions on docker installation, cross-compilation, TogetherROS compilation, and deployment, please refer to the README.md in the robot development platform's robot_dev_config repo.
- The dnn node package has been compiled.
- The hbm_img_msgs package has been compiled (see Dependency section for compilation methods).

2. Compilation

- Compilation command:

  ```shell
  # RDK X5
  bash robot_dev_config/build.sh -p X5 -s reid
  ```

- Shared memory communication method is enabled by default in the compilation options.

## Notes


# Instructions

## Dependencies

- mipi_cam package: Publishes image messages
- usb_cam package: Publishes image messages
- websocket package: Renders images and AI perception messages

## Parameters

| Parameter Name      | Explanation                            | Mandatory            | Type | Default Value       |                                                                  |
| ------------------- | -------------------------------------- | -------------------- | ------------------- |------------------------------------ |----------------------------------- |
| feed_type           | Image source, 0: local; 1: subscribe   | No                   | int |0                   |
| is_sync_mode   | 0: Synchronous Inference, 1: Asynchronous Inference | No  | int |0                   |
| model_file_name               | model file name                       | No        | string           | config/reid.bin     |
| is_shared_mem_sub   | Subscribe to images using shared memory communication method | No  | int |0                   |
| dump_render_img     | Whether to render, 0: no; 1: yes       | No                   | int |0                   |
| ai_msg_sub_topic_name | Topic name for subscribing ai msg to change detect box | No | string | /hobot_mono2d_body_detection |
| ai_msg_pub_topic_name | Topic name for publishing intelligent results for web display | No | string | /hobot_sam |
| ros_img_sub_topic_name | Topic name for subscribing image msg | No | string | /image_raw |

## Running

## Running on X5 Ubuntu System

Running method 1, use the executable file to start:
```shell
export COLCON_CURRENT_PREFIX=./install
source /opt/ros/humble/setup.bash
source /opt/tros/humble/setup.bash
source ./install/local_setup.bash
# The config includes models used by the example and local images for filling
# Copy based on the actual installation path (the installation path in the docker is install/lib/reid/config/, the copy command is cp -r install/lib/reid/config/ .).
cp -r install/lib/reid/config/ .

# Run mode 1:Use local JPG format images for backflow prediction:

ros2 run reid reid --ros-args -p feed_type:=0 -p dump_render_img:=1

# Run mode 2: Shared memory communication method (topic name: /hbmem_img), set the controlled topic name (topic name: /hobot_mono2d_body_detection) to and set the log level to warn. At the same time, send a ai topic (topic name: /hobot_mono2d_body_detection) in another window get the person body box:

ros2 run reid reid --ros-args -p feed_type:=1 --ros-args --log-level warn -p ai_msg_sub_topic_name:="/hobot_mono2d_body_detection"

ros2 launch mono2d_body_detection mono2d_body_detection.launch.py
```

Running method 2, use a launch file:

```shell
export COLCON_CURRENT_PREFIX=./install
source /opt/ros/humble/setup.bash
source /opt/tros/humble/setup.bash
source ./install/setup.bash
# Copy the configuration based on the actual installation path
cp -r install/lib/reid/config/ .

# Configure MIPI camera
export CAM_TYPE=mipi

# Start the launch file, run reid node only.
ros2 launch reid reid.launch.py
```

## Run on X5 Yocto system:

```shell
export ROS_LOG_DIR=/userdata/
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./install/lib/

# Copy the configuration used by the example and the local image used for inference
cp -r install/lib/reid/config/ .

# Run mode 1:Use local JPG format images for backflow prediction:

./install/lib/reid/reid --ros-args -p feed_type:=0 -p dump_render_img:=1

# Run mode 3: Shared memory communication method (topic name: /hbmem_img), set the controlled topic name (topic name: /hobot_mono2d_body_detection) to and set the log level to warn. At the same time, send a ai topic (topic name: /hobot_mono2d_body_detection) get the person body ai msg:

./install/lib/reid/reid --ros-args -p feed_type:=1 --ros-args --log-level warn -p ai_msg_sub_topic_name:="/hobot_mono2d_body_detection"
```

# Results Analysis

## X5 Results Display

log:

Command executed: `ros2 run reid reid --ros-args -p feed_type:=0 -p dump_render_img:=1`

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