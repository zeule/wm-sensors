// SPDX-License-Identifier: GPL-3.0+
#ifndef WM_SENSORS_LIB_IMPL_GROUP_AFFINITY_HXX
#define WM_SENSORS_LIB_IMPL_GROUP_AFFINITY_HXX

namespace wm_sensors::impl {
	unsigned short processorGroupCount();

	class GroupAffinity {
	public:
		static GroupAffinity undefined();
		static GroupAffinity single(unsigned short group, unsigned index);

		GroupAffinity(unsigned short group, unsigned long long mask);

		static GroupAffinity set(GroupAffinity affinity);
		
		unsigned short group() const
		{
			return group_;
		}

		unsigned long long mask() const
		{
			return mask_;
		}

	private:
		unsigned long long mask_;
		unsigned short group_;
	};


	class ThreadGroupAffinityGuard {
	public:
		ThreadGroupAffinityGuard(GroupAffinity a);
		~ThreadGroupAffinityGuard();

		void release();

	private:
		GroupAffinity previous_;
		bool set_; // TODO atomic?
	};
}

#endif
