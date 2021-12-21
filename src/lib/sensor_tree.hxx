// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_SENSORS_TREE_HXX
#define WM_SENSORS_LIB_SENSORS_TREE_HXX

#include "./sensor.hxx"
#include "./sensor_path.hxx"
#include "./visitor/chip_visitor.hxx"

#include "utility/utility.hxx"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "wm-sensors_export.h"

namespace wm_sensors {

	class WM_SENSORS_EXPORT TreeNode {
	public:
		TreeNode& child(std::string_view path);
		const TreeNode& child(std::string_view path) const;

		virtual ~TreeNode();
		TreeNode(TreeNode&& other) noexcept;

		static const char pathSeparator = '/';

	protected:
		TreeNode(TreeNode* parent);

		const std::map<std::string, std::unique_ptr<TreeNode>>& children() const
		{
			return children_;
		}

	private:
		virtual std::unique_ptr<TreeNode> create(TreeNode* parent) = 0;
		struct FindResult {
			const TreeNode* node;
			std::string_view pathTail;
		};
		FindResult find(std::string_view path) const;

		TreeNode(const TreeNode& other) = delete;
		TreeNode& operator=(const TreeNode& other) = delete;

		TreeNode* parent_;
		std::map<std::string, std::unique_ptr<TreeNode>> children_;
	};

	template <class PayloadIdentifier, class Payload>
	class SensorTreeNode: protected TreeNode {
		using base = TreeNode;

	public:
		using ThisType = SensorTreeNode;
		using PayloadObject = utility::remove_smart_pointer_t<Payload>;
		using VisitorType = SensorTreeVisitor<PayloadObject>;

		SensorTreeNode(ThisType* parent)
		    : TreeNode{parent}
		{
		}

		ThisType& child(std::string_view path)
		{
			return static_cast<ThisType&>(base::child(path));
		}

		const ThisType& child(std::string_view path) const
		{
			return static_cast<const ThisType&>(base::child(path));
		}

		void addPayload(Payload&& payload)
		{
			payload_.push_back(std::move(payload));
		}

		const PayloadObject& payload(std::size_t index) const
		{
			if constexpr (std::is_same_v<Payload, PayloadObject>) {
				return payload_.at(index);
			} else {
				return *payload_.at(index);
			}
		}

		PayloadObject& payload(std::size_t index)
		{
			return const_cast<PayloadObject&>(const_cast<const ThisType*>(this)->payload(index));
		}

		void accept(VisitorType& visitor)
		{
			accept(std::string() + pathSeparator, {}, visitor);
		}

		template <class F, class... Args>
		std::unique_ptr<SensorTreeNode<PayloadIdentifier, std::invoke_result_t<F, PayloadObject, Args...>>>
		transform(F&& f, Args&&... args) const
		{
			using res_t = SensorTreeNode<PayloadIdentifier, std::invoke_result_t<F, PayloadObject, Args...>>;
			std::unique_ptr<res_t> res = std::make_unique<res_t>(nullptr);
			this->transform(*res, std::forward<F>(f), std::forward<Args>(args)...);
			return res;
		}

		using TreeNode::pathSeparator;

	private:
		SensorTreeNode(const SensorTreeNode&) = delete;
		SensorTreeNode& operator=(const SensorTreeNode&) = delete;

		void accept(const std::string& path, const std::string& nodeName, VisitorType& visitor)
		{
			typename VisitorType::NodeAddress address{path, nodeName};
			visitor.visit(address, *this);

			for (std::size_t i = 0; i < payload_.size(); ++i) {
				if constexpr (std::is_same_v<Payload, PayloadObject>) {
					visitor.visit(address, i, payload_[i]);
				} else {
					visitor.visit(address, i, *payload_[i]);
				}
			}
			for (const auto& childPair: children()) {
				static_cast<ThisType*>(childPair.second.get())
				    ->accept(path + childPair.first + pathSeparator, childPair.first, visitor);
			}
			visitor.ascend();
		}

		std::unique_ptr<TreeNode> create(TreeNode* parent) override
		{
			return std::unique_ptr<TreeNode>(new SensorTreeNode(static_cast<SensorTreeNode*>(parent)));
		}

		template <class F, class... Args>
		void transform(
		    SensorTreeNode<PayloadIdentifier, std::invoke_result_t<F, PayloadObject, Args...>>& res, F&& f,
		    Args&&... args) const
		{
			for (const auto& p: payload_) {
				if constexpr (std::is_same_v<Payload, PayloadObject>) {
					res.addPayload(std::invoke(std::forward<F>(f), p, std::forward<Args>(args)...));
				} else {
					res.addPayload(std::invoke(std::forward<F>(f), *p, std::forward<Args>(args)...));
				}
			}

			for (const auto& cp: children()) {
				static_cast<const ThisType*>(cp.second.get())
				    ->transform(res.child(cp.first), std::forward<F>(f), std::forward<Args>(args)...);
			}
		}

		std::vector<Payload> payload_;
	};

	using SensorChipTreeNode = SensorTreeNode<HardwareType, std::unique_ptr<SensorChip>>;
	using SensorChipVisitor = SensorTreeVisitor<SensorChip>;

	class WM_SENSORS_EXPORT SensorsTree {
	public:
		SensorsTree();
		SensorsTree(SensorsTree&& other);

		decltype(SensorChip::sensorAdded) sensorAdded;
		decltype(SensorChip::sensorRemoved) sensorRemoved;

		sigslot::signal<void(const SensorChipVisitor::NodeAddress& path, const SensorChipTreeNode::PayloadObject& chip)>
		    chipAdded;
		sigslot::signal<void(const SensorChipVisitor::NodeAddress& path, const SensorChipTreeNode::PayloadObject& chip)>
		    chipRemoved;

		const SensorChipTreeNode& chips() const
		{
			return *sensors_;
		}

		SensorChipTreeNode& chips()
		{
			return *sensors_;
		}
	private:
		SensorsTree(const SensorsTree&) = delete;
		SensorsTree& operator=(const SensorsTree&) = delete;

		std::unique_ptr<SensorChipTreeNode> sensors_;
	};

} // namespace wm_sensors

#endif
