// SPDX-License-Identifier: LGPL-3.0+

#include "./ec.hxx"

#include "../../../../utility/utility.hxx"
#include "../../../impl/ring0.hxx"

#include <chrono>
#include <mutex>
#include <thread>

struct wm_sensors::hardware::motherboard::lpc::ec::EC::State {
	State(u16 cPort, u16 dPort) : mutex{}, waitReadFailures{0}, commandPort{cPort}, dataPort{dPort}
	{
	}

	DELETE_COPY_CTOR_AND_ASSIGNMENT(State)

	std::mutex mutex;
	int waitReadFailures;
	u16 commandPort;
	u16 dataPort;
};


namespace {
	using wm_sensors::hardware::impl::Ring0;
	using wm_sensors::hardware::motherboard::lpc::ec::EC;
	using namespace wm_sensors::stdtypes;

	using namespace std::chrono_literals;

	// see the ACPI specification chapter 12
	const u16 firstECCommandPort = 0x66;
	const u16 firstECDataPort = 0x62;

	enum class Status : u8
	{
		OutputBufferFull = 0x01, // EC_OBF
		InputBufferFull = 0x02, // EC_IBF
		Command = 0x08, // CMD
		BurstMode = 0x10, // BURST
		SciEventPending = 0x20, // SCI_EVT
		SmiEventPending = 0x40 // SMI_EVT
	};

	const int FailuresBeforeSkip = 20;
	const int MaxRetries = 5;

	// implementation
	const int WaitSpins = 50;


	bool waitForStatus(u16 port, Status status, bool isSet)
	{
		for (int i = 0; i < WaitSpins; i++) {
			u8 value = Ring0::instance().readIOPort(port);

			if ((wm_sensors::utility::to_underlying(status) & (!isSet ? value : ~value)) == 0) { return true; }

			std::this_thread::sleep_for(1ms);
		}

		return false;
	}

} // namespace

wm_sensors::hardware::motherboard::lpc::ec::EC::EC(u16 commandPort, u16 dataPort)
    : state_{std::make_unique<State>(commandPort, dataPort)}
{
}

wm_sensors::hardware::motherboard::lpc::ec::EC::~EC() = default;

wm_sensors::hardware::motherboard::lpc::ec::EC& wm_sensors::hardware::motherboard::lpc::ec::EC::first()
{
	static EC firstEC{firstECCommandPort, firstECDataPort};

	return firstEC;
}

void wm_sensors::hardware::motherboard::lpc::ec::EC::transaction(
    Command command, const u8* wdata, unsigned wdata_len, u8* rdata, unsigned rdata_len)
{
	std::lock_guard<std::mutex> lock{state_->mutex};

	if (!waitWrite()) { throw TransactionFailed(); }

	Ring0::instance().writeIOPOrt(state_->commandPort, utility::to_underlying(command));

	if (wdata_len) {}

	for (unsigned i = 0; i < wdata_len; i++) {
		if (waitWrite()) {
			Ring0::instance().writeIOPOrt(state_->dataPort, wdata[i]);
		} else {
			throw TransactionFailed();
		}
	}

	for (unsigned i = 0; i < rdata_len; i++) {
		if (waitWrite() && waitRead()) {
			rdata[i] = Ring0::instance().readIOPort(state_->dataPort);
		} else {
			throw TransactionFailed();
		}
	}
}

bool wm_sensors::hardware::motherboard::lpc::ec::EC::waitRead()
{
	if (state_->waitReadFailures > FailuresBeforeSkip) { return true; }

	if (waitForStatus(state_->commandPort, Status::OutputBufferFull, true)) {
		state_->waitReadFailures = 0;
		return true;
	}

	++state_->waitReadFailures;
	return false;
}

bool wm_sensors::hardware::motherboard::lpc::ec::EC::waitWrite()
{
	return waitForStatus(state_->commandPort, Status::InputBufferFull, false);
}

wm_sensors::u8 wm_sensors::hardware::motherboard::lpc::ec::EC::read(wm_sensors::stdtypes::u8 addr)
{
	u8 res;
	transaction(Command::Read, &addr, 1, &res, 1);
	return res;
}

void wm_sensors::hardware::motherboard::lpc::ec::EC::write(u8 addr, u8 val)
{
	u8 data[] = {addr, val};
	transaction(Command::Write, data, 2, nullptr, 0);
}

wm_sensors::hardware::motherboard::lpc::ec::EC::TransactionFailed::TransactionFailed()
    : std::runtime_error("ACPI EC transaction failed")
{
}
