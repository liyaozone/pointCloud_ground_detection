/*
 * Copyright (c) 2016-2017, Riccardo Monica
 *   RIMLab, Department of Engineering and Architecture
 *   University of Parma, Italy
 *   http://www.rimlab.ce.unipr.it/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef RVIZ_CLOUD_ANNOTATION_CLASS_H
#define RVIZ_CLOUD_ANNOTATION_CLASS_H

#include <rviz_cloud_annotation/RectangleSelectionViewport.h>
#include <rviz_cloud_annotation/UndoRedoState.h>
#include "point_neighborhood.h"
#include "rviz_cloud_annotation.h"
#include "rviz_cloud_annotation_points.h"
#include "rviz_cloud_annotation_undo.h"

#include <sys/stat.h>
#include <sys/types.h>

// STL
#include <dirent.h>
#include <iostream>

#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int32.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
// ROS
#include <interactive_markers/interactive_marker_server.h>
#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Empty.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt32.h>
#include <std_msgs/UInt64MultiArray.h>
#include <termios.h>

// PCL
#include <pcl/PCLPointCloud2.h>
#include <pcl/common/colors.h>
#include <pcl/conversions.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

// boost
#include <boost/lexical_cast.hpp>

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define _M_PI 3.1415926

#define DISTANCE_LIMMIT 20

#define HEIGHT_LIMMIT 2

#define _max(a, b) (((a) > (b)) ? (a) : (b))
#define _min(a, b) (((a) > (b)) ? (b) : (a))

class RVizCloudAnnotation  //点云标注Main类
{
public:
  typedef visualization_msgs::InteractiveMarker InteractiveMarker;
  typedef visualization_msgs::Marker Marker;
  // typedef visualization_msgs::MarkerArray MarkerArray;
  typedef interactive_markers::InteractiveMarkerServer InteractiveMarkerServer;
  typedef boost::shared_ptr<interactive_markers::InteractiveMarkerServer> InteractiveMarkerServerPtr;
  typedef visualization_msgs::InteractiveMarkerFeedback InteractiveMarkerFeedback;
  typedef visualization_msgs::InteractiveMarkerFeedbackPtr InteractiveMarkerFeedbackPtr;
  typedef visualization_msgs::InteractiveMarkerFeedbackConstPtr InteractiveMarkerFeedbackConstPtr;

  typedef pcl::PointXYZRGBNormal PointXYZRGBNormal;
  typedef pcl::PointCloud<PointXYZRGBNormal> PointXYZRGBNormalCloud;

  typedef pcl::Normal PointNormal;
  typedef pcl::PointCloud<PointNormal> PointNormalCloud;

  typedef pcl::PointXYZI PointXYZI;
  typedef pcl::PointCloud<PointXYZI> PointXYZICloud;

  typedef pcl::PointXYZRGB PointXYZRGB;

  typedef pcl::PointCloud<PointXYZRGB> PointXYZRGBCloud;
  typedef pcl::PointXYZRGBL PointXYZRGBL;
  typedef pcl::PointCloud<PointXYZRGBL> PointXYZRGBLCloud;

  typedef pcl::KdTreeFLANN<PointXYZRGBNormal> KdTree;

  typedef RVizCloudAnnotationPoints::CPData ControlPointData;
  typedef RVizCloudAnnotationPoints::CPDataVector ControlPointDataVector;

  typedef uint64_t uint64;
  typedef int64_t int64;
  typedef uint32_t uint32;
  typedef int32_t int32;
  typedef uint8_t uint8;
  typedef unsigned int uint;
  typedef std::vector<uint32> Uint32Vector;
  typedef std::vector<uint64> Uint64Vector;
  typedef std::vector<Uint64Vector> Uint64VectorVector;
  typedef std::vector<float> FloatVector;
  typedef std::vector<int64_t> Int64Vector;
  typedef std::vector<bool> BoolVector;
  typedef std::vector<std::string> StringVector;
  typedef std::vector<Eigen::Vector3f> PointVector;

  struct Int32PolyTriangle
  {
    int32 x[3];
    int32 y[3];
    Int32PolyTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3)
    {
      x[0] = x1;
      x[1] = x2;
      x[2] = x3;
      y[0] = y1;
      y[1] = y2;
      y[2] = y3;
    }

    // -1: inside 1: outside 0: on border
    int32 Contains(const int32 x, const int32 y) const;
  };
  typedef std::vector<Int32PolyTriangle> Int32PolyTriangleVector;

  enum ControlPointVisual
  {
    CONTROL_POINT_VISUAL_SPHERE,
    CONTROL_POINT_VISUAL_THREE_SPHERES,
    CONTROL_POINT_VISUAL_LINE,
  };

  RVizCloudAnnotation(ros::NodeHandle &nh);

  void LoadCloud(const std::string &filename, const std::string &normal_source, PointXYZRGBNormalCloud &cloud);

  void onSave(const std_msgs::String &filename_msg)
  {
    Save();
  }
  void onAutosave(const ros::TimerEvent &event)
  {
    Save(true);
  }
  void Save(const bool is_autosave = false);

  std::string AppendTimestampBeforeExtension(const std::string &filename);

  void onRestore(const std_msgs::String &filename_msg)
  {
    std::string filename = filename_msg.data.empty() ? m_annotation_filename_out : filename_msg.data;
    Restore(filename);
  }

  void Restore(const std::string &filename);

  void onClear(const std_msgs::UInt32 &label_msg);

  void onClickOnCloud(const InteractiveMarkerFeedbackConstPtr &feedback_ptr);

  std::string GetClickType(const std::string &marker_name, uint64 &label_out) const;

  uint64 GetClickedPointId(const InteractiveMarkerFeedback &click_feedback, bool &ok);

  void onRectangleSelectionViewport(const rviz_cloud_annotation::RectangleSelectionViewport &msg);

  void VectorSelection(const Uint64Vector &ids);

  Uint64Vector RectangleSelectionToIds(const Eigen::Matrix4f proj_matrix, const Eigen::Affine3f camera_pose_inv,
                                       const PointXYZRGBNormalCloud &cloud, const uint32 start_x, const uint32 start_y,
                                       const uint32 width, const uint32 height, const Int32PolyTriangleVector tri_cond,
                                       const float point_size, const float focal_length, const bool is_deep_selection);

  void SetCurrentLabel(const uint64 label);

  void SetEditMode(const uint64 new_edit_mode);

  void onViewLabels(const std_msgs::Bool &msg)
  {
    m_view_labels = msg.data;

    SendCloudMarker(false);
    SendControlPointsMarker(RangeUint64(1, m_annotation->GetNextLabel()), true);
  }

  void onViewControlPoints(const std_msgs::Bool &msg)
  {
    m_view_control_points = msg.data;

    SendCloudMarker(false);
    SendControlPointsMarker(RangeUint64(1, m_annotation->GetNextLabel()), true);
  }

  void onViewCloud(const std_msgs::Bool &msg)
  {
    m_view_cloud = msg.data;

    SendCloudMarker(false);
    SendControlPointsMarker(RangeUint64(1, m_annotation->GetNextLabel()), true);
  }

  void onSetCurrentLabel(const std_msgs::UInt32 &msg)
  {
    SetCurrentLabel(msg.data);
  }

  void onSetEditMode(const std_msgs::UInt32 &msg)
  {
    SetEditMode(msg.data);
  }

  void onSetAnnotationType(const std_msgs::UInt32 &type)
  {
    ANNOTATION_TYPE = type.data;
    ROS_INFO("rviz_cloud_annotation: annotation type: %u", type.data);
  }

  void onToggleNoneMode(const std_msgs::Empty &msg)
  {
    if ((m_edit_mode == EDIT_MODE_NONE) && (m_prev_edit_mode != EDIT_MODE_NONE))
    {
      SetEditMode(m_prev_edit_mode);
    }
    else if (m_edit_mode != EDIT_MODE_NONE)
    {
      SetEditMode(EDIT_MODE_NONE);
      ROS_INFO("rviz_cloud_annotation: OK7");
    }
  }

  void onSetName(const std_msgs::String &msg)
  {
    // m_undo_redo.SetNameForLabel(m_current_label, msg.data);
    ROS_INFO("rviz_cloud_annotation: New File Path");
    // SendName();
    // SendUndoRedoState();
    m_dataset_files.clear();
    m_annotation_cloud_files.clear();
    m_annotation_file_files.clear();
    m_bbox_files.clear();
    m_label_files.clear();

    FILE_ID = 0;

    std::string param_stringA = msg.data + "/";
    std::string param_stringB = param_stringA + "_pcd/";
    std::string param_stringC = param_stringA + "_annotation/";
    std::string param_stringD = param_stringA + "_bbox/";
    std::string param_stringE = param_stringA + "_label/";
    std::string param_stringF;
    mkdir(param_stringA.c_str(), S_IRWXU);
    mkdir(param_stringB.c_str(), S_IRWXU);
    mkdir(param_stringC.c_str(), S_IRWXU);
    mkdir(param_stringD.c_str(), S_IRWXU);
    mkdir(param_stringE.c_str(), S_IRWXU);
    LoadDataSet(param_stringA, param_stringB, param_stringC, param_stringD, param_stringE, param_stringF);

    InitNewCloud(*nh);
  }

  void onUndo(const std_msgs::Empty &);
  void onRedo(const std_msgs::Empty &);

  void onPointSizeChange(const std_msgs::Int32 &msg);

  void onControlPointWeightChange(const std_msgs::Int32 &msg);

  void onControlYawChange(const std_msgs::Int32 &msg);

  void onControlOccludedChange(const std_msgs::Int32 &msg);

  void onControlBiasChange(const std_msgs::Float32MultiArray &msg);

  void onGotoFirstUnused(const std_msgs::Empty &);
  void onGotoLastUnused(const std_msgs::Empty &);
  void onGotoFirst(const std_msgs::Empty &);
  void onGotoNextUnused(const std_msgs::Empty &);

  std::string label2Name(int label);

  void SendName();

  void SendBiasZero();

  void SendUndoRedoState();

  void SendPointCounts(const Uint64Vector &labels);

  Uint64Vector RangeUint64(const uint64 start, const uint64 end) const
  {
    Uint64Vector result(end - start);
    for (uint64 i = start; i < end; i++)
      result[i - start] = i;
    return result;
  }

  void SendCloudMarker(const bool apply);

  void ClearControlPointsMarker(const Uint64Vector &indices, const bool apply);

  void SendControlPointsMarker(const Uint64Vector &changed_labels, const bool apply);

  void SendControlPointMaxWeight();

  void SendYawMax();

  void SendYawMin();

  InteractiveMarker ControlPointsToMarker(const PointXYZRGBNormalCloud &cloud,
                                          const ControlPointDataVector &control_points, const uint64 label,
                                          const bool interactive);
  InteractiveMarker LabelsToMarker(const PointXYZRGBNormalCloud &cloud, const Uint64Vector &labels, const uint64 label,
                                   const bool interactive);

  InteractiveMarker CloudToMarker(const PointXYZRGBNormalCloud &cloud, const bool interactive);

  void colorize_point_cloud(double intensity, PointXYZRGB *sample);

private:
  ros::NodeHandle &m_nh;
  InteractiveMarkerServerPtr m_interactive_marker_server;
  PointXYZRGBNormalCloud::Ptr m_cloud;

  RVizCloudAnnotationPoints::ConstPtr m_annotation;
  RVizCloudAnnotationUndo m_undo_redo;

  KdTree::Ptr m_kdtree;

  ros::Subscriber m_set_edit_mode_sub;
  ros::Subscriber m_toggle_none_sub;
  ros::Subscriber m_set_current_label_sub;

  ros::Publisher m_set_edit_mode_pub;
  ros::Publisher m_set_current_label_pub;

  ros::Subscriber m_set_name_sub;
  ros::Publisher m_set_name_pub;

  ros::Subscriber m_rect_selection_sub;

  ros::Subscriber m_save_sub;
  ros::Subscriber m_restore_sub;
  ros::Subscriber m_clear_sub;
  ros::Subscriber m_new_sub;

  ros::Subscriber m_next_sub;
  ros::Subscriber m_pre_sub;

  ros::Subscriber m_undo_sub;
  ros::Subscriber m_redo_sub;
  ros::Publisher m_undo_redo_state_pub;

  ros::Subscriber m_point_size_change_sub;

  ros::Subscriber m_goto_first_unused_sub;
  ros::Subscriber m_goto_last_unused_sub;
  ros::Subscriber m_goto_first_sub;
  ros::Subscriber m_goto_next_unused_sub;

  ros::Subscriber m_view_control_points_sub;
  ros::Subscriber m_view_cloud_sub;
  ros::Subscriber m_view_labels_sub;
  bool m_view_cloud;
  bool m_view_labels;
  bool m_view_control_points;

  ros::Publisher m_point_count_update_pub;

  ros::Subscriber m_control_points_weight_sub;
  ros::Publisher m_control_point_weight_max_weight_pub;

  ros::Subscriber m_bbox_occluded_sub;

  ros::Subscriber m_control_yaw_sub;
  ros::Publisher m_control_yaw_max_pub;
  ros::Publisher m_control_yaw_min_pub;

  ros::Publisher m_control_bias_zero_pub;
  ros::Subscriber m_control_bias_sub;

  uint32 m_control_point_weight_step;
  uint32 m_control_point_max_weight;

  ros::Subscriber m_on_set_annotation_type_sub;

  int32 m_control_yaw_step;
  int32 m_control_yaw_min;
  int32 m_control_yaw_max;

  ros::Timer m_autosave_timer;
  bool m_autosave_append_timestamp;

  std::string m_frame_id;
  float m_point_size;
  float m_label_size;
  float m_control_label_size;

  float m_point_size_multiplier;
  float m_point_size_change_multiplier;

  bool m_show_points_back_labels;
  float m_cp_weight_scale_fraction;
  ControlPointVisual m_control_points_visual;
  bool m_show_zero_weight_control_points;

  uint64 m_current_label;
  uint64 m_edit_mode;
  uint64 m_prev_edit_mode;

  PointNeighborhood::ConstPtr m_point_neighborhood;

  std::string m_annotation_filename_in;
  std::string m_annotation_filename_out;
  std::string m_ann_cloud_filename_out;
  std::string m_label_names_filename_out;

  std::string m_bbox_save;

  // My defined data:
private:
  std::string m_dataset_folder;
  std::string m_annotation_cloud_folder;
  std::string m_annotation_file_folder;
  std::string m_bbox_folder;
  std::string m_line_folder;
  std::string m_label_folder;

  std::string m_log_file;

  StringVector m_dataset_files;
  StringVector m_annotation_cloud_files;
  StringVector m_annotation_file_files;
  StringVector m_bbox_files;
  StringVector m_line_files;
  StringVector m_label_files;

  int FILE_ID = 0;

  ros::Publisher bbox_marker_pub;

  ros::Publisher kerb_marker_pub;

  ros::Publisher plane_marker_pub;

  ros::Publisher lane_marker_pub;

  const uint BBOX = 0u;
  const uint PLANE = 1u;
  const uint KERB = 2u;
  const uint LANE = 3u;

  // MarkerArray LineMarkers;

  uint ANNOTATION_TYPE = BBOX;

  int BBOX_ID = 0;

  int KERB_ID = 0;

  int LANE_ID = 0;

  int PLANE_ID = 0;

  float m_box_bias[100][6];

  float BBOX_YAW = 0;

  float KERB_YAW = 0;

  bool if_tilt = false;

  Uint64Vector ids_in_bbox[100];

  Uint64Vector ids_in_kerb;

  Uint64Vector ids_in_lane;

  Uint64Vector ids_in_plane;

  Uint64Vector ids_out_plane;

  Uint64Vector ids_in_plane_flag;

  float BBOX_SET[100][11];

  float BBOX_LABEL_SET[100][10];

  float KERB_SET[100][3];

  int KERB_SIZE = 0;

  float LANE_SET[100][3];

  int LANE_SIZE = 0;

  float m_bbox_occluded[100];

  Int64Vector m_label;

  Int64Vector m_plane_flag;

  std::string param_string2;

  ros::NodeHandle *nh;

  // My defined function:
public:
  void InitNewCloud(ros::NodeHandle &nh);

  void LoadDataSet(std::string A, std::string B, std::string C, std::string D, std::string E, std::string F);

  void AddBbox(float A, float B, float B1, float B2, float B3, float B4, float C1, float C2, bool tilt);

  void onNew(const std_msgs::UInt32 &label_msg);

  void EmptyBboxToMarker(const int id);

  void EmptyLaneToMarker(const int id);

  void EmptyPlaneToMarker(const int id);

  void EmptyKerbToMarker(const int id);

  void FinalLabel(PointXYZRGBNormalCloud &cloud);

  bool InBbox(float x, float y, float z, int i);

  bool InKerb(float x, float y, float z, uint64 id);

  bool InLane(float x, float y, float z, uint64 id);

  bool InPlane(float x, float y, float z, uint64 id);

  void BboxToMarker(const float shape[], const int id, bool if_tilt);

  RVizCloudAnnotation::Uint64Vector RecountIds(const Uint64Vector &ids);

  void generateLane(const PointXYZRGBNormalCloud &cloud);

  void generatePlane(const PointXYZRGBNormalCloud &cloud);

  void generateKerb(const PointXYZRGBNormalCloud &cloud);

  void generateBbox(const PointXYZRGBNormalCloud &cloud, bool if_tilt);

  void onNextObject(const std_msgs::Empty &)
  {
    BBOX_ID++;
    ROS_INFO("rviz_cloud_annotation:current BBOX ID: %i", BBOX_ID);
    SendControlPointMaxWeight();
    SendYawMin();
    SendBiasZero();
  }

  void gotoNextObject()
  {
    BBOX_ID++;
    ROS_INFO("rviz_cloud_annotation:current BBOX ID: %i", BBOX_ID);
    SendControlPointMaxWeight();
    SendYawMin();
    SendBiasZero();
  }

  void onPreObject(const std_msgs::Empty &)
  {
    if (BBOX_ID > 0)
    {
      BBOX_ID--;
    }
    ROS_INFO("rviz_cloud_annotation:current BBOX ID: %i", BBOX_ID);
  }

  float line(float v);
  float line(float x, float y, bool axis);

  void int2str(const int &int_temp, std::string &string_temp)
  {
    std::stringstream stream;
    stream << int_temp;
    string_temp = stream.str();  //此处也可以用 stream>>string_temp
  }
  float m_sqrt(float x)
  {
    float half_x = 0.5 * x;
    int i = *((int *)&x);              // 以整数方式读取X
    i = 0x5f3759df - (i >> 1);         // 神奇的步骤
    x = *((float *)&i);                // 再以浮点方式读取i
    x = x * (1.5 - (half_x * x * x));  // 牛顿迭代一遍提高精度
    return 1 / x;
  }
};

#endif  // RVIZ_CLOUD_ANNOTATION_CLASS_H