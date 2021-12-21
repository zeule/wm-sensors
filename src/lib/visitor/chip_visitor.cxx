// SPDX-License-Identifier: LGPL-3.0+

#include "./chip_visitor.hxx"

void wm_sensors::SensorTreeVisitorBase::visit(const NodeAddress& /*address*/, const TreeNode& /*node*/) {}

void wm_sensors::SensorTreeVisitorBase::ascend() {}

wm_sensors::SensorTreeVisitorBase::~SensorTreeVisitorBase() = default;
