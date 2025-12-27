/*
 * Copyright (C) 2024, Google Inc.
 *
 * imgsys_task.h - Base class of task to call getImgSysMetaTuning
 */

#pragma once

#include "libcamera/internal/task_scheduler.h"

#include "libcamera/controls.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"

namespace libcamera {

class ImgSysTask : public Task
{
protected:
	ImgSysTask(Scheduler *scheduler, const std::string &id,
		   uint32_t camSysMetaRequestId, uint32_t internalRequestId,
		   Feature feature, IPADelegate *ipa, ControlList &controls);

	void getImgSysMetaTuning(
		const bool needCropTNC16x9,
		const std::vector<ipa::mtkisp7::ImgMetaRequestData> &imgMetaRequests);

	uint32_t camSysMetaRequestId_;
	uint32_t internalRequestId_;
	Feature feature_;

	IPADelegate *ipa_;
	ControlList controls_;
};

} /* namespace libcamera */
