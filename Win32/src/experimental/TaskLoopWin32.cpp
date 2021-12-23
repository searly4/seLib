#include <seLib/experimental/TaskLoop.h>
#include <map>
//#include <mutex>
#include <thread>

#include <seLib/experimental/TaskLoopWin32.h>


namespace seLib {

list<TaskHandler*> TaskHandler::_taskHandlers;

TaskHandler::Task& TaskHandler::GetMyTask() {
	auto iter = _taskHandlers.begin();
	while (iter != _taskHandlers.end()) {
		Task* task = (*iter)->_GetMyTask();
		if (task != nullptr)
			return *task;
	}
	throw exception("Task not found.");
}


Win32Task::Win32Task(TaskHandlerWin32& handler) :
	Task((TaskHandler&)handler)
{ }

void Win32Task::Join() {
	if (Threadable) {
		if (_taskThread == nullptr) {
		  //lock_guard<recursive_mutex> lg(_lock);
			_taskThread = new thread(&Win32Task::_threadLoop, this);
		} else {
			Wake();
		}
	} else {
		_execPass();
	}
}

/*
void Win32Task::Stop(bool wait) {
  Enabled = false;
}

void Win32Task::Wake() {
  TaskHandler::Task::Wake();
  _wait.notify_all();
}//*/

void Win32Task::_threadLoop() {
	while (Enabled) {
		if (_sleep) {
		  //_wait.wait(std::unique_lock<std::mutex>(_waitLock));
		}
		if (Enabled) {
			_execPass();
			if (OneShot)
				_sleep = true;
		}
	}
	//lock_guard<recursive_mutex> lg(_lock);
	delete _taskThread;
	_taskThread = nullptr;
}

void Win32Task::_execPass() {
	((TaskHandlerWin32&)Handler)._execTask(*this);
	//TaskHandler::Task::Join();
}//*/

bool Win32Task::Invoke(std::function<void()> func) {
  //lock_guard<recursive_mutex> lg(_lock);
	_invokeList.push_back(func);
	return true;
}

bool Win32Task::AddCall(std::function<void()> func) {
  //lock_guard<recursive_mutex> lg(_lock);
	for (auto iter = _callList.begin(); iter != _callList.end(); iter++) {
		if (iter->target<void*>() == func.target<void*>())
			return false;
	}
	_callList.push_back(func);
	return true;
}

bool Win32Task::RemoveCall(std::function<void()> func) {
  //lock_guard<recursive_mutex> lg(_lock);
	for (auto iter = _callList.begin(); iter != _callList.end(); iter++) {
		if (iter->target<void*>() == func.target<void*>()) {
			_callList.erase(iter);
			return true;
		}
	}
	return false;
}

bool Win32Task::PostMessage(TaskHandler::TaskMessage&& message) {
  //lock_guard<recursive_mutex> lg(_lock);
	if (TaskMessageReceiver != nullptr) {
		MessageReceiveQueue.push_back(std::move(message));
		Wake();
		return true;
	}
	return false;
}

TaskHandlerWin32::TaskHandlerWin32() { }
TaskHandlerWin32::~TaskHandlerWin32() { }

void TaskHandlerWin32::Start() {
	_active = true;
	auto iter = _taskList.begin();
	while (_active) {
		auto cacheiter = iter;
		iter++;
		if (iter == _taskList.end())
			iter = _taskList.begin();

		Win32Task& task = *cacheiter;

		if (task.Delete)
			_taskList.erase(cacheiter);
		else if (task.Enabled && !task.Active)
			task.Join();

	}
}
void TaskHandlerWin32::Stop(bool wait) {
	_active = false;
}

Win32Task& TaskHandlerWin32::NewTask(Task* parent) {
	lock_guard<mutex> lg(_listMutex);
	_taskList.emplace_back(*this);
	Win32Task& task = _taskList.back();
	task.Parent = parent;
	return task;
}

TaskHandler::Task* TaskHandlerWin32::_GetMyTask() {
	thread::id tid = this_thread::get_id();
	auto iter = _threads.find(tid);
	if (iter == _threads.end())
		return nullptr;
	return iter->second;
}

void TaskHandlerWin32::_wakeTask(Task& task) {
	TaskHandler::_wakeTask(task);
	auto& t = (Win32Task&)task;
	if (t.WakeHandler != nullptr)
		t.WakeHandler();
	else
		t._wait.notify_all();
}

void TaskHandlerWin32::_execTask(Task& task) {
	auto& w32task = (Win32Task&)task;

	vector<std::function<void()>> callList;
	vector<TaskMessage> messages;

	{
	  //lock_guard<recursive_mutex> lg(w32task._lock);
	  //msclr::lock l(this);

		callList.reserve(w32task._invokeList.size() + w32task._callList.size());
		callList.insert(callList.end(), w32task._invokeList.begin(), w32task._invokeList.end());
		w32task._invokeList.clear();
		callList.insert(callList.end(), w32task._callList.begin(), w32task._callList.end());

		messages.reserve(w32task.MessageReceiveQueue.size());
		for (auto iter = w32task.MessageReceiveQueue.begin(); iter != w32task.MessageReceiveQueue.end(); iter++)
			messages.push_back(std::move(*iter));

		w32task.MessageReceiveQueue.clear();
	}

	for (auto iter = callList.begin(); iter != callList.end(); iter++) {
		(*iter)();
	}

	if (w32task.TaskMessageReceiver != nullptr)
		for (auto iter = messages.begin(); iter != messages.end(); iter++)
			w32task.TaskMessageReceiver(std::move(*iter));

	if (w32task._callList.empty() && w32task.MessageReceiveQueue.empty())
		w32task.Sleep();
}

}
