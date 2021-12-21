// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_PORT_HXX
#define WM_SENSORS_LIB_HARDWARE_MOTHERBOARD_PORT_HXX

#include "../../utility/macro.hxx"
#include "../../utility/utility.hxx"

namespace wm_sensors::hardware::motherboard::lpc {
	struct IndexDataRegisters {
		u8 indexRegOffset;
		u8 dataRegOffset;
	};

	class Port {
	public:
		Port(u16 address);

		u8 inByte(u8 portOffset) const;
		void outByte(u8 portOffset, u8 value) const;

	private:
		u16 address_;
	};

	struct SingleBankAddress {
		u16 address;
		IndexDataRegisters regs;
	};

	class SingleBankPort: public Port {
		using base = Port;
	public:
		SingleBankPort(SingleBankAddress address);

		u8 readByte(u8 registerIndex) const;
		void writeByte(u8 registerIndex, u8 value) const;

		u16 readWord(u8 registerIndex) const;

		void select(u8 logicalDeviceNumber) const;

		const IndexDataRegisters regs() const {
			return regs_;
		}

	protected:
		void writeToRegister(IndexDataRegisters regs, u8 registerIndex, u8 value) const;
		u8 readFromRegister(IndexDataRegisters regs, u8 registerIndex) const;

	private:
		IndexDataRegisters regs_;
	};

	struct AddressWithBank: public SingleBankAddress {
		IndexDataRegisters bankSelectionPorts;
		u8 bankSelectionRegister;
	};

	class PortWithBanks: public SingleBankPort {
		using base = SingleBankPort;

	public:
		PortWithBanks(AddressWithBank a);
		PortWithBanks(SingleBankAddress a, IndexDataRegisters bankRegs, u8 bankSelectionRegister);
		PortWithBanks(SingleBankAddress address, u8 bankSelectionRegister);

		u8 readByte(u8 bank, u8 registerIndex) const;
		u8 readByte(u16 addr) const
		{
			return readByte(utility::hibyte(addr), utility::lobyte(addr));
		}
		void writeByte(u8 bank, u8 registerIndex, u8 value) const;
		void writeByte(u16 addr, u8 value) const
		{
			writeByte(utility::hibyte(addr), utility::lobyte(addr), value);
		}

		u16 readWord(u8 bank, u8 registerIndex) const;
		u16 readWord(u16 addr) const
		{
			return readWord(utility::hibyte(addr), utility::lobyte(addr));
		}

		void switchBank(u8 bank) const;
	private:
		IndexDataRegisters bankRegs_;
		u8 bankSelectionRegister_;
	};

	class PortGuard {
	protected:
		PortGuard(const SingleBankPort& port);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(PortGuard)

		const SingleBankPort& port_;
	};

	class WinbondNuvotonFintekEnterExit: protected PortGuard {
	public:
		WinbondNuvotonFintekEnterExit(const SingleBankPort& port);
		~WinbondNuvotonFintekEnterExit();

		DELETE_COPY_CTOR_AND_ASSIGNMENT(WinbondNuvotonFintekEnterExit)
	};

	class IT87EnterExit: protected PortGuard {
	public:
		IT87EnterExit(const SingleBankPort& port);
		~IT87EnterExit();

		DELETE_COPY_CTOR_AND_ASSIGNMENT(IT87EnterExit)
	};

	class SmscEnterExit: protected PortGuard {
	public:
		SmscEnterExit(const SingleBankPort& port);
		~SmscEnterExit();

		DELETE_COPY_CTOR_AND_ASSIGNMENT(SmscEnterExit)
	};

	void nuvotonDisableIOSpaceLock(const SingleBankPort& port);
} // namespace wm_sensors::hardware::motherboard::lpc

#endif
