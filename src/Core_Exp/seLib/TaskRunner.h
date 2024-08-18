#pragma once
///-------------------------------------------------------------------------------------------------
/// @file	xPort\Util\ModuleManager.h
///
/// @brief	Declares the base classes for Modules and the ModuleManager class.

//#include "ProjectConfig.h"
/// \cond

#include <stdint.h>
#include <stdlib.h>
//#include <concepts>
/// \endcond

//#include "Messages.h"
//#include "Faults.h"
//#include "LogQueue.h"
//#include <xPort/xPort_Timing.h>
#include <seLib/Time.h>
#include <seLib/Timing.h>
//#include <xPort/xPort_HAL_Util.h>
#include <seLib/Filters.h>
#include <seLib/Buffers.h>
//#include <xPort/Util/Signals.h>
//#include <xPort/Util/Messages.h>
#include <seLib/Iterators.h>

namespace seLib {


namespace TaskRunner {

//===========================================================================
// Type definitions

enum class Status_e {
	Success,
	Blocked,
	Error,
	CriticalError,
	ErrorNotRunning,
	DeferredStart,
	TasksPending,
	NotScheduled,
	Invalid,
};

constexpr TimeValue32kHz_t<int32_t> Interval1Second{ TimeValue32kHz_t<int32_t>::FromRawValue(1 << TimeValue32kHz_t<int32_t>::Radix) };

template <typename Timestamp_T, size_t ProviderCount_N>
class TaskManager_t;

template <typename Timestamp_T>
class TaskInstance_t;

template <typename Timestamp_T>
class TaskInstance_t;


template <typename Timestamp_T>
struct TaskState_t {
	/// @brief	Number of executions per second.
	seLib::Timing::RateCounter_t<typename Timestamp_T::Storage_t> Rate = { Interval1Second.Value };

	/// @brief	Longest interval between executions.
	seLib::Timing::PeakInterval_t<typename Timestamp_T::Storage_t> PeakInterval = { Interval1Second.Value };

	Timestamp_T LastTimestamp { 0 };
	float ExecTime { 0 }; /// Average execution time.
	float ExecTime2 { 0 }; /// Last execution time.
	uint8_t ReadyPriority { 0 };
};


template <typename Timestamp_T>
struct TaskDefBase_t {
	Timestamp_T::Storage_t Mask { 0 };
	uint8_t Priority { 0 };

	constexpr TaskDefBase_t(Timestamp_T::Storage_t mask) :
		Mask(mask)
	{}

	constexpr TaskDefBase_t(Timestamp_T::Storage_t mask, uint8_t priority) :
		Mask(seLib::FillBitsHigh(mask)), Priority(priority)
	{
		if (priority > 7)
			throw std::exception();
	}

	constexpr TaskDefBase_t(TaskDefBase_t const &) = default;
};


template <typename TaskInstance_T, typename Timestamp_T>
requires std::derived_from< TaskInstance_T, TaskInstance_t< Timestamp_T>>
struct TaskDef_t : public TaskDefBase_t<Timestamp_T> {
	using TimeInterval_t = typename Timestamp_T::TimeInterval_t;
	using TaskInstance_t = TaskInstance_T;
	//typedef Status_e (TaskInstance_t::*TaskFuncPtr_t)(Timestamp_T, TimeInterval_t) const;
	//TaskFuncPtr_t Handler;

	constexpr TaskDef_t(
		//TaskFuncPtr_t handler,
		Timestamp_T::Storage_t mask,
		uint8_t priority
	) :
		TaskDefBase_t<Timestamp_T>(mask, priority)//, Handler(handler)
	{}

	constexpr TaskDef_t(TaskDef_t const &) = default;

};



///-------------------------------------------------------------------------------------------------
/// @class	TaskInstance_t
/// @brief	
template <typename Timestamp_T>
class TaskInstance_t {
public:
	using TimeInterval_t = typename Timestamp_T::TimeInterval_t;
	using State_t = TaskState_t<Timestamp_T>;

	constexpr TaskInstance_t() = default;
	constexpr TaskInstance_t(TaskInstance_t const &) = default;
	TaskInstance_t& operator=(TaskInstance_t const&) = default;

	virtual State_t& GetState() const noexcept = 0;
	virtual TaskDef_t<TaskInstance_t<Timestamp_T>, Timestamp_T> const & GetDef() const noexcept = 0;
	virtual bool IsEnabled() const noexcept { return true; }
	virtual Status_e ExecTask(Timestamp_T timestamp) const override = 0;

	//Status_e Run(TaskInstance_t const& module, TaskState_t<Timestamp_T>& state, Timestamp_T timestamp) const {
	//	state.PeakInterval.Add(timestamp.Value);
	//	state.Rate.Add(timestamp.Value);
	//
	//	seLib::Timing::TimeExecution_t<uint16_t> timer1(xPort::HAL::Fast32kHzClock, nullptr);
	//
	//	auto retval = ((&module)->*Handler)(timestamp, timestamp - state.LastTimestamp);
	//
	//	state.LastTimestamp = timestamp;
	//	auto elapsed = timer1.Elapsed();
	//	state.ExecTime = seLib::Filters::Filter_LP_t<float>{ 0.01f }(elapsed, state.ExecTime);
	//	state.ExecTime2 = seLib::Filters::Filter_LP_t<float>{ 1.0f }(elapsed, state.ExecTime2);
	//
	//	return retval;
	//}
};



template <typename Timestamp_T>
class TaskManagerState_t {
public:
	size_t LastTickIntModule { 0 };
};

template <typename Timestamp_T>
class TaskList_t :
	public seLib::Iterators::IndexIterator_t<TaskList_t<Timestamp_T>, TaskInstance_t<Timestamp_T>>
{
public:
	//virtual TaskInstance_t<Timestamp_T> const;
};


///-------------------------------------------------------------------------------------------------
/// @class	TaskManager_t
/// @brief	Manages a collection of Module_t objects.
template <typename Timestamp_T, size_t ProviderCount_N>
class TaskManager_t {
public:
	using TimeInterval_t = Timestamp_T::TimeInterval_t;
	using TaskProvider_t = TaskList_t<Timestamp_T>;

	///-------------------------------------------------------------------------------------------------
	/// @property	TaskInstance_t const * const * ProviderList
	/// @brief	Gets a list of modules
	/// @returns	A list of modules.
	TaskManagerState_t<Timestamp_T>& State;
	std::array<TaskProvider_t const *, ProviderCount_N> ProviderList;

	constexpr TaskManager_t(TaskManagerState_t<Timestamp_T>& state, std::array<TaskProvider_t const*, ProviderCount_N> const & providers)
		: State(state),
		ProviderList(providers)
	{}

	virtual Timestamp_T GetTime() const noexcept = 0;


	//TaskInstance_t::Status_e UpdateTasks(Timestamp_t timestamp) const;

	inline Status_e ExecTasks(
		Timestamp_T timestamp,
		TimeInterval_t time_limit,
		uint8_t priority_mask
	) const
	{
		// TODO: (Critical) Review for correctness.

		using mask_t = decltype(priority_mask);
		const auto mask_bit_f = seLib::MaskBitsHigh<mask_t>;
		constexpr size_t priority_mask_bits = sizeof(priority_mask) * 8;

		auto start_time = GetTime(); // only works up to 2 seconds
		bool highest_priority = true; // don't time out highest priority in priority_mask

		for (size_t priority_i = 1; priority_i <= priority_mask_bits; priority_i++) {
			if (!(mask_bit_f(priority_i - 1) & priority_mask))
				continue; // skip priorities not in priority_mask

			for (TaskProvider_t const* provider : ProviderList) {
				auto mark_time = GetTime();
				if (!highest_priority && (mark_time - start_time) > time_limit)
					return Status_e::TasksPending; // time limit exceeded

				// Iterate module's tasks and run those that are due.
				for (TaskInstance_t<Timestamp_T>* task_instance : provider) {
					auto& task_status = task_instance->GetState();
					auto* task_def = &task_instance->GetDef();

					if (task_def == nullptr || (!task_status->ReadyPriority && !task_def->Mask))
						continue;

					if (task_def->Mask && !(priority_mask & mask_bit_f(task_def->Priority)))
						continue; // scheduled task not in this priority group

					timestamp = GetTime();

					// Skip priority analysis if it already matches the current priority.
					if (task_status->ReadyPriority != priority_i) {
						if (!task_def->Mask)
							continue; // N/A: not a scheduled task

						auto time_inc = ~task_def->Mask + 1;

						Timestamp_T time_due =
							{ (task_status->LastTimestamp.Value & task_def->Mask) + time_inc };

						auto time_diff = (timestamp - time_due).Value;

						if (time_diff < 0)
							continue; // not due yet

						if (!task_status->ReadyPriority)
							task_status->ReadyPriority = task_def->Priority + 1; // default priority

						// Increase task priority if it's significantly overdue. Don't exceed
						// the priorities allowed by the current priority_mask.
						if (time_diff & task_def->Mask && task_status->ReadyPriority != priority_i) {
							auto pri = task_def->Priority + 1u; // start from default priority
							mask_t pri_mask = mask_bit_f(pri - 1);
							while (time_diff & task_def->Mask && pri != priority_i && pri > 0) {
								do { // search for next valid priority in priority_mask
									pri--;
									pri_mask >>= 1;
								} while (pri > 0 && !(pri_mask & priority_mask));

								time_diff -= time_inc;
								if (pri_mask & priority_mask && task_status->ReadyPriority > pri)
									task_status->ReadyPriority = pri;
							}
						}

						if (task_status->ReadyPriority != priority_i)
							continue;
					}

					// Run the task.
					task_instance->ExecTask(timestamp);
					task_status->ReadyPriority = 0;
				}
			}

			highest_priority = false;
		}

		return Status_e::Success;
	}
};


}
}