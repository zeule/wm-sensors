// SPDX-License-Identifier: LGPL-3.0+

#include "./sensor_tree.hxx"

#include "visitor/chip_visitor.hxx"
#include "impl/chip_registrator.hxx"

#include <fmt/format.h>

#include <cassert>

wm_sensors::TreeNode::TreeNode(TreeNode* parent)
    : parent_{parent}
{
}

wm_sensors::TreeNode::FindResult wm_sensors::TreeNode::find(std::string_view path) const
{
	if (path.empty()) {
		return {this, {}};
	}

	if (path[0] == pathSeparator) {
		return find(path.substr(1));
	}

	std::size_t nextSep = path.find(pathSeparator);
	std::string childName = std::string(path.substr(0, nextSep));
	auto it = children_.find(childName);
	if (it == children_.end()) {
		return {this, path};
	}
	return nextSep == std::string_view::npos ? FindResult{it->second.get(), {}} : it->second->find(path.substr(nextSep));
	}

wm_sensors::TreeNode& wm_sensors::TreeNode::child(std::string_view path)
{
	const auto fr = find(path);
	if (fr.pathTail.empty()) {
		return const_cast<TreeNode&>(*fr.node);
	}

	TreeNode* node = const_cast<TreeNode*>(fr.node);
	std::string_view subPath = fr.pathTail;
	for (std::size_t nextSep = subPath.find(pathSeparator); nextSep != std::string::npos;
		nextSep = subPath.find(pathSeparator)) {
		node = node->children_.insert({std::string(path.substr(0, nextSep)), node->create(node)}).first->second.get();
		subPath = subPath.substr(nextSep);
	}
	return subPath.empty() ? *node :
                             *node->children_.insert({std::string(subPath), node->create(node)}).first->second.get();
}

const wm_sensors::TreeNode& wm_sensors::TreeNode::child(std::string_view path) const
{
	const auto fr = find(path);
	if (fr.pathTail.empty()) {
		return *fr.node;
	}
	throw std::runtime_error(fmt::format("Could not find sub-node '{0}', path tail: '{1}'", path, fr.pathTail));
}

wm_sensors::TreeNode::~TreeNode() = default;

wm_sensors::TreeNode::TreeNode(TreeNode&& other) noexcept = default;

wm_sensors::SensorsTree::SensorsTree()
    : sensors_{std::make_unique<SensorChipTreeNode>(nullptr)}
{
	impl::ChipProbesRegistry::instance().probeAll(*sensors_);
}

wm_sensors::SensorsTree::SensorsTree(SensorsTree&& other) = default;
