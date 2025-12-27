/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * task_scheduler.cpp - A task scheduler
 */

#include "libcamera/internal/task_scheduler.h"

#include <libcamera/base/log.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(Task)

Task::Task(Scheduler *scheduler, const std::string &id)
	: scheduler_(scheduler), id_(id)
{
}

size_t Task::removeDependency(Task *task)
{
	precedents_.remove(task);
	return precedents_.size();
}

void Task::depend(Task *task)
{
	precedents_.emplace_back(task);
	task->succedents_.emplace_back(this);
}

void Task::launch()
{
	running_ = true;
	launchTime_ = std::chrono::steady_clock::now();

	auto *method = new BoundMethodMember{
		this, scheduler_, &Task::run, ConnectionTypeQueued
	};

	method->activate();
}

void Task::notifyDone()
{
	scheduler_->invokeMethod(&Scheduler::taskDone, ConnectionTypeQueued, this);
}

DelayedTask::DelayedTask(std::chrono::milliseconds duration,
			 Scheduler *scheduler, const std::string &id)
	: Task(scheduler, id), duration_(duration)
{
	timer_ = std::make_unique<Timer>();
	timer_->timeout.connect(static_cast<Task *>(this), &Task::notifyDone);
}

void DelayedTask::run()
{
	timer_->start(duration_);
}

Scheduler::Scheduler()
{
}

void Scheduler::precede(Task *precedent, Task *task)
{
	ASSERT(task && precedent);
	task->depend(precedent);
}

void Scheduler::succeedPrevTaskByStep(int32_t group, size_t step, Task *task)
{
	ASSERT(task);

	auto &tasks = groupTasks_[group];
	if (tasks.size() <= step)
		return;

	auto iter = tasks.rbegin();
	for (size_t i = 0; i < step; i++)
		iter++;

	precede(*iter, task);
}

void Scheduler::schedule()
{
	for (auto it = pendingTasks_.begin(); it != pendingTasks_.end();) {
		auto *task = *it;
		if (!task->precedents_.empty()) {
			it++;
			continue;
		}

		runningTasks_.emplace(task);
		it = pendingTasks_.erase(it);

		task->launch();
	}
}

void Scheduler::removeFromGroupTasks(Task *task)
{
	for (auto &[group, tasks] : groupTasks_)
		tasks.remove(task);
}

void Scheduler::taskDone(Task *task)
{
	/* Sample execution time of the task, from launch to notifyDone */
	std::chrono::milliseconds milliseconds =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - task->launchTime_);

	LOG(Task, Debug) << "Task " << task->id() << " executed in "
			 << milliseconds.count() << "ms";

	taskDone_.emit(task);

	runningTasks_.erase(task);
	removeFromGroupTasks(task);

	for (auto *succedent : task->succedents_) {
		if (0 == succedent->removeDependency(task)) {
			runningTasks_.emplace(succedent);
			pendingTasks_.erase(succedent);

			succedent->launch();
		}
	}

	tasksHolder_.erase(task);
}

void Scheduler::queueTask(Task *task, int32_t group)
{
	/* \todo: Detect cyclic dependency */
	tasksHolder_.emplace(task, std::unique_ptr<Task>(task));

	pendingTasks_.emplace(task);
	groupTasks_[group].emplace_back(task);
}

std::list<Task *> &Scheduler::groupTasks(int32_t group)
{
	return groupTasks_[group];
}

void Scheduler::log()
{
	std::stringstream ss;
	for (auto &[group, name] : groupNames_)
		ss << name << "[" << groupTasks_[group].size() << "] ";

	LOG(Task, Info) << ss.str();
}

bool Scheduler::hasCyclicDependency() const
{
	std::map<Task *, int> precedentCnts;
	std::unordered_set<Task *> runningTasks = runningTasks_;
	std::unordered_set<Task *> pendingTasks;

	for (const auto &task : pendingTasks_) {
		if (task->precedents_.empty()) {
			runningTasks.emplace(task);
			continue;
		}

		pendingTasks.emplace(task);
		precedentCnts[task] = task->precedents_.size();
	}

	while (!runningTasks.empty()) {
		Task *task = *runningTasks.begin();
		runningTasks.erase(runningTasks.begin());

		for (Task *succedent : task->succedents_) {
			if (0 == --precedentCnts[succedent]) {
				pendingTasks.erase(succedent);
				runningTasks.emplace(succedent);
			}
		}
	}

	return !pendingTasks.empty();
}

} /* namespace libcamera */
