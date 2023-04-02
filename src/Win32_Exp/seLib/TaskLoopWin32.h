#pragma once
#include <map>
//#include <mutex>
#include <thread>
//#include <condition_variable>
//#include <vcclr.h>
//#include <msclr/lock.h>

#include <seLib/experimental/TaskLoop.h>

namespace seLib {

class TaskHandlerWin32;

class Win32Task : public TaskHandler::Task {
	friend class TaskHandlerWin32;
public:
	TaskHandler::Event WakeHandler = nullptr;

protected:
	std::thread* _taskThread = nullptr;
	condition_variable _wait;
	System::Threading::Monitor _lock;
	//recursive_mutex _lock;
	//mutex _waitLock;
public:
	Win32Task(TaskHandlerWin32& handler);

	void Join() override;

	bool Invoke(std::function<void()> func) override;

	bool AddCall(std::function<void()> func) override;

	bool RemoveCall(std::function<void()> func) override;

	bool PostMessage(TaskHandler::TaskMessage&& message) override;

protected:

	void _threadLoop();
	void _execPass();

};

class TaskHandlerWin32 : public TaskHandler {
	friend class Win32Task;
protected:
	list<Win32Task> _taskList;
	map<thread::id, Win32Task*> _threads;
	mutex _listMutex;
	condition_variable _wait;
	mutex _waitLock;

public:
	TaskHandlerWin32();
	~TaskHandlerWin32();

	void Start() override;
	void Stop(bool wait = true) override;

	Win32Task& NewTask(Task* parent) override;

protected:
	Task* _GetMyTask() override;
	void _wakeTask(Task& task) override;
	void _execTask(Task& task) override;
};

}
