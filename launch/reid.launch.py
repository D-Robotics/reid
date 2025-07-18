# Copyright (c) 2025，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from launch import LaunchDescription
from launch_ros.actions import Node

from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python import get_package_share_directory
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    web_smart_topic_arg = DeclareLaunchArgument(
        'smart_topic',
        default_value='/perception/detection/reid',
        description='websocket smart topic')

    threshold_topic_arg = DeclareLaunchArgument(
        'reid_threshold',
        default_value="0.70",
        description='websocket smart topic')

    mono2d_body_det_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('mono2d_body_detection'),
                'launch/mono2d_body_detection.launch.py')),
        launch_arguments={
            'smart_topic': LaunchConfiguration('smart_topic'),
            'mono2d_body_pub_topic': '/hobot_mono2d_body_detection'
        }.items()
    )

    reid_det_node = Node(
        package='reid',
        executable='reid',
        output='screen',
        parameters=[
            {"is_sync_mode": 1},
            {"threshold": LaunchConfiguration('reid_threshold')},
            {"ai_msg_pub_topic_name": LaunchConfiguration('smart_topic')},
            {"ai_msg_sub_topic_name": "/hobot_mono2d_body_detection"}
        ],
        arguments=['--ros-args', '--log-level', 'warn']
    )

    return LaunchDescription([
        web_smart_topic_arg,
        threshold_topic_arg,
        mono2d_body_det_node,
        reid_det_node
    ])
