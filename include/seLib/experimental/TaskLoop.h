#pragma once
#include <stdint.h>
#include <list>
#include <string>
#include <vector>
#include <map>
//#include <mutex>
#include <functional>
#include <queue>
#include <deque>
#include <memory>

#include "Debug.h"
#include "PlatformCore.h"
#include "../RefObj.h"

#undef PostMessage // fix Windows' usurping of this name

#define def_TaskLoophMessages \
_X_(Empty) \
_X_(Exception) \
_X_(Debug) \
_X_(Ping) \
_X_(PingReply) \
_X_(TaskLoopCreated) \
_X_(TaskLoopDestroyed) \
_X_(SubscribeMessage) \
_X_(UnsubscribeMessage) \
_X_(LastTaskLoopMessage)

namespace seLib {

__CLI_PUBLIC enum class TaskLoopMessages {
#define _X_(name) name,
	def_TaskLoophMessages
#undef _X_
};

inline uint32_t operator*(TaskLoopMessages enumval) {
	return (uint32_t)enumval;
}

class TaskLoopHandler;

class TaskLoop {
	friend class TaskLoopHandler;

public:
  // Message packet containing message ID and associated memory block reference.
	class Message {
	public:
		using DataRef_t = RefBufferView;
		uint32_t TypeID;
		DataRef_t Data;
		int DataSize = 0;

	public:
		inline Message(uint32_t typeID) :
			TypeID(typeID) { }

		  /*inline Message(uint32_t typeID, const uint8_t* data, int datasize, bool dontDestroy = false) :
			TypeID(typeID),
			Data((data == nullptr && datasize > 0) ? new uint8_t[datasize] : data, !dontDestroy),
			DataSize(datasize)
		  {
			if (data != nullptr && datasize <= 0)
			  throw exception();
			  //SETrace("ReadTaxDump/1", SE::Debug::Trace::Warn);
		  }*/

		inline Message(uint32_t typeID, DataRef_t data) :
			TypeID(typeID),
			Data(data),
			DataSize(data.size())
		{ }

		inline Message(uint32_t typeID, const uint8_t* copysource, int len) :
			TypeID(typeID),
			Data(len),
			DataSize(len)
		{
			Data.set(copysource, len, 0);
		}

		inline Message(const Message& copysrc) :
			TypeID(copysrc.TypeID),
			Data(&copysrc.Data),
			DataSize(copysrc.DataSize)
		{ }
		/*inline Message(Message&& movesrc) :
		  TypeID(movesrc.TypeID),
		  Data(std::move(movesrc.Data)), // todo: no move?
		  DataSize(movesrc.DataSize)
		{
		  movesrc.TypeID = 0;
		  movesrc.Data = RefObj<uint8_t*>();
		  movesrc.DataSize = 0;
		}*/

		inline ~Message() {
		}

		DataRef_t GetRef() {
			return Data;
		}

		template <typename T>
		T* Get() {
			T* ptr = (T*)*Data;
			return ptr;
		}

		/*inline const uint8_t* release() {
		  const uint8_t* buffer = Data.release();
		  DataSize = 0;
		  return buffer;
		}*/

		inline DataRef_t data() {
			return Data;
		}
	};

	// Message packet containing message ID and associated memory block.
	//typedef RefObj<MessageContent> Message;

	using MessageReceiver = std::function<bool(Message&)>;
	using Job = std::function<bool(void)>;
	using Event = std::function<void(void)>;

	// Abstract class for implementing services managed by a TaskLoop.
	class Service {
	public:
		TaskLoop& _task;

	protected:
		TaskLoop::MessageReceiver _messageReceiver = TaskLoop::MessageReceiver([&](TaskLoop::Message& msg) { return this->MessageReceiver(msg); }); // todo: valid init?
		bool _workPending = false;

	public:
		inline Service(TaskLoop& task) :
			_task(task)
		{
			_messageReceiver = TaskLoop::MessageReceiver([&](TaskLoop::Message& msg) { return this->MessageReceiver(msg); }); // todo: redundant?
			_task.AddMessageReceiver(_messageReceiver);
		}

		inline virtual ~Service() {
			_task.RemoveMessageReceiver(_messageReceiver);
		}

		inline bool MessageReceiver(TaskLoop::Message& message) {
			return _MessageReceiver(message);
		}

		inline void PostMessage(uint32_t typeID, Message::DataRef_t data) {
			_task.PostMessage(Message(typeID, data));
		}

		inline void PostMessage(uint32_t typeID, const uint8_t* copysource, int len) const {
			TaskLoop::Message::DataRef_t buffer(len);
			buffer.set(copysource, len, 0);
			_task.PostMessage(Message(typeID, buffer));
		}

		inline void PostMessage(uint32_t typeID) const {
			_task.PostMessage(typeID);
		}

		inline void PostMessage(Message& copysrc) {
			_task.PostMessage(copysrc);
		}

		inline void ParentPostMessage(uint32_t typeID, Message::DataRef_t data) const {
			_task.ParentPostMessage(Message(typeID, data));
		}

		inline void ParentPostMessage(uint32_t typeID, const uint8_t* copysource, int len) const {
			TaskLoop::Message::DataRef_t buffer(len);
			buffer.set(copysource, len, 0);
			_task.ParentPostMessage(Message(typeID, buffer));
		}

		inline void ParentPostMessage(Message& copysrc) const {
			_task.ParentPostMessage(copysrc);
		}

		inline void ParentPostMessage(uint32_t typeID) const {
			_task.ParentPostMessage(typeID);
		}

		inline virtual bool WorkPending() {
			return _workPending;
		}

	protected:
		virtual bool _MessageReceiver(TaskLoop::Message& message) = 0;
	};

public:
	TaskLoopHandler& Handler;
	TaskLoop* Parent = nullptr;
	std::string Name;
	bool Enabled = false;
	bool OneShot = false;
	bool Active = false;
	bool Threadable = false;
	bool Delete = false;

	std::vector<MessageReceiver*> TaskMessageReceiver;
	TaskLoop::MessageReceiver ParentReceiver;

protected:
	LockObj _Mutex;
	std::deque<Message> _MessageReceiveQueue;
	std::vector<TaskLoop*> _taskList;
	std::vector<Job> _callList;
	std::list<Event> _invokeList;

public:
	TaskLoop(TaskLoopHandler& handler) : Handler(handler) {}

	bool Start();

	bool Stop(bool wait);

	inline bool Ready() const {
		return !Active && Enabled;
	}

	inline bool Running() const {
		return Active;
	}

	// Call each function in this task's call list;

	bool Invoke(Event func) {
		auto lg = seLib::LockGuard(_Mutex);
		_invokeList.push_back(func);
		return true;
	}

	bool AddCall(Job func) {
	  //lock_guard<recursive_mutex> lg(_lock);
		auto lg = seLib::LockGuard(_Mutex);
		for (auto iter = _callList.begin(); iter != _callList.end(); iter++) {
			if (iter->target<void*>() == func.target<void*>())
				return false;
		}
		_callList.push_back(func);
		return true;
	}

	bool RemoveCall(Job func) {
	  //lock_guard<recursive_mutex> lg(_lock);
		auto lg = seLib::LockGuard(_Mutex);
		for (auto iter = _callList.begin(); iter != _callList.end(); iter++) {
			if (iter->target<void*>() == func.target<void*>()) {
				_callList.erase(iter);
				return true;
			}
		}
		return false;
	}

	TaskLoop& AddTask() {
		return AddTask(Handler);
	}

	TaskLoop& AddTask(TaskLoopHandler& handler) {
	  //lock_guard<recursive_mutex> lg(_lock);
		auto lg = seLib::LockGuard(_Mutex);
		auto task = new TaskLoop(handler);
		_taskList.push_back(task);
		task->Parent = this;
		return *task;
	}

	bool RemoveTask(TaskLoop& task) {
	  //lock_guard<recursive_mutex> lg(_lock);
		auto lg = seLib::LockGuard(_Mutex);
		for (auto iter = _taskList.begin(); iter != _taskList.end(); iter++) {
			if (*iter == &task) {
				_taskList.erase(iter);
				delete& task;
				return true;
			}
		}
		return false;

	}

	void AddMessageReceiver(MessageReceiver& receiver) {
		auto lg = seLib::LockGuard(_Mutex);
		TaskMessageReceiver.push_back(&receiver);
	}

	void RemoveMessageReceiver(MessageReceiver& receiver) {
		auto lg = seLib::LockGuard(_Mutex);
		TaskMessageReceiver.erase(std::find(TaskMessageReceiver.begin(), TaskMessageReceiver.end(), &receiver));
	}

	bool PostMessage(const Message& message) {
	  //lock_guard<recursive_mutex> lg(_lock);
		auto lg = seLib::LockGuard(_Mutex);
		if (TaskMessageReceiver.size() > 0) {
		  //_MessageReceiveQueue.push_back(std::move(message));
			_MessageReceiveQueue.push_back(message);
			//SETrace(string("Message count: ") + to_string(_MessageReceiveQueue.size()), SE::Debug::Trace::Debug);
			//Wake();
			return true;
		}
		return false;
	}

	inline void ParentPostMessage(uint32_t typeID, Message::DataRef_t data) const {
		auto message = Message(typeID, data);

		if (Parent != nullptr)
			Parent->PostMessage(message);

		if (ParentReceiver != nullptr)
			ParentReceiver(message);
	}

	inline void ParentPostMessage(Message copysrc) const {
		if (Parent != nullptr)
			Parent->PostMessage(copysrc);

		if (ParentReceiver != nullptr)
			ParentReceiver(copysrc);
	}

	static TaskLoop& GetMyTask();

protected:
  //virtual TaskLoop* _GetMyTask() = 0;

};

class TaskLoopHandler {
public:
protected:
public:
  //TaskLoopHandler();
	virtual bool Start(TaskLoop& taskloop) = 0;
	virtual bool Stop(TaskLoop& taskloop, bool wait) = 0;
protected:
	inline const std::vector<TaskLoop*>& _gettaskList(const TaskLoop& taskloop) const {
		return taskloop._taskList;
	}
	inline const std::vector<TaskLoop::Job>& _getcallList(const TaskLoop& taskloop) const {
		return taskloop._callList;
	}
	inline std::list<TaskLoop::Event>& _getinvokeList(TaskLoop& taskloop) const {
		return taskloop._invokeList;
	}
	std::deque<TaskLoop::Message>& _getMessageReceiveQueue(TaskLoop& taskloop) const {
		return taskloop._MessageReceiveQueue;
	}

};

class TaskLoopPollingHandler : public TaskLoopHandler {
public:
protected:
  //TaskLoop& _task;

public:
  //TaskLoopPollingHandler(TaskLoop& taskloop) : _task(taskloop) {}
	TaskLoopPollingHandler() {}

	bool Start(TaskLoop& taskloop) override {
		return _execPass(taskloop);
	}

	bool Stop(TaskLoop& taskloop, bool wait) override {
		taskloop.Enabled = false;
		auto& _taskList = _gettaskList(taskloop);
		for (auto iter = _taskList.begin(); iter != _taskList.end(); iter++) {
			auto& tl = (**iter);
			if (tl.Ready())
				tl.Stop(wait);
		}
		return true; // todo
	}

protected:
	bool _execPass(TaskLoop& taskloop) {
		bool workpending = false;

		//const auto& _MessageReceiveQueue = _getMessageReceiveQueue(taskloop);

		std::list<TaskLoop::Event> invokeList;
		invokeList.swap(_getinvokeList(taskloop));
		//invokeList.swap(_invokeList);

		std::vector<TaskLoop::Job> callList(_getcallList(taskloop));
		//callList.reserve(_callList.size());
		//callList.insert(callList.end(), _callList.begin(), _callList.end());

		std::vector<TaskLoop*> taskList(_gettaskList(taskloop));
		//taskList.reserve(_taskList.size());
		//taskList.insert(_taskList.end(), _taskList.begin(), _taskList.end());

		std::deque<TaskLoop::Message> messages;
		messages.swap(_getMessageReceiveQueue(taskloop));
		//for (auto iter = _MessageReceiveQueue.begin(); iter != _MessageReceiveQueue.end(); iter++)
		  //messages.push_back(std::move(*iter));

		//string debugmsg = string("TaskLoopPollingHandler::_execPass, ") + to_string(taskloop.TaskMessageReceiver.size()) + ", " + to_string(messages.size()) + ", " + to_string(taskList.size()) + ", " + to_string(callList.size());
		//SETrace(debugmsg, SE::Debug::Trace::Verbose);

		for (auto iter = callList.begin(); iter != callList.end(); iter++) {
			workpending |= (*iter)();
		}
		for (auto iter = invokeList.begin(); iter != invokeList.end(); iter++) {
			(*iter)();
		}

		for (auto iter = taskList.begin(); iter != taskList.end(); iter++) {
			auto& tl = (**iter);
			if (tl.Ready())
				workpending |= tl.Start();
		}

		for (auto iter = messages.begin(); iter != messages.end(); iter++) {
			for (auto iter2 = taskloop.TaskMessageReceiver.begin(); iter2 != taskloop.TaskMessageReceiver.end(); iter2++)
				workpending |= (**iter2)(*iter);
		}

		workpending |= !_getinvokeList(taskloop).empty();
		return workpending;
	}

};

inline bool TaskLoop::Start() {
  //SETrace("TaskLoop::Start", SE::Debug::Trace::Verbose);
	return Handler.Start(*this);
}

inline bool TaskLoop::Stop(bool wait) {
	return Handler.Stop(*this, wait);
}

}