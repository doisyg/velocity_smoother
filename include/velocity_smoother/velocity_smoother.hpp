/*
 * Copyright (c) 2012, Yujin Robot.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Yujin Robot nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
 ** Ifdefs
 *****************************************************************************/

#ifndef VELOCITY_SMOOTHER__VELOCITY_SMOOTHER_HPP_
#define VELOCITY_SMOOTHER__VELOCITY_SMOOTHER_HPP_

/*****************************************************************************
 ** Includes
 *****************************************************************************/

#include <algorithm>
#include <string>
#include <vector>

#include <geometry_msgs/msg/twist.hpp>
#include <rcl_interfaces/msg/set_parameters_result.hpp>
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>

/*****************************************************************************
** Namespaces
*****************************************************************************/

namespace velocity_smoother {

/*****************************************************************************
** VelocitySmoother
*****************************************************************************/

class VelocitySmoother final : public rclcpp::Node
{
public:
  explicit VelocitySmoother(const rclcpp::NodeOptions & options);

  ~VelocitySmoother() override;
  VelocitySmoother(VelocitySmoother && c) = delete;
  VelocitySmoother & operator=(VelocitySmoother && c) = delete;
  VelocitySmoother(const VelocitySmoother & c) = delete;
  VelocitySmoother & operator=(const VelocitySmoother & c) = delete;

private:
  enum RobotFeedbackType
  {
    NONE,
    ODOMETRY,
    COMMANDS
  } robot_feedback;  /**< What source to use as robot velocity feedback */

  bool quiet_;        /**< Quieten some warnings that are unavoidable because of velocity multiplexing. **/
  double speed_lim_v_, accel_lim_v_, decel_lim_v_;
  double speed_lim_w_, accel_lim_w_, decel_lim_w_;

  geometry_msgs::msg::Twist current_vel_;
  geometry_msgs::msg::Twist target_vel_;
  double last_cmd_vel_linear_x_;
  double last_cmd_vel_angular_z_;

  double period_;
  double decel_factor_;
  bool input_active_;
  double cb_avg_time_;
  rclcpp::Time last_velocity_cb_time_;
  std::vector<double> period_record_; /**< Historic of latest periods between velocity commands */
  unsigned int pr_next_; /**< Next position to fill in the periods record buffer */

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odometry_sub_;    /**< Current velocity from odometry */
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr current_vel_sub_; /**< Current velocity from commands sent to the robot, not necessarily by this node */
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr raw_in_vel_sub_;  /**< Incoming raw velocity commands */
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr  smooth_vel_pub_;  /**< Outgoing smoothed velocity commands */
  rclcpp::TimerBase::SharedPtr timer_;

  void timerCB();
  void velocityCB(const geometry_msgs::msg::Twist::SharedPtr msg);
  void robotVelCB(const geometry_msgs::msg::Twist::SharedPtr msg);
  void odometryCB(const nav_msgs::msg::Odometry::SharedPtr msg);

  rcl_interfaces::msg::SetParametersResult parameterUpdate(
    const std::vector<rclcpp::Parameter> & parameters);

  double sign(double x)  { return x < 0.0 ? -1.0 : +1.0; };

  double median(std::vector<double> & values) {
    // Return the median element of an doubles vector
    std::nth_element(values.begin(), values.begin() + values.size()/2, values.end());
    return values[values.size()/2];
  }
};

} // namespace velocity_smoother

#endif /* VELOCITY_SMOOTHER__VELOCITY_SMOOTHER_HPP_ */