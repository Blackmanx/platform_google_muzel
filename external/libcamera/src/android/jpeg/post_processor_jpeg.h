/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * post_processor_jpeg.h - JPEG Post Processor
 */

#pragma once

#include <cstdint>

#include <libcamera/base/span.h>

#include <libcamera/controls.h>
#include <libcamera/geometry.h>

#include "../post_processor.h"

#include "encoder_libjpeg.h"
#include "thumbnailer.h"

class CameraDevice;

class PostProcessorJpeg : public PostProcessor
{
public:
	PostProcessorJpeg(CameraDevice *const device);

	int configure(const libcamera::StreamConfiguration &incfg,
		      const libcamera::StreamConfiguration &outcfg) override;
	void process(StreamBuffer *streamBuffer) override;

private:
	void generateThumbnail(const libcamera::FrameBuffer &source,
			       const libcamera::Size &targetSize,
			       unsigned int quality,
			       std::vector<unsigned char> *thumbnail);
	int insertAppSegments(libcamera::Span<uint8_t> jpegBlob,
			      int jpegSize,
			      const libcamera::ControlList &metadata);
	void writeAppSegment(int appSegmentIdx, uint8_t *dest,
			     uint8_t *src, size_t size);

	CameraDevice *const cameraDevice_;
	std::unique_ptr<Encoder> encoder_;
	libcamera::Size streamSize_;
	EncoderLibJpeg thumbnailEncoder_;
	Thumbnailer thumbnailer_;
};
