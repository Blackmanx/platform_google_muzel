/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * ipc_pipe_unixsocket.cpp - Image Processing Algorithm IPC module using unix socket
 */

#include "libcamera/internal/ipc_pipe_unixsocket.h"

#include <vector>

#include <libcamera/base/event_dispatcher.h>
#include <libcamera/base/log.h>
#include <libcamera/base/thread.h>
#include <libcamera/base/timer.h>

#include "libcamera/internal/ipc_pipe.h"
#include "libcamera/internal/ipc_unixsocket.h"
#include "libcamera/internal/process.h"

using namespace std::chrono_literals;

namespace libcamera {

LOG_DECLARE_CATEGORY(IPCPipe)

IPCPipeUnixSocket::IPCPipeUnixSocket(const char *ipaModulePath,
				     const char *ipaProxyWorkerPath)
	: IPCPipe(), inCall_(false)
{
	std::vector<int> fds;
	std::vector<std::string> args;
	args.push_back(ipaModulePath);

	socket_ = std::make_unique<IPCUnixSocket>();
	UniqueFD fd = socket_->create();
	if (!fd.isValid()) {
		LOG(IPCPipe, Error) << "Failed to create socket";
		return;
	}
	socket_->readyRead.connect(this, &IPCPipeUnixSocket::readyRead);
	args.push_back(std::to_string(fd.get()));
	fds.push_back(fd.get());

	proc_ = std::make_unique<Process>();
	int ret = proc_->start(ipaProxyWorkerPath, args, fds);
	if (ret) {
		LOG(IPCPipe, Error)
			<< "Failed to start proxy worker process";
		return;
	}

	connected_ = true;
}

IPCPipeUnixSocket::IPCPipeUnixSocket(std::string path)
	: IPCPipe(), inCall_(false)
{
	std::vector<int> fds;

	socket_ = std::make_unique<IPCUnixSocket>();

	// TODO(chenghaoyang): Design a more general API to allow other use
	// cases than CrOS.
	int success = socket_->connectRemote(path);
	if (!success) {
		LOG(IPCPipe, Error) << "Failed to create socket";
		return;
	}

	socket_->readyRead.connect(this, &IPCPipeUnixSocket::readyRead);

	connected_ = true;
}

IPCPipeUnixSocket::~IPCPipeUnixSocket()
{
}

int IPCPipeUnixSocket::sendSync(const IPCMessage &in, IPCMessage *out)
{
	IPCUnixSocket::Payload response;

	int ret = call(in.payload(), &response, in.header().cookie);
	if (ret) {
		LOG(IPCPipe, Error) << "Failed to call sync";
		return ret;
	}

	if (out)
		*out = IPCMessage(response);

	return 0;
}

int IPCPipeUnixSocket::sendAsync(const IPCMessage &data)
{
	int ret = socket_->send(data.payload());
	if (ret) {
		LOG(IPCPipe, Error) << "Failed to call async";
		return ret;
	}

	return 0;
}

Signal<> *IPCPipeUnixSocket::disconnected()
{
	if (socket_)
		return &socket_->disconnected;

	return nullptr;
}

void IPCPipeUnixSocket::readyRead()
{
	IPCUnixSocket::Payload payload;
	int ret = socket_->receive(&payload);
	if (ret) {
		LOG(IPCPipe, Error) << "Receive message failed" << ret;
		return;
	}

	/* \todo Use span to avoid the double copy when callData is found. */
	if (payload.data.size() < sizeof(IPCMessage::Header)) {
		LOG(IPCPipe, Error) << "Not enough data received";
		return;
	}

	std::shared_ptr<IPCMessage> ipcMessage = std::make_shared<IPCMessage>(payload);
	auto callData = callData_.find(ipcMessage->header().cookie);
	if (callData != callData_.end()) {
		*callData->second.response = std::move(payload);
		callData->second.done = true;
		return;
	}

	if (!inCall_) {
		emitRecv(ipcMessage);
		return;
	}

	/* During synchronous call, queue other tasks to process later. */
	Object::invokeMethod(&IPCPipeUnixSocket::emitRecv, ConnectionTypeQueued, ipcMessage);
}

void IPCPipeUnixSocket::emitRecv(std::shared_ptr<IPCMessage> &msg)
{
	recv.emit(msg);
}

int IPCPipeUnixSocket::call(const IPCUnixSocket::Payload &message,
			    IPCUnixSocket::Payload *response, uint32_t cookie)
{
	Timer timeout;
	int ret;

	const auto result = callData_.insert({ cookie, { response, false } });
	const auto &iter = result.first;

	ret = socket_->send(message);
	if (ret) {
		callData_.erase(iter);
		return ret;
	}

	inCall_ = true;

	while (!iter->second.done) {
		Thread::current()->eventDispatcher()->processEventsOnNotifier(socket_->notifier_);
	}

	inCall_ = false;

	callData_.erase(iter);

	return 0;
}

} /* namespace libcamera */
