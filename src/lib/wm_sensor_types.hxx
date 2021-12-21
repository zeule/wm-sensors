// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_TYPES_HXX
#define WM_SENSORS_LIB_TYPES_HXX

#include "./utility/utility.hxx"
#include "./stdint.hxx"

#include <cstdint>
#include <type_traits>
#ifndef NDEBUG
#	include <limits>
#	include <stdexcept>
#endif

namespace wm_sensors {
		// The following enums are copied from hwmon.h
	enum class SensorType : u8
	{
		chip,
		temp, // [℃]
		in,   // [V]
		voltage = in,
		curr,     // [A]
		power,    // [W]
		energy,   // [J]
		humidity, // [%]
		fan,      // [RPM]
		pwm,
		intrusion,
		hwmonMax,
		// extended sensor types, not recognized by libsensors/sensors
		data = hwmonMax, // [B]
		dataRate,        // [B/s]
		duration,        // [s]
		frequency,       // [Hz]
		flow,            // [litres/s]
		load,            // [%]
		raw,             // Raw number
		fraction,        // [0;1], might be displayed in %
		max,
	};

	enum class SensorVisibility
	{
		Readable,
		Writable
	};

	constexpr inline u8 sensor_type_max = utility::to_underlying(SensorType::max);

	namespace attributes { // TODO migrate to enum class + using enum
		using wm_sensors::utility::bit;

		enum generic_attributes
		{
			generic_enable_bit,
			generic_input_bit,
			generic_label_bit
		};

		constexpr inline const u32 generic_enable = bit<u32>(generic_enable_bit);
		constexpr inline const u32 generic_input = bit<u32>(generic_input_bit);
		constexpr inline const u32 generic_label = bit<u32>(generic_label_bit);

		enum chip_attributes
		{
			chip_temp_reset_history_bit,
			chip_in_reset_history_bit,
			chip_curr_reset_history_bit,
			chip_power_reset_history_bit,
			chip_register_tz_bit,
			chip_update_interval_bit,
			chip_alarms_bit,
			chip_samples_bit,
			chip_curr_samples_bit,
			chip_in_samples_bit,
			chip_power_samples_bit,
			chip_temp_samples_bit,
			chip_bit_max
		};

		constexpr inline const u32 chip_temp_reset_history = bit<u32>(chip_temp_reset_history_bit);
		constexpr inline const u32 chip_in_reset_history = bit<u32>(chip_in_reset_history_bit);
		constexpr inline const u32 chip_curr_reset_history = bit<u32>(chip_curr_reset_history_bit);
		constexpr inline const u32 chip_power_reset_history = bit<u32>(chip_power_reset_history_bit);
		constexpr inline const u32 chip_register_tz = bit<u32>(chip_register_tz_bit);
		constexpr inline const u32 chip_update_interval = bit<u32>(chip_update_interval_bit);
		constexpr inline const u32 chip_alarms = bit<u32>(chip_alarms_bit);
		constexpr inline const u32 chip_samples = bit<u32>(chip_samples_bit);
		constexpr inline const u32 chip_curr_samples = bit<u32>(chip_curr_samples_bit);
		constexpr inline const u32 chip_in_samples = bit<u32>(chip_in_samples_bit);
		constexpr inline const u32 chip_power_samples = bit<u32>(chip_power_samples_bit);
		constexpr inline const u32 chip_temp_samples = bit<u32>(chip_temp_samples_bit);

		enum temp_attributes
		{
			temp_enable_bit = generic_enable_bit,
			temp_input_bit = generic_input_bit,
			temp_label_bit = generic_label_bit,
			temp_type_bit,
			temp_lcrit_bit,
			temp_lcrit_hyst_bit,
			temp_min_bit,
			temp_min_hyst_bit,
			temp_max_bit,
			temp_max_hyst_bit,
			temp_crit_bit,
			temp_crit_hyst_bit,
			temp_emergency_bit,
			temp_emergency_hyst_bit,
			temp_alarm_bit,
			temp_lcrit_alarm_bit,
			temp_min_alarm_bit,
			temp_max_alarm_bit,
			temp_crit_alarm_bit,
			temp_emergency_alarm_bit,
			temp_fault_bit,
			temp_offset_bit,
			temp_lowest_bit,
			temp_highest_bit,
			temp_reset_history_bit,
			temp_rated_min_bit,
			temp_rated_max_bit,
			temp_bit_max
		};

		constexpr inline const u32 temp_enable = bit<u32>(temp_enable_bit);
		constexpr inline const u32 temp_input = bit<u32>(temp_input_bit);
		constexpr inline const u32 temp_label = bit<u32>(temp_label_bit);
		constexpr inline const u32 temp_type = bit<u32>(temp_type_bit);
		constexpr inline const u32 temp_lcrit = bit<u32>(temp_lcrit_bit);
		constexpr inline const u32 temp_lcrit_hyst = bit<u32>(temp_lcrit_hyst_bit);
		constexpr inline const u32 temp_min = bit<u32>(temp_min_bit);
		constexpr inline const u32 temp_min_hyst = bit<u32>(temp_min_hyst_bit);
		constexpr inline const u32 temp_max = bit<u32>(temp_max_bit);
		constexpr inline const u32 temp_max_hyst = bit<u32>(temp_max_hyst_bit);
		constexpr inline const u32 temp_crit = bit<u32>(temp_crit_bit);
		constexpr inline const u32 temp_crit_hyst = bit<u32>(temp_crit_hyst_bit);
		constexpr inline const u32 temp_emergency = bit<u32>(temp_emergency_bit);
		constexpr inline const u32 temp_emergency_hyst = bit<u32>(temp_emergency_hyst_bit);
		constexpr inline const u32 temp_alarm = bit<u32>(temp_alarm_bit);
		constexpr inline const u32 temp_min_alarm = bit<u32>(temp_min_alarm_bit);
		constexpr inline const u32 temp_max_alarm = bit<u32>(temp_max_alarm_bit);
		constexpr inline const u32 temp_crit_alarm = bit<u32>(temp_crit_alarm_bit);
		constexpr inline const u32 temp_lcrit_alarm = bit<u32>(temp_lcrit_alarm_bit);
		constexpr inline const u32 temp_emergency_alarm = bit<u32>(temp_emergency_alarm_bit);
		constexpr inline const u32 temp_fault = bit<u32>(temp_fault_bit);
		constexpr inline const u32 temp_offset = bit<u32>(temp_offset_bit);
		constexpr inline const u32 temp_lowest = bit<u32>(temp_lowest_bit);
		constexpr inline const u32 temp_highest = bit<u32>(temp_highest_bit);
		constexpr inline const u32 temp_reset_history = bit<u32>(temp_reset_history_bit);
		constexpr inline const u32 temp_rated_min = bit<u32>(temp_rated_min_bit);
		constexpr inline const u32 temp_rated_max = bit<u32>(temp_rated_max_bit);

		enum in_attributes
		{
			in_enable_bit = generic_enable_bit,
			in_input_bit = generic_input_bit,
			in_label_bit = generic_label_bit,
			in_min_bit,
			in_max_bit,
			in_lcrit_bit,
			in_crit_bit,
			in_average_bit,
			in_lowest_bit,
			in_highest_bit,
			in_reset_history_bit,
			in_alarm_bit,
			in_min_alarm_bit,
			in_max_alarm_bit,
			in_lcrit_alarm_bit,
			in_crit_alarm_bit,
			in_rated_min_bit,
			in_rated_max_bit,
			in_bit_max
		};

		constexpr inline const u32 in_enable = bit<u32>(in_enable_bit);
		constexpr inline const u32 in_input = bit<u32>(in_input_bit);
		constexpr inline const u32 in_label = bit<u32>(in_label_bit);
		constexpr inline const u32 in_min = bit<u32>(in_min_bit);
		constexpr inline const u32 in_max = bit<u32>(in_max_bit);
		constexpr inline const u32 in_lcrit = bit<u32>(in_lcrit_bit);
		constexpr inline const u32 in_crit = bit<u32>(in_crit_bit);
		constexpr inline const u32 in_average = bit<u32>(in_average_bit);
		constexpr inline const u32 in_lowest = bit<u32>(in_lowest_bit);
		constexpr inline const u32 in_highest = bit<u32>(in_highest_bit);
		constexpr inline const u32 in_reset_history = bit<u32>(in_reset_history_bit);
		constexpr inline const u32 in_alarm = bit<u32>(in_alarm_bit);
		constexpr inline const u32 in_min_alarm = bit<u32>(in_min_alarm_bit);
		constexpr inline const u32 in_max_alarm = bit<u32>(in_max_alarm_bit);
		constexpr inline const u32 in_lcrit_alarm = bit<u32>(in_lcrit_alarm_bit);
		constexpr inline const u32 in_crit_alarm = bit<u32>(in_crit_alarm_bit);
		constexpr inline const u32 in_rated_min = bit<u32>(in_rated_min_bit);
		constexpr inline const u32 in_rated_max = bit<u32>(in_rated_max_bit);

		enum curr_attributes
		{
			curr_enable_bit = generic_enable_bit,
			curr_input_bit = generic_input_bit,
			curr_label_bit = generic_label_bit,
			curr_min_bit,
			curr_max_bit,
			curr_lcrit_bit,
			curr_crit_bit,
			curr_average_bit,
			curr_lowest_bit,
			curr_highest_bit,
			curr_reset_history_bit,
			curr_alarm_bit,
			curr_min_alarm_bit,
			curr_max_alarm_bit,
			curr_lcrit_alarm_bit,
			curr_crit_alarm_bit,
			curr_rated_min_bit,
			curr_rated_max_bit,
			curr_bit_max
		};

		constexpr inline const u32 curr_enable = bit<u32>(curr_enable_bit);
		constexpr inline const u32 curr_input = bit<u32>(curr_input_bit);
		constexpr inline const u32 curr_label = bit<u32>(curr_label_bit);
		constexpr inline const u32 curr_min = bit<u32>(curr_min_bit);
		constexpr inline const u32 curr_max = bit<u32>(curr_max_bit);
		constexpr inline const u32 curr_lcrit = bit<u32>(curr_lcrit_bit);
		constexpr inline const u32 curr_crit = bit<u32>(curr_crit_bit);
		constexpr inline const u32 curr_average = bit<u32>(curr_average_bit);
		constexpr inline const u32 curr_lowest = bit<u32>(curr_lowest_bit);
		constexpr inline const u32 curr_highest = bit<u32>(curr_highest_bit);
		constexpr inline const u32 curr_reset_history = bit<u32>(curr_reset_history_bit);
		constexpr inline const u32 curr_alarm = bit<u32>(curr_alarm_bit);
		constexpr inline const u32 curr_min_alarm = bit<u32>(curr_min_alarm_bit);
		constexpr inline const u32 curr_max_alarm = bit<u32>(curr_max_alarm_bit);
		constexpr inline const u32 curr_lcrit_alarm = bit<u32>(curr_lcrit_alarm_bit);
		constexpr inline const u32 curr_crit_alarm = bit<u32>(curr_crit_alarm_bit);
		constexpr inline const u32 curr_rated_min = bit<u32>(curr_rated_min_bit);
		constexpr inline const u32 curr_rated_max = bit<u32>(curr_rated_max_bit);

		enum power_attributes
		{
			power_enable_bit = generic_enable_bit,
			power_input_bit = generic_input_bit,
			power_label_bit = generic_label_bit,
			power_average_bit,
			power_average_interval_bit,
			power_average_interval_max_bit,
			power_average_interval_min_bit,
			power_average_highest_bit,
			power_average_lowest_bit,
			power_average_max_bit,
			power_average_min_bit,
			power_input_highest_bit,
			power_input_lowest_bit,
			power_reset_history_bit,
			power_accuracy_bit,
			power_cap_bit,
			power_cap_hyst_bit,
			power_cap_max_bit,
			power_cap_min_bit,
			power_min_bit,
			power_max_bit,
			power_crit_bit,
			power_lcrit_bit,
			power_alarm_bit,
			power_cap_alarm_bit,
			power_min_alarm_bit,
			power_max_alarm_bit,
			power_lcrit_alarm_bit,
			power_crit_alarm_bit,
			power_rated_min_bit,
			power_rated_max_bit,
			power_bit_max
		};

		constexpr inline const u32 power_enable = bit<u32>(power_enable_bit);
		constexpr inline const u32 power_input = bit<u32>(power_input_bit);
		constexpr inline const u32 power_label = bit<u32>(power_label_bit);
		constexpr inline const u32 power_average = bit<u32>(power_average_bit);
		constexpr inline const u32 power_average_interval = bit<u32>(power_average_interval_bit);
		constexpr inline const u32 power_average_interval_max = bit<u32>(power_average_interval_max_bit);
		constexpr inline const u32 power_average_interval_min = bit<u32>(power_average_interval_min_bit);
		constexpr inline const u32 power_average_highest = bit<u32>(power_average_highest_bit);
		constexpr inline const u32 power_average_lowest = bit<u32>(power_average_lowest_bit);
		constexpr inline const u32 power_average_max = bit<u32>(power_average_max_bit);
		constexpr inline const u32 power_average_min = bit<u32>(power_average_min_bit);
		constexpr inline const u32 power_input_highest = bit<u32>(power_input_highest_bit);
		constexpr inline const u32 power_input_lowest = bit<u32>(power_input_lowest_bit);
		constexpr inline const u32 power_reset_history = bit<u32>(power_reset_history_bit);
		constexpr inline const u32 power_accuracy = bit<u32>(power_accuracy_bit);
		constexpr inline const u32 power_cap = bit<u32>(power_cap_bit);
		constexpr inline const u32 power_cap_hyst = bit<u32>(power_cap_hyst_bit);
		constexpr inline const u32 power_cap_max = bit<u32>(power_cap_max_bit);
		constexpr inline const u32 power_cap_min = bit<u32>(power_cap_min_bit);
		constexpr inline const u32 power_min = bit<u32>(power_min_bit);
		constexpr inline const u32 power_max = bit<u32>(power_max_bit);
		constexpr inline const u32 power_lcrit = bit<u32>(power_lcrit_bit);
		constexpr inline const u32 power_crit = bit<u32>(power_crit_bit);
		constexpr inline const u32 power_alarm = bit<u32>(power_alarm_bit);
		constexpr inline const u32 power_cap_alarm = bit<u32>(power_cap_alarm_bit);
		constexpr inline const u32 power_min_alarm = bit<u32>(power_min_alarm_bit);
		constexpr inline const u32 power_max_alarm = bit<u32>(power_max_alarm_bit);
		constexpr inline const u32 power_lcrit_alarm = bit<u32>(power_lcrit_alarm_bit);
		constexpr inline const u32 power_crit_alarm = bit<u32>(power_crit_alarm_bit);
		constexpr inline const u32 power_rated_min = bit<u32>(power_rated_min_bit);
		constexpr inline const u32 power_rated_max = bit<u32>(power_rated_max_bit);

		enum energy_attributes
		{
			energy_enable_bit = generic_enable_bit,
			energy_input_bit = generic_input_bit,
			energy_label_bit = generic_label_bit,
			energy_bit_max
		};

		constexpr inline const u32 energy_enable = bit<u32>(energy_enable_bit);
		constexpr inline const u32 energy_input = bit<u32>(energy_input_bit);
		constexpr inline const u32 energy_label = bit<u32>(energy_label_bit);

		enum humidity_attributes
		{
			humidity_enable_bit = generic_enable_bit,
			humidity_input_bit = generic_input_bit,
			humidity_label_bit = generic_label_bit,
			humidity_min_bit,
			humidity_min_hyst_bit,
			humidity_max_bit,
			humidity_max_hyst_bit,
			humidity_alarm_bit,
			humidity_fault_bit,
			humidity_rated_min_bit,
			humidity_rated_max_bit,
			humidity_bit_max
		};

		constexpr inline const u32 humidity_enable = bit<u32>(humidity_enable_bit);
		constexpr inline const u32 humidity_input = bit<u32>(humidity_input_bit);
		constexpr inline const u32 humidity_label = bit<u32>(humidity_label_bit);
		constexpr inline const u32 humidity_min = bit<u32>(humidity_min_bit);
		constexpr inline const u32 humidity_min_hyst = bit<u32>(humidity_min_hyst_bit);
		constexpr inline const u32 humidity_max = bit<u32>(humidity_max_bit);
		constexpr inline const u32 humidity_max_hyst = bit<u32>(humidity_max_hyst_bit);
		constexpr inline const u32 humidity_alarm = bit<u32>(humidity_alarm_bit);
		constexpr inline const u32 humidity_fault = bit<u32>(humidity_fault_bit);
		constexpr inline const u32 humidity_rated_min = bit<u32>(humidity_rated_min_bit);
		constexpr inline const u32 humidity_rated_max = bit<u32>(humidity_rated_max_bit);

		enum fan_attributes
		{
			fan_enable_bit = generic_enable_bit,
			fan_input_bit = generic_input_bit,
			fan_label_bit = generic_label_bit,
			fan_min_bit,
			fan_max_bit,
			fan_div_bit,
			fan_pulses_bit,
			fan_target_bit,
			fan_alarm_bit,
			fan_min_alarm_bit,
			fan_max_alarm_bit,
			fan_fault_bit,
			fan_bit_max
		};

		constexpr inline const u32 fan_enable = bit<u32>(fan_enable_bit);
		constexpr inline const u32 fan_input = bit<u32>(fan_input_bit);
		constexpr inline const u32 fan_label = bit<u32>(fan_label_bit);
		constexpr inline const u32 fan_min = bit<u32>(fan_min_bit);
		constexpr inline const u32 fan_max = bit<u32>(fan_max_bit);
		constexpr inline const u32 fan_div = bit<u32>(fan_div_bit);
		constexpr inline const u32 fan_pulses = bit<u32>(fan_pulses_bit);
		constexpr inline const u32 fan_target = bit<u32>(fan_target_bit);
		constexpr inline const u32 fan_alarm = bit<u32>(fan_alarm_bit);
		constexpr inline const u32 fan_min_alarm = bit<u32>(fan_min_alarm_bit);
		constexpr inline const u32 fan_max_alarm = bit<u32>(fan_max_alarm_bit);
		constexpr inline const u32 fan_fault = bit<u32>(fan_fault_bit);

		enum pwm_attributes
		{
			pwm_enable_bit = generic_enable_bit,
			pwm_input_bit = generic_input_bit,
			pwm_label_bit = generic_label_bit, // new
			pwm_mode_bit,
			pwm_freq_bit,
			pwm_bit_max
		};

		constexpr inline const u32 pwm_input = bit<u32>(pwm_input_bit);
		constexpr inline const u32 pwm_enable = bit<u32>(pwm_enable_bit);
		constexpr inline const u32 pwm_label = bit<u32>(pwm_label_bit);
		constexpr inline const u32 pwm_mode = bit<u32>(pwm_mode_bit);
		constexpr inline const u32 pwm_freq = bit<u32>(pwm_freq_bit);

		enum intrusion_attributes
		{
			intrusion_alarm_bit,
			intrusion_beep_bit,
			intrusion_bit_max
		};

		constexpr inline const u32 intrusion_alarm = bit<u32>(intrusion_alarm_bit);
		constexpr inline const u32 intrusion_beep = bit<u32>(intrusion_beep_bit);

		enum duration_attributes
		{
			duration_enable_bit = generic_enable_bit,
			duration_input_bit = generic_input_bit,
			duration_label_bit = generic_label_bit,
			duration_bit_max
		};

		constexpr inline const u32 duration_enable = bit<u32>(duration_enable_bit);
		constexpr inline const u32 duration_input = bit<u32>(duration_input_bit);
		constexpr inline const u32 duration_label = bit<u32>(duration_label_bit);

		enum frequency_attributes
		{
			frequency_enable_bit = generic_enable_bit,
			frequency_input_bit = generic_input_bit,
			frequency_label_bit = generic_label_bit,
			frequency_bit_max
		};

		constexpr inline const u32 frequency_enable = bit<u32>(frequency_enable_bit);
		constexpr inline const u32 frequency_input = bit<u32>(frequency_input_bit);
		constexpr inline const u32 frequency_label = bit<u32>(frequency_label_bit);

		enum load_attributes
		{
			load_enable_bit = generic_enable_bit,
			load_input_bit = generic_input_bit,
			load_label_bit = generic_label_bit,
			load_bit_max
		};

		constexpr inline const u32 load_enable = bit<u32>(load_enable_bit);
		constexpr inline const u32 load_input = bit<u32>(load_input_bit);
		constexpr inline const u32 load_label = bit<u32>(load_label_bit);

		enum raw_attributes
		{
			raw_enable_bit = generic_enable_bit,
			raw_input_bit = generic_input_bit,
			raw_label_bit = generic_label_bit,
			raw_bit_max
		};

		constexpr inline const u32 raw_enable = bit<u32>(raw_enable_bit);
		constexpr inline const u32 raw_input = bit<u32>(raw_input_bit);
		constexpr inline const u32 raw_label = bit<u32>(raw_label_bit);
	} // namespace attributes
} // namespace wm_sensors

#endif
