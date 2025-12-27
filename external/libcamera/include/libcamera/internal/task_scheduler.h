/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * task_scheduler.h - A task scheduler
 */

#pragma once

#include <chrono>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <libcamera/base/object.h>
#include <libcamera/base/timer.h>

namespace libcamera {

class Scheduler;

class Task
{
public:
	Task(Scheduler *scheduler, const std::string &id = "");
	virtual ~Task() = default;

	virtual void launch();
	virtual void notifyDone();

	virtual void run() = 0;
	std::string &id() { return id_; }

	bool isRunning() { return running_; }

protected:
	Scheduler *scheduler_;
	std::string id_;

private:
	friend Scheduler;

	bool running_ = false;

	void depend(Task *task);
	size_t removeDependency(Task *task);

	std::list<Task *> precedents_;
	std::list<Task *> succedents_;

	std::chrono::steady_clock::time_point launchTime_;
};

class DelayedTask : public Task
{
public:
	DelayedTask(std::chrono::milliseconds duration,
		    Scheduler *scheduler,
		    const std::string &id = "");

	virtual void run() override final;

private:
	std::unique_ptr<Timer> timer_;
	std::chrono::milliseconds duration_;
};

class Scheduler : public Object
{
public:
	static void precede(Task *precedent, Task *task);

	Scheduler();

	void schedule();
	void log();

	// Debug function.
	bool hasCyclicDependency() const;

protected:
	void queueTask(Task *task, int32_t group);
	void succeedPrevTaskByStep(int32_t group, size_t step, Task *task);
	std::list<Task *> &groupTasks(int32_t group);

	std::map<int32_t, std::string> groupNames_;

private:
	friend Task;

	void removeFromGroupTasks(Task *task);

	void taskDone(Task *task);
	Signal<Task *> taskDone_;

	std::unordered_map<Task *, std::unique_ptr<Task>> tasksHolder_;

	std::map<int32_t, std::list<Task *>> groupTasks_;
	std::unordered_set<Task *> pendingTasks_;
	std::unordered_set<Task *> runningTasks_;
};

template<typename Category, std::enable_if_t<std::is_enum_v<Category>> * = nullptr>
class CategorizedScheduler : public Scheduler
{
	static_assert(std::is_enum<Category>::value, "Category should be an enum");

public:
	CategorizedScheduler(const std::map<Category, std::string> &categoryName)
	{
		for (auto &[group, name] : categoryName)
			Scheduler::groupNames_[(int32_t)group] = name;
	}

	void queueTask(Task *task, Category group)
	{
		Scheduler::queueTask(task, static_cast<int32_t>(group));
	}

	std::list<Task *> &groupTasks(Category group)
	{
		return Scheduler::groupTasks(static_cast<int32_t>(group));
	}

	void succeedPrevTaskByStep(Category group, size_t step, Task *task)
	{
		Scheduler::succeedPrevTaskByStep(static_cast<uint32_t>(group), step, task);
	}
};

} /* namespace libcamera */
