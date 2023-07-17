/*
 * Copyright (c) 2017 Daniel Koch and James Jackson, BYU MAGICC Lab.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
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
 */

/**
 * \file mag_cal.h
 * \author Jerel Nielsen <jerel.nielsen@gmail.com>
 * \author Devon Morris <devonmorris1992@gmail.com>
 */

#ifndef ROSFLIGHT_SENSORS_CALIBRATE_MAG_H
#define ROSFLIGHT_SENSORS_CALIBRATE_MAG_H

#include <rclcpp/rclcpp.hpp>
#include <message_filters/subscriber.h>

#include <rosflight_msgs/srv/param_set.hpp>

#include <sensor_msgs/msg/magnetic_field.hpp>

#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <random>

#include <eigen_stl_containers/eigen_stl_vector_container.h>
#include <vector>

namespace rosflight
{
/**
 * \brief CalibrateMag sensor class
 */
class CalibrateMag : public rclcpp::Node
{
public:
  CalibrateMag();

  void run();

  /**
   * \brief Begin the magnetometer calibration routine
   */
  void start_mag_calibration();

  void do_mag_calibration();

  /**
   * @brief set_refence_magnetic_field_strength
   * @param reference_magnetic_field
   */
  bool mag_callback(const sensor_msgs::msg::MagneticField::ConstSharedPtr &mag);

  void set_reference_magnetic_field_strength(double reference_magnetic_field);

  /**
   * \brief Check if a calibration is in progress
   * \return True if a calibration is currently in progress
   */
  bool is_calibrating() const { return calibrating_; }

  /// The const stuff is to make it read-only
  double a11() const { return A_(0, 0); }
  double a12() const { return A_(0, 1); }
  double a13() const { return A_(0, 2); }
  double a21() const { return A_(1, 0); }
  double a22() const { return A_(1, 1); }
  double a23() const { return A_(1, 2); }
  double a31() const { return A_(2, 0); }
  double a32() const { return A_(2, 1); }
  double a33() const { return A_(2, 2); }
  double bx() const { return b_(0, 0); }
  double by() const { return b_(1, 0); }
  double bz() const { return b_(2, 0); }

private:
  bool set_param(std::string name, double value);

  message_filters::Subscriber<sensor_msgs::msg::MagneticField> mag_subscriber_;

  rclcpp::Client<rosflight_msgs::srv::ParamSet>::SharedPtr param_set_client_;

  Eigen::MatrixXd A_, b_;

  double reference_field_strength_; //!< the strength of earth's magnetic field at your location

  bool calibrating_;        //!< whether a temperature calibration is in progress
  bool first_time_;         //!< waiting for first measurement for calibration
  double calibration_time_; //!< seconds to record data for temperature compensation
  double start_time_;       //!< timestamp of first calibration measurement
  int ransac_iters_;        //!< number of ransac iterations to fit ellipsoid to mag measurements
  int measurement_skip_;
  int measurement_throttle_;
  double inlier_thresh_; //!< threshold to consider measurement an inlier in ellipsoidRANSAC
  Eigen::Vector3d measurement_prev_;
  EigenSTL::vector_Vector3d measurements_;

  // function to perform RANSAC on ellipsoid data
  Eigen::MatrixXd ellipsoidRANSAC(EigenSTL::vector_Vector3d meas, int iters, double inlier_thresh);

  // function to vector from ellipsoid center to surface along input vector
  static Eigen::Vector3d intersect(const Eigen::Vector3d &r_m,
                                   const Eigen::Vector3d &r_e,
                                   const Eigen::MatrixXd &Q,
                                   const Eigen::MatrixXd &ub,
                                   double k);

  /*
      sort eigenvalues and eigenvectors output from Eigen library
  */
  static void eigSort(Eigen::MatrixXd &w, Eigen::MatrixXd &v);

  /*
      This function gets ellipsoid parameters via least squares on ellipsoidal data
      according to the paper: Li, Qingde, and John G. Griffiths. "Least squares ellipsoid
      specific fitting." Geometric modeling and processing, 2004. proceedings. IEEE, 2004.
  */
  static Eigen::MatrixXd ellipsoidLS(EigenSTL::vector_Vector3d meas);

  /*
      This function compute magnetometer calibration parameters according to Section 5.3 of the
      paper: Renaudin, Valérie, Muhammad Haris Afzal, and Gérard Lachapelle. "Complete triaxis
      magnetometer calibration in the magnetic domain." Journal of sensors 2010 (2010).
  */
  void magCal(Eigen::MatrixXd u, Eigen::MatrixXd &A, Eigen::MatrixXd &bb) const;
};

} // namespace rosflight

#endif // ROSFLIGHT_SENSORS_CALIBRATE_MAG_H