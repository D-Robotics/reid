// Copyright (c) 2025，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"

#include "include/feature_manage.h"

float cosine_similarity(const float* data1, const float* data2) {
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    float result = 0.0f;

    for (int i = 0; i < 512; ++i) {
        result += data1[i] * data2[i];
        norm1 += data1[i] * data1[i];
        norm2 += data2[i] * data2[i];
    }

    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);

    if (norm1 == 0 || norm2 == 0) {
        return 0.0f;
    }

    return result / (norm1 * norm2);
}

FeatureManage::FeatureManage(std::string db_file, int feature_size, float threshold) {
  feature_size_ = feature_size;
  threshold_ = threshold;
  db.initialize(db_file);

  if (!db.createTable()) {
    RCLCPP_ERROR(rclcpp::get_logger("reid_fet_manage"),
      "Failed to create table.");
    return;
  }
}

int32_t FeatureManage::Parse(
    std::shared_ptr<TrackIdResult> &output,
    std::shared_ptr<DNNTensor> &output_tensor,
    std::shared_ptr<std::vector<hbDNNRoi>> rois,
    std::shared_ptr<NV12PyramidInput> pyramid) {

  if (rois == nullptr || static_cast<int>(rois->size()) == 0) {
    RCLCPP_INFO(rclcpp::get_logger("reid_fet_manage"), "get null rois");
    return -1;
  }

  if (!output_tensor) {
    RCLCPP_ERROR(rclcpp::get_logger("reid_fet_manage"), "invalid out tensor");
    return -1;
  }

  std::shared_ptr<TrackIdResult> result = nullptr;
  if (!output) {
    result = std::make_shared<TrackIdResult>();
    result->Reset();
    output = result;
  } else {
    result = std::dynamic_pointer_cast<TrackIdResult>(output);
    result->Reset();
  }
  result->ids.resize(rois->size());

  hbSysFlushMem(&(output_tensor->sysMem[0]), HB_SYS_MEM_CACHE_INVALIDATE);
  float* data = reinterpret_cast<float*>(output_tensor->sysMem[0].virAddr);

  // 取对应的float_tensor解析
  for (int roi_idx = 0; roi_idx < static_cast<int>(rois->size()); roi_idx++) {
    std::vector<float> feature(feature_size_);
    std::copy(data + roi_idx * feature_size_, data + (roi_idx + 1) * feature_size_, feature.begin());
    int ret = UpdateReid(feature, roi_idx, result);
    if (pyramid && ret != 0) {
      std::string file_name = std::to_string(ret) + ".jpg";
      Render(pyramid, rois->at(roi_idx), file_name);
    }
  }
  return 0;
}

int FeatureManage::UpdateReid(
    std::vector<float> &feature,
    const int roi_idx,
    std::shared_ptr<TrackIdResult> &output) {

  std::vector<Item> target_items;
  int ret = Query(feature.data(), target_items);

  if (target_items.size() == 1) {
    output->ids[roi_idx] = target_items[0].id;
    RCLCPP_INFO(rclcpp::get_logger("reid_fet_manage"),
          "Query success, result: %d.", target_items[0].id);
  } else {    
    int num = db.getItemCount();
    int newid = num + 1;
    output->ids[roi_idx] = newid;

    Item Item;
    Item.url = std::to_string(newid) + ".jpg";
    Item.feature.assign(feature.begin(), feature.end()); // 复制特征值
    if (!db.insertItem(Item)) {
      RCLCPP_ERROR(rclcpp::get_logger("reid_fet_manage"), "Failed to insert item.");
    }
    ret = newid;
    RCLCPP_WARN(rclcpp::get_logger("reid_fet_manage"),
      "Query failed, storage: %d.", newid);
  }
  return ret;
}


// 定义函数，过滤出 type 为 false 的 Item
int FeatureManage::Query(const float *data,
              std::vector<Item>& target_items) {

  int num = db.getItemCount();
  RCLCPP_DEBUG(rclcpp::get_logger("reid_fet_manage"),
          "Query start, num of database: %d.", num);
  // int page_num = 10;
  int page_num = 1;
  num = (num / page_num) + 1;

  std::stringstream sss;
  for (int i = 0; i < num; i++) {
    std::vector<Item> image_items = db.queryItemsByPage(i, page_num);
    
    for (int j = 0; j < image_items.size(); j++) {
      auto item = image_items[j];
      const float* data_image = item.feature.data();
      item.similarity = cosine_similarity(data_image, data);
      std::stringstream ss;
      ss << "id: " << page_num * i + j
          << ", item similarity: " << item.similarity;
      RCLCPP_INFO(rclcpp::get_logger("reid_fet_manage"), "%s", ss.str().c_str());
      sss << "\nid: " << page_num * i + j
          << ", item similarity: " << item.similarity;
      if (item.similarity < threshold_) {
        continue;
      }
      target_items.push_back(item);
    }
  }

  // 降序排序
  std::sort(target_items.begin(), target_items.end(), compareBySimilarity);

  // 检查向量大小并删除多余元素
  if (target_items.size() > 1) {
    target_items.erase(target_items.begin() + 1, target_items.end());
  } else if (target_items.size() == 0){
    RCLCPP_WARN(rclcpp::get_logger("reid_fet_manage"), "%s", sss.str().c_str());
  }

  return 0;
}

int FeatureManage::Render(const std::shared_ptr<NV12PyramidInput>& pyramid,
                           hbDNNRoi &roi,
                           std::string &file_name) {

  char* y_img = reinterpret_cast<char*>(pyramid->y_vir_addr);
  char* uv_img = reinterpret_cast<char*>(pyramid->uv_vir_addr);
  auto height = pyramid->height;
  auto width = pyramid->width;
  auto img_y_size = height * width;
  auto img_uv_size = img_y_size / 2;
  char* buf = new char[img_y_size + img_uv_size];
  memcpy(buf, y_img, img_y_size);
  memcpy(buf + img_y_size, uv_img, img_uv_size);
  cv::Mat nv12(height * 3 / 2, width, CV_8UC1, buf);
  cv::Mat bgr;
  cv::cvtColor(nv12, bgr, CV_YUV2BGR_NV12);
  delete[] buf;

  cv::Rect roi_rect(roi.left, roi.top, roi.right - roi.left, roi.bottom - roi.top);
  cv::Mat roi_img = bgr(roi_rect);
  cv::imwrite(file_name, roi_img);

  return 0;
}