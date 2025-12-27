/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * imgsys.cpp - MTK MtkISP7 ImgSys device
 */

#include "imgsys.h"

#include <dlfcn.h>
#include <numeric>
#include <sys/ioctl.h>

#include <libcamera/formats.h>
#include <libcamera/geometry.h>
#include <libcamera/request.h>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/pools.h"

#include "kernel-headers/mtk_header_desc.h"
#include "kernel-headers/mtk_imgsys.h"
#include "libcamera/framebuffer.h"
#include "pipeline/mtkisp7/imgsys/single_device.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/ImgPortDef.h"
#include "platform/mtkisp7/single_device_helper.h"
#include "platform/mtkisp7/topology.h"

namespace libcamera {

/* TODO: Align the definition with mcnr */
constexpr Size kMeL0Size{ 576, 432 };
constexpr Size kMeL1Size{ 144, 108 };

constexpr Size kMeMapSize0{ 289, 217 };
constexpr Size kMeMapSize1{ 145, 109 };
constexpr Size kMeMapSize2{ 73, 55 };
constexpr Size kMeMapSize3{ 37, 28 };

constexpr Size kFmbSize{ 36, 27 };
constexpr Size kFstSize{ 1, 112 };
constexpr Size kTnrsoSize{ 40, 1 };

constexpr Size kTrawSttSize{ 738624, 1 };
constexpr Size kTunSize{ 219348, 1 };
constexpr Size kCtrlMetaSize{ 28672, 1 };

constexpr uint32_t kCtrlMetaPoolSize = 48;

using namespace NSCam::NSImgStream;

LOG_DECLARE_CATEGORY(MtkISP7)

static IMG_PORT getDevicePort(uint32_t portIdx)
{
	switch (portIdx) {
	case IMG_PORT_LTIMGI:
		return IMG_PORT_TIMGI;
	case IMG_PORT_LTYUV2O:
		return IMG_PORT_TYUV2O;
	case IMG_PORT_LTYUV3O:
		return IMG_PORT_TYUV3O;
	case IMG_PORT_LTYUV4O:
		return IMG_PORT_TYUV4O;
	case IMG_PORT_LTYUV5O:
	case IMG_PORT_FEO:
		return IMG_PORT_TYUV5O;
	default:
		return IMG_PORT(portIdx);
	}
}

ImgSysBufferCache::FormatCache::FormatCache(
	const V4L2DeviceFormat &format,
	uint32_t userId, size_t count, uint64_t offset, bool dynamic)
	: format_(format), userId_(userId), count_(count), offset_(offset),
	  cache_(new SimpleV4L2BufferCache(count, offset)), dynamic_(dynamic)
{
}

ImgSysBufferCache::FormatCache::~FormatCache()
{
	delete cache_;
}

ImgSysBufferCache::~ImgSysBufferCache()
{
	for (auto formatCache : formatCaches_) {
		delete formatCache;
	}
}

bool ImgSysBufferCache::isEmpty() const
{
	for (FormatCache *f : formatCaches_) {
		if (!f->cache_->isEmpty())
			return false;
	}

	return true;
}

int ImgSysBufferCache::get(const FrameBuffer &buffer, uint32_t userId)
{
	std::optional<uint64_t> formatCacheIdx = getFormatIdx(currentFormat_, userId);
	if (!formatCacheIdx.has_value()) {
		LOG(MtkISP7, Error) << "Format uninitialized: "
				    << currentFormat_
				    << " colorSpace "
				    << (currentFormat_.colorSpace.has_value() ? (*currentFormat_.colorSpace).toString() : "9527")
				    << " planesCount "
				    << currentFormat_.planesCount
				    << " userId "
				    << userId;
		for (size_t i = 0; i < formatCaches_.size(); i++)
			LOG(MtkISP7, Error) << "Formats : "
					    << formatCaches_[i]->format_
					    << " colorSpace "
					    << (formatCaches_[i]->format_.colorSpace.has_value() ? (*formatCaches_[i]->format_.colorSpace).toString() : "9527")
					    << " planesCount "
					    << formatCaches_[i]->format_.planesCount
					    << " userId "
					    << formatCaches_[i]->userId_;
		return -EINVAL;
	}
	return formatCaches_[formatCacheIdx.value()]->cache_->get(buffer, userId);
}

void ImgSysBufferCache::put(unsigned int offsetedIdx)
{
	for (FormatCache *f : formatCaches_) {
		if (offsetedIdx >= f->offset_ && offsetedIdx < f->offset_ + f->count_) {
			f->cache_->put(offsetedIdx);
			return;
		}
	}
	LOG(MtkISP7, Fatal) << "Index not found";
}

std::optional<uint64_t> ImgSysBufferCache::getFormatIdx(const V4L2DeviceFormat &fmt, uint32_t userId)
{
	for (uint64_t i = 0; i < formatCaches_.size(); i++) {
		if (formatCaches_[i]->format_.fourcc == fmt.fourcc &&
		    formatCaches_[i]->format_.size == fmt.size &&
		    formatCaches_[i]->format_.planesCount == fmt.planesCount &&
		    (noCheckUserId_ || formatCaches_[i]->userId_ == userId)) {
			return i;
		}
	}
	return std::nullopt;
}

bool ImgSysBufferCache::formatReady(V4L2DeviceFormat &fmt, uint32_t userId)
{
	return getFormatIdx(fmt, userId) != std::nullopt;
}

void ImgSysBufferCache::addCurrentFormat(size_t count, uint64_t offset)
{
	addFormat(currentFormat_, 0, count, offset);
}

void ImgSysBufferCache::addFormat(V4L2DeviceFormat &fmt, uint32_t userId,
				  size_t count, uint64_t offset, bool dynamic)
{
	if (formatReady(fmt, userId)) {
		LOG(MtkISP7, Fatal) << "Format initialized: "
				    << fmt;
		return;
	}
	FormatCache *cache = new FormatCache(fmt, userId, count, offset, dynamic);
	formatCaches_.push_back(cache);
}

ImgsysVideoDevice::ImgsysVideoDevice(const MediaEntity *entity)
	: V4L2VideoDevice(entity)
{
	if (cache_)
		delete cache_;

	cache_ = new ImgSysBufferCache;
}

int ImgsysVideoDevice::open()
{
	int ret = V4L2VideoDevice::open();
	if (ret)
		return ret;

	getCache()->setFormat(format_);
	return 0;
}

int ImgsysVideoDevice::configure(V4L2DeviceFormat *fmt, int resizeRatio,
				 Rectangle crop, bool forceResetCrop)
{
	int ret;
	bool needResetCrop = false;
	if (*fmt != format_) {
		ret = setFormat(fmt);
		if (ret)
			return ret;

		resizeRatio_ = 0;
		crop_ = Rectangle();
		if (forceResetCrop)
			needResetCrop = true;
	}

	if (resizeRatio != resizeRatio_) {
		struct v4l2_ext_control ext_ctrl;
		ext_ctrl.id = V4L2_CID_MTK_RESIZE_RATIO;
		ext_ctrl.size = sizeof(int);
		ext_ctrl.value = resizeRatio;
		ret = setExtControl(&ext_ctrl, -1);
		if (ret)
			return ret;

		resizeRatio_ = resizeRatio;
	}

	/**
 	 * TODO: needResetCrop is used by MCDS_F1 to check the crop == {0,0} request,
	 * and since crop = {0,0} doesn't seem to be invalid in V4l2 api, need to
	 * check with mtk and see if we can fix it in scp
	 */
	if (crop != crop_ || needResetCrop) {
		ret = setSelection(V4L2_SEL_TGT_CROP, &crop);
		if (ret)
			return ret;

		crop_ = crop;
	}

	return 0;
}

int ImgsysVideoDevice::importBuffers(unsigned int count)
{
	assert(getCache()->isEmpty());

	memoryType_ = V4L2_MEMORY_DMABUF;

	int ret = requestBuffers(count, V4L2_MEMORY_DMABUF);
	if (ret)
		return ret;

	getCache()->addCurrentFormat(count, 0);
	getCache()->noCheckUserId_ = true;

	return 0;
}

int ImgsysVideoDevice::importBuffersWithFormat(
	uint32_t userId, unsigned int count, V4L2DeviceFormat *format,
	bool dynamic)
{
	if (caps().isMeta())
		return importBuffers(count);
	else if (!caps().isMultiplanar())
		return -EINVAL;

	if (getCache()->formatReady(*format, userId)) {
		LOG(MtkISP7, Fatal) << "Buffers already allocated " << *format << " "
				    << "name " << driverName()
				    << " : " << deviceName()
				    << " : " << busName();
		return -EINVAL;
	}

	memoryType_ = V4L2_MEMORY_DMABUF;

	uint32_t output_index, output_count;
	int ret = v4l2CreateBuffers(count, format, &output_index, &output_count);
	if (ret)
		LOG(MtkISP7, Fatal) << "Failed to restore v4l2 buffer: " << ret;

	getCache()->addFormat(*format, userId, output_count, output_index, dynamic);

	return 0;
}

int ImgsysVideoDevice::v4l2CreateBuffers(unsigned int count, V4L2DeviceFormat *format,
					 uint32_t *output_index,
					 uint32_t *output_count)
{
	struct v4l2_create_buffers cb;
	cb.count = count;
	cb.memory = memoryType_;
	cb.format.type = bufferType_;

	struct v4l2_pix_format_mplane *pix = &cb.format.fmt.pix_mp;

	pix->width = format->size.width;
	pix->height = format->size.height;
	pix->pixelformat = format->fourcc;
	pix->num_planes = format->planesCount;
	pix->field = V4L2_FIELD_NONE;

	ASSERT(pix->num_planes <= std::size(pix->plane_fmt));

	for (unsigned int i = 0; i < pix->num_planes; ++i) {
		pix->plane_fmt[i].bytesperline = format->planes[i].bpl;
		pix->plane_fmt[i].sizeimage = format->planes[i].size;
	}

	int ret = ioctl(VIDIOC_CREATE_BUFS, &cb);
	if (ret) {
		LOG(MtkISP7, Error) << "Unable to create " << count
				    << " buffers: " << strerror(-ret);
		return ret;
	}

	if (cb.count < count) {
		LOG(MtkISP7, Warning) << "Attempted to create " << count
				      << " buffers, but got " << cb.count;
	}

	*output_index = cb.index;
	*output_count = cb.count;

	return 0;
}

void ImgsysVideoDevice::resetBuffers(uint32_t userId)
{
	for (ImgSysBufferCache::FormatCache *formatCache : getCache()->formatCaches_) {
		if (!formatCache->dynamic_ || formatCache->userId_ != userId)
			continue;

		uint64_t count = formatCache->count_;

		struct v4l2_remove_buffers rb = {};
		rb.index = formatCache->offset_;
		rb.count = count;
		rb.type = bufferType_;
		int ret = ioctl(VIDIOC_REMOVE_BUFS, &rb);
		if (ret) {
			LOG(MtkISP7, Error) << "Unable to remove " << rb.count
					    << " buffers: " << strerror(-ret);
			return;
		}

		auto *format = &formatCache->format_;

		uint32_t output_index, output_count;
		ret = v4l2CreateBuffers(count, format, &output_index, &output_count);
		if (ret) {
			LOG(MtkISP7, Error) << "Unable to create " << count
					    << " buffers: " << strerror(-ret);
			return;
		}

		formatCache->count_ = output_count;
		formatCache->offset_ = output_index;
	}
}

int ImgsysVideoDevice::releaseBuffers()
{
	delete cache_;
	cache_ = new ImgSysBufferCache;

	return requestBuffers(0, memoryType_);
}

int ImgsysVideoDevice::setFormat(V4L2DeviceFormat *format)
{
	int ret = V4L2VideoDevice::setFormat(format);
	if (ret) {
		LOG(MtkISP7, Info) << "Failed format " << *format;
		return ret;
	}

	getCache()->setFormat(*format);
	format_ = *format;
	crop_ = Rectangle();
	resizeRatio_ = 0;

	return 0;
}

int ImgsysVideoDevice::getFormat(V4L2DeviceFormat *format)
{
	int ret = V4L2VideoDevice::getFormat(format);
	if (ret)
		return ret;
	getCache()->setFormat(*format);

	return 0;
}

ImgSysBufferCache *ImgsysVideoDevice::getCache()
{
	return static_cast<ImgSysBufferCache *>(cache_);
}

Rectangle ImgSysDevice::getCrop(Size inSize, Size outSize)
{
	/* 4:3 */
	if (outSize.width * 3 == outSize.height * 4)
		return { 0, 0, inSize.width, inSize.height };

	/* 16:9 */
	unsigned int height = inSize.width * 9 / 16;
	int y = (inSize.height - height) / 2;

	return { 0, y, inSize.width, height };
}

/**
 * \brief Shrink rectangular crop area by 3%
 * \param[in] originalCrop Rectangle crop area
 *
 * MediaTek TNR algorithm failed to denoise the edges of the image,
 * causing a visible bad image quality on those edges.
 * This function is a workaround because they do not want to fix the issue.
 * The left, right, top, and bottom part of the given crop area will be cut
 * by 3%.
 *
 * \return Shrank crop area
 */
Rectangle ImgSysDevice::cropNoisyBorder(const Rectangle &originalCrop)
{
	// Crop out the noisy border by 5% due to MCNR limitation
	float x = originalCrop.x + (originalCrop.width * (1.0f - 0.97)) / 2;
	float y = originalCrop.y + (originalCrop.height * (1.0f - 0.97)) / 2;

	float w = (originalCrop.width * 0.97);
	float h = (originalCrop.height * 0.97);

	Rectangle result;
	result.x = x + 0.5;
	result.y = y + 0.5;
	result.width = w;
	result.height = h;

	result.width &= ~(0x01);
	result.height &= ~(0x01);

	return result;
}

ImgSysDevice::ImgSysDevice(OnDeviceTuner *odt)
	: onDeviceTuner_(odt)
{
}

int ImgSysDevice::init(MediaDevice *media, DmaHeap *dmaHeap)
{
	media_ = media;
	dmaHeap_ = dmaHeap;

	const std::string hubName = "MTK-ISP-DIP-V4L2";
	mtkIspDip_ = V4L2Subdevice::fromEntityName(media_, hubName);

	if (!mtkIspDip_ || mtkIspDip_->open())
		return -ENODEV;

	/* The four entities would be configured differently */
	MediaEntity *sigdevNorm = media_->getEntityByName(hubName + " SIGDEVN");
	MediaEntity *tuningMeta = media_->getEntityByName(hubName + " Tuning");
	MediaEntity *ctrlMeta = media_->getEntityByName(hubName + " CtrlMeta");

	media_->disableLinks();

	/* Helper function to configure video nodes */
	auto configureVideo = [](ImgsysVideoDevice *device,
				 const PixelFormat &pixelFormat, Size size) {
		V4L2DeviceFormat format;
		format.size = size;
		format.fourcc = device->toV4L2PixelFormat(pixelFormat);

		device->setFormat(&format);
	};

	/* Find video devices, configure and save them in allVideoDevices_*/
	for (auto &port : ports) {
		MediaEntity *entity = media_->getEntityByName(port.device_name);
		if (!entity) {
			LOG(MtkISP7, Warning) << "Entity " << port.device_name << " not found";
			continue;
		}

		if (entity->type() != MediaEntity::Type::V4L2VideoDevice)
			continue;

		// Enable the only link of video devices to/from hub
		MediaLink *link = entity->pads()[0]->links()[0];
		link->setEnabled(true);

		std::unique_ptr<ImgsysVideoDevice> videoDev =
			std::make_unique<ImgsysVideoDevice>(entity);

		if (videoDev->open())
			return -ENODEV;

		if (entity == sigdevNorm) {
			// Weak ptr for sigdevNorm for easier queuing requests
			sigdevNorm_ = videoDev.get();
			configureVideo(videoDev.get(), formats::MTSR_MTISP, { sizeof(struct singlenode_desc_norm), 1 });
		} else if (entity == ctrlMeta) {
			ctrlMeta_ = videoDev.get();
			configureVideo(videoDev.get(), formats::MTFP_MTISP, { 28672, 1 });
		} else if (entity == tuningMeta)
			configureVideo(videoDev.get(), formats::MTFD_MTISP, { 219348, 1 });

		// All video devices for easier streamOn/Off
		allVideoDevices_[IMG_PORT(port.port_index)] = std::move(videoDev);
	}

	std::vector<UniqueFD> requests;
	media_->allocateRequests(kCtrlMetaPoolSize, requests);
	mediaRequestPool_.setData(requests);

	/* Sync token starts from 1 */
	std::vector<BasicContainer<uint32_t>> syncs;
	for (uint32_t i = 1; i < 200; i++)
		syncs.emplace_back(i);

	syncPool_.setData(syncs);

	for (auto &[portIdx, device] : allVideoDevices_)
		device->requestBufferReady.connect(this, &ImgSysDevice::bufferReady);

	return 0;
}

void reconfigureVideoNode(ImgsysVideoDevice &device, const PortInfoEx &info)
{
	if (info.portIdx == IMG_PORT_METAI ||
	    info.portIdx == IMG_PORT_DRV_CTRLMETAI ||
	    info.portIdx == IMG_PORT_DRV_SIGDEV_NORMI ||
	    info.portIdx == IMG_PORT_IMGSTATO)
		return;

	V4L2DeviceFormat format;
	uint32_t fourcc =
		getV4L2Fmt(info.img.getImgFormat(),
			   info.img.getColorArrangement());

	format.size = { static_cast<unsigned int>(info.img.getImgSize().w),
			static_cast<unsigned int>(info.img.getImgSize().h) };
	format.fourcc = V4L2PixelFormat(fourcc);
	format.planesCount = info.img.getPlaneCount();
	for (unsigned int i = 0; i < format.planesCount; ++i) {
		format.planes[i] = { static_cast<uint32_t>(info.img.getBufSizeInBytes(i)),
				     static_cast<uint32_t>(info.img.getBufStridesInBytes(i)) };
	}
	Rectangle crop =
		Rectangle(info.CropX, info.CropY,
			  { static_cast<unsigned int>(info.CropW),
			    static_cast<unsigned int>(info.CropH) });
	bool forceResetCrop = false;
	/* Force reset crop when crop == {0,0} in mfnr*/
	if (crop == Rectangle() &&
	    (info.portIdx == IMG_PORT_LTYUV2O || info.portIdx == IMG_PORT_TYUV2O ||
	     info.portIdx == IMG_PORT_LTYUV3O || info.portIdx == IMG_PORT_TYUV3O ||
	     info.portIdx == IMG_PORT_LTYUV4O || info.portIdx == IMG_PORT_TYUV4O ||
	     info.portIdx == IMG_PORT_LTYUV5O)) {
		forceResetCrop = true;
	}
	device.configure(&format, info.mResizeRatio, crop, forceResetCrop);
}

int ImgSysDevice::queueRequestV4L2(Request *request)
{
	SharedMailBox<InfoFrame> ctrlMeta = makeMailBox<InfoFrame>();
	ctrlMetaPool_.fetch(ctrlMeta);
	const InfoFrame &infoCtrl = ctrlMeta->get();

	SharedMailBox<InfoFrame> singleDevNorm = makeMailBox<InfoFrame>();
	int mediaRequest = mediaRequestPool_.get();

	std::vector<PEU_Stage> stages{
		request->sdRequest->Stages()[request->stage].getStageEnum()
	};
	{
		DmaSyncer syncerCtrl(infoCtrl.buffer()->planes()[0].fd.get(), DmaHeap::SyncWrite);

		request->sdRequest->fillRequestBufferForStage(
			infoCtrl, mediaRequest, request->stage);
		onDeviceTuner_->tuneImgsysMetadata(
			request->sdRequest->sequence(),
			request->frameNumber,
			request->layer,
			stages,
			infoCtrl, mediaRequest);
	}

	StageEx &stage = request->sdRequest->Stages()[request->stage];
	int ret = 0;

	for (const PortInfoEx &port : stage.getInputs()) {
		IMG_PORT portIdx = getDevicePort(port.portIdx);
		ImgsysVideoDevice &device = *allVideoDevices_[portIdx];
		reconfigureVideoNode(device, port);

		for (size_t p = 0; p < port.frameBuffer->planes().size(); ++p)
			port.frameBuffer->_d()->metadata().planes()[p].bytesused =
				port.frameBuffer->planes()[p].length;

		ret |= device.queueBuffer(port.frameBuffer, mediaRequest, request->userId);
		request->buffers_count++;
	}

	for (const PortInfoEx &port : stage.getOutputs()) {
		IMG_PORT portIdx = getDevicePort(port.portIdx);
		ImgsysVideoDevice &device = *allVideoDevices_[portIdx];
		reconfigureVideoNode(device, port);
		for (size_t p = 0; p < port.frameBuffer->planes().size(); ++p)
			port.frameBuffer->_d()->metadata().planes()[p].bytesused =
				port.frameBuffer->planes()[p].length;

		ret |= device.queueBuffer(port.frameBuffer, mediaRequest, request->userId);
		request->buffers_count++;
	}

	ret |= ctrlMeta_->queueBuffer(infoCtrl.buffer(), mediaRequest, request->userId);
	request->buffers_count++;
	ret |= media_->queueRequest(mediaRequest);

	if (ret)
		LOG(MtkISP7, Fatal) << "Fail to queue request. Need to check driver error";

	pendingRequests_.push_back({ request, mediaRequest,
				     request->sdRequest->sequence(),
				     stages, ctrlMeta,
				     singleDevNorm });

	return 0;
}

int ImgSysDevice::queueRequest(Request *request)
{
	SharedMailBox<InfoFrame> ctrlMeta = makeMailBox<InfoFrame>();
	ctrlMetaPool_.fetch(ctrlMeta);
	const InfoFrame &infoCtrl = ctrlMeta->get();

	SharedMailBox<InfoFrame> singleDevNorm = makeMailBox<InfoFrame>();
	descPool_.fetch(singleDevNorm);
	const InfoFrame &infoDesc = singleDevNorm->get();

	FrameBuffer *singleDev = infoDesc.buffer();
	int mediaRequest = mediaRequestPool_.get();

	{
		DmaSyncer syncerCtrl(infoCtrl.buffer()->planes()[0].fd.get(), DmaHeap::SyncWrite);
		DmaSyncer syncerDesc(infoDesc.buffer()->planes()[0].fd.get(), DmaHeap::SyncWrite);

		request->sdRequest->fillRequestBuffer(infoCtrl, infoDesc, mediaRequest);
		onDeviceTuner_->tuneImgsysMetadata(
			request->sdRequest->sequence(),
			request->frameNumber,
			request->layer,
			request->sdRequest->getStageEnums(),
			infoCtrl, mediaRequest);
	}

	int ret = sigdevNorm_->queueBuffer(singleDev, mediaRequest, request->userId);
	request->buffers_count++;
	ret |= media_->queueRequest(mediaRequest);

	if (ret) {
		LOG(MtkISP7, Error) << "Fail to queue request";
		return ret;
	}

	pendingRequests_.push_back({ request, mediaRequest,
				     request->sdRequest->sequence(),
				     request->sdRequest->getStageEnums(),
				     ctrlMeta, singleDevNorm });
	return 0;
}

int ImgSysDevice::claimCompletedRequest(Request *request)
{
	for (auto iter = completedRequests_.begin();
	     iter != completedRequests_.end(); ++iter) {
		if (*iter == request) {
			completedRequests_.erase(iter);
			return 0;
		}
	}

	return -EINVAL;
}

void ImgSysDevice::bufferReady(std::pair<FrameBuffer *, int> pair)
{
	auto [buffer, mediaRequest] = pair;
	ASSERT(buffer);

	bool foundRequest = false;
	for (auto iter = pendingRequests_.begin();
	     iter != pendingRequests_.end(); iter++) {
		PendingRequest &request = *iter;
		if (request.mediaRequest != mediaRequest)
			continue;

		if (request.request->buffers_count <= 0) {
			LOG(MtkISP7, Error)
				<< "Buffer count for media request: "
				<< mediaRequest << "doesn't match.";
			return;
		}

		foundRequest = true;

		request.request->buffers_count--;

		if (request.request->buffers_count)
			return;

		/* Mark request as completed */
		completedRequests_.emplace_back(request.request);
		requestCompleted.emit(request.request);

		/* Use the media request to tune the driver */
		onDeviceTuner_->tuneImgsysDriver(request.internalRequestId,
						 request.mediaRequest,
						 request.stages);

		/* Re-init media request. Buffers will be recycled on the
		 * destructor of PendingRequest */
		media_->reInitRequest(request.mediaRequest);
		mediaRequestPool_.put(request.mediaRequest);

		pendingRequests_.erase(iter);
		break;
	}

	ASSERT(foundRequest == true);
}

int ImgSysDevice::configure(
	const Size sensorFullSize, const Size CamSysYuv,
	const Size video1, const Size video2,
	const Size still1, const Size still2,
	const Size wrappingMapSize, const Size confMapSize)
{
#if !V4L2_STANDARD_MODE
	handleIova(Delete, ctrlMetaPool_);
	descPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size{ 266960, 1 }, 32, DmaHeap::CMA);
#endif

	ctrlMetaPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size{ 28672, 1 }, kCtrlMetaPoolSize, DmaHeap::CMA);

#if !V4L2_STANDARD_MODE
	handleKva(Add, descPool_);
	handleIova(Add, ctrlMetaPool_);

	descPool_.mmap();
#endif
	ctrlMetaPool_.mmap();

	releaseAllBuffers();

	int ret = importBuffers(sensorFullSize, CamSysYuv,
				video1, video2, still1, still2,
				wrappingMapSize, confMapSize);
	if (ret) {
		releaseAllBuffers();
		LOG(MtkISP7, Fatal) << "Failed to import buffers";
	}

	return ret;
}

int ImgSysDevice::start()
{
	for (auto &[portIdx, device] : allVideoDevices_) {
		int ret = device->streamOn();
		if (ret) {
			LOG(MtkISP7, Error) << "Fail to start "
					    << device->devicePath();
			return ret;
		}
	}

	ASSERT(pendingRequests_.empty());
	ASSERT(completedRequests_.empty());

	return 0;
}

int ImgSysDevice::stop()
{
	for (auto &[portIdx, device] : allVideoDevices_) {
		int ret = device->streamOff();
		if (ret) {
			LOG(MtkISP7, Error) << "Fail to streamOff "
					    << device->devicePath();
			return ret;
		}
	}

	ASSERT(pendingRequests_.empty());
	ASSERT(completedRequests_.empty());

	return 0;
}

int ImgSysDevice::handleKva(FdCtrl fdHandle, InfoFramePool &pool)
{
	std::vector<int> data = pool.collectFds();
	if (data.empty())
		return -EINVAL;

	const unsigned long int ctrl =
		(fdHandle == Add) ? MTKDIP_IOC_ADD_KVA : MTKDIP_IOC_DEL_KVA;

	struct fd_info fds;
	fds.fd_num = data.size();
	std::copy(data.begin(), data.end(), fds.fds);

	if (mtkIspDip_->ioctl(ctrl, &fds)) {
		LOG(MtkISP7, Error) << "Fail to handle kva";
		return -EINVAL;
	}

	return 0;
}

int ImgSysDevice::handleIova(FdCtrl fdHandle, InfoFramePool &pool)
{
	std::vector<int> data = pool.collectFds();
	if (data.empty())
		return -EINVAL;

	const unsigned long int ctrl =
		(fdHandle == Add) ? MTKDIP_IOC_ADD_IOVA : MTKDIP_IOC_DEL_IOVA;

	struct fd_tbl fds;
	fds.fd_num = data.size();
	fds.fds = reinterpret_cast<uint64_t>(data.data());

	if (mtkIspDip_->ioctl(ctrl, &fds)) {
		LOG(MtkISP7, Error) << "Fail to handle iova";
		return -EINVAL;
	}

	return 0;
}

void ImgSysDevice::importBufferByList(std::vector<PortBuffers> &portBufs, uint32_t userId)
{
	for (PortBuffers &portBuf : portBufs) {
		// TODO: Remove the usage of getDevicePort.
		IMG_PORT adjustedPort =
			getDevicePort(static_cast<uint32_t>(portBuf.port));

		V4L2DeviceFormat deviceFormat;
		deviceFormat.fourcc = V4L2PixelFormat(portBuf.pixelFmt);
		const PixelFormatInfo pixFmtInfo = PixelFormatInfo::info(deviceFormat.fourcc);
		deviceFormat.size = portBuf.size;
		deviceFormat.planesCount = pixFmtInfo.numPlanes();
		deviceFormat.planes = {};
		for (uint64_t i = 0; i < deviceFormat.planesCount; i++) {
			deviceFormat.planes[i].size =
				pixFmtInfo.planeSize(deviceFormat.size, i,
						     portBuf.strideAlign,
						     portBuf.scanAlign);
			deviceFormat.planes[i].bpl =
				pixFmtInfo.stride(deviceFormat.size.width, i,
						  portBuf.strideAlign);
		}

		// TODO: Do this after size/stride calculation and overwrite
		// the fourcc. Remove the translation.
		deviceFormat.fourcc = getImgSysV4L2PixelFormat(portBuf.pixelFmt);
		int ret = allVideoDevices_[adjustedPort]->importBuffersWithFormat(
			userId, portBuf.count, &deviceFormat, portBuf.dynamic);
		if (ret)
			LOG(MtkISP7, Fatal) << "Failed to import buffer: " << ret;
	}
}

/**
 * Prepares the V4L2 devices to work with these buffers.
 *
 * Technical debts todo:
 * #1 The specified format may not be the actual format.
 * #2 The specified IMG_PORT may not be the actual port.
 */
int ImgSysDevice::importBuffers(
	const Size sensorFullSize, const Size CamSysYuv,
	const Size video1, const Size video2,
	const Size still1, const Size still2,
	const Size wrappingMapSize, const Size confMapSize)
{
	/*
	 * We don't need to actually get format, this is just to initilize the
	 * currentformat_ for the cache.
	 * TODO: design the cache better and get rid of this.
	 */
	for (auto &[_, device] : allVideoDevices_) {
		V4L2DeviceFormat deviceFormat;
		device->getFormat(&deviceFormat);
	}

	std::vector<Size> mcnrSizes(7);
	Size size = CamSysYuv;
	/* Assign the size to 1/2 of the previous level.
	 * Align to 2 for hardware's requirement */
	for (size_t i = 0; i < mcnrSizes.size(); i++) {
		mcnrSizes[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignUpTo(2, 2);
	}

	std::vector<Size> meMmapSizes(4);
	std::vector<Size> wtSizes(7);

	meMmapSizes[0] = kMeMapSize0;
	meMmapSizes[1] = kMeMapSize1;
	meMmapSizes[2] = kMeMapSize2;
	meMmapSizes[3] = kMeMapSize3;

	wtSizes[0] = mcnrSizes[3];
	wtSizes[1] = mcnrSizes[3];
	wtSizes[2] = mcnrSizes[3];
	wtSizes[3] = mcnrSizes[3];
	wtSizes[4] = mcnrSizes[4];
	wtSizes[5] = mcnrSizes[5];
	wtSizes[6] = mcnrSizes[5];

	bool needCropTNC16x9 = false;
	if ((video1.width * 9 == video1.height * 16) &&
	    (video2.width * 9 == video2.height * 16))
		needCropTNC16x9 = true;

	Size tncSize(mcnrSizes[0]);
	if (needCropTNC16x9) {
		tncSize.height = tncSize.width * 9 / 16;
	}

	Size meConf0 = kMeL1Size;
	Size meConf4 = mcnrSizes[4].boundedTo(kMeL1Size);
	Size meConf5 = mcnrSizes[5].boundedTo(kMeL1Size);

	// TODO: Move the portBufs calculation into a specific plannar class
	// and share it with mcnr/lpnr/mfnr manager for buffer allocation.
	std::vector<PortBuffers> portBufsMcnr = {
		{ IMG_PORT_IMGI, formats::NV12_10P_MTISP, mcnrSizes[0], 14, 1, 1 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[1], 14, 1, 1 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[2], 16, 1, 64 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[3], 16, 1, 64 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[4], 16, 1, 64 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[5], 16, 1, 64 },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mcnrSizes[6], 16, 1, 64 },

		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[1], 16, 1, 64 },
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[2], 16, 1, 64 },
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[3], 16, 1, 64 },
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[4], 16, 1, 64 },
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[5], 16, 1, 64 },
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mcnrSizes[6], 16, 1, 64 },

		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[1], 16, 1, 64 },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[2], 16, 1, 64 },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[3], 16, 1, 64 },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[4], 16, 1, 64 },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[5], 16, 1, 64 },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mcnrSizes[6], 16, 1, 64 },

		{ IMG_PORT_TNRSI, formats::Y32_MTISP, kTnrsoSize, 64, 1, 1 },

		{ IMG_PORT_TNRWI, formats::GREY, wtSizes[3], 48, 192, 192 },
		{ IMG_PORT_TNRWI, formats::GREY, wtSizes[4], 12, 192, 192 },
		{ IMG_PORT_TNRWI, formats::GREY, wtSizes[5], 12, 192, 192 },

		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[1], 3, 16, 1 },
		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[2], 3, 16, 1 },
		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[3], 3, 16, 1 },
		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[4], 3, 16, 1 },
		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[5], 3, 16, 1 },
		{ IMG_PORT_TNRMI, formats::GREY, mcnrSizes[6], 3, 16, 1 },

		{ IMG_PORT_TNRLFDI, formats::NV21, mcnrSizes[6], 128, 1, 1 },

		{ IMG_PORT_TNRVBI, formats::GREY, mcnrSizes[2], 8, 16, 1 },
		{ IMG_PORT_TNRVBI, formats::GREY, mcnrSizes[3], 4, 16, 1 },
		{ IMG_PORT_TNRVBI, formats::GREY, mcnrSizes[4], 4, 16, 1 },
		{ IMG_PORT_TNRVBI, formats::GREY, mcnrSizes[5], 4, 16, 1 },
		{ IMG_PORT_TNRVBI, formats::GREY, mcnrSizes[6], 4, 16, 1 },

		{ IMG_PORT_TNRSO, formats::Y32_MTISP, kTnrsoSize, 64, 1, 1 },

		{ IMG_PORT_TNRWO, formats::GREY, wtSizes[3], 48, 192, 192 },
		{ IMG_PORT_TNRWO, formats::GREY, wtSizes[4], 12, 192, 192 },
		{ IMG_PORT_TNRWO, formats::GREY, wtSizes[5], 12, 192, 192 },

		{ IMG_PORT_TNRMO, formats::GREY, mcnrSizes[1], 3, 16, 1 },
		{ IMG_PORT_TNRMO, formats::GREY, mcnrSizes[2], 3, 16, 1 },
		{ IMG_PORT_TNRMO, formats::GREY, mcnrSizes[3], 3, 16, 1 },
		{ IMG_PORT_TNRMO, formats::GREY, mcnrSizes[4], 3, 16, 1 },
		{ IMG_PORT_TNRMO, formats::GREY, mcnrSizes[5], 3, 16, 1 },

		{ IMG_PORT_IMG3O, formats::NV12_10P_MTISP, tncSize, 3, 1, 1 },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mcnrSizes[1], 16, 1, 64 },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mcnrSizes[2], 16, 1, 64 },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mcnrSizes[3], 16, 1, 64 },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mcnrSizes[4], 16, 1, 64 },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mcnrSizes[5], 16, 1, 64 },
		{ IMG_PORT_IMG3O, formats::NV21, mcnrSizes[6], 21, 1, 1 },

		{ IMG_PORT_IMG4O, formats::NV12_10P_MTISP, mcnrSizes[0], 8, 1, 1 },
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mcnrSizes[1], 8, 1, 1 },

		// TR_F1/LTR_F1
		{ IMG_PORT_TIMGI, formats::NV12_12P_MTISP, mcnrSizes[1], 14, 1, 1 },
		{ IMG_PORT_TYUV2O, formats::NV12_12P_MTISP, mcnrSizes[2], 32, 1, 64 },
		{ IMG_PORT_TYUV3O, formats::NV12_12P_MTISP, mcnrSizes[3], 32, 1, 64 },
		{ IMG_PORT_TYUV4O, formats::NV12_12P_MTISP, mcnrSizes[4], 32, 1, 64 },
		{ IMG_PORT_TYUV5O, formats::GREY, mcnrSizes[2], 4, 16, 1 },

		// TR_F4/LTR_F4
		{ IMG_PORT_TIMGI, formats::NV12_12P_MTISP, mcnrSizes[4], 32, 1, 64 },
		{ IMG_PORT_TYUV2O, formats::NV12_12P_MTISP, mcnrSizes[5], 32, 1, 64 },
		{ IMG_PORT_TYUV3O, formats::NV12_12P_MTISP, mcnrSizes[6], 32, 1, 64 },

		// HW_MVMAP
		{ IMG_PORT_TIMGI, formats::WARP2P_MTISP, kMeMapSize0, 24, 1168, 217 },
		{ IMG_PORT_TYUV2O, formats::WARP2P_MTISP, kMeMapSize1, 40, 1168, 217 },
		{ IMG_PORT_TYUV3O, formats::WARP2P_MTISP, kMeMapSize2, 24, 1168, 217 },
		{ IMG_PORT_TYUV4O, formats::WARP2P_MTISP, kMeMapSize3, 24, 1168, 217 },

		// LTR_ME
		{ IMG_PORT_TIMGI, formats::GREY, kMeL0Size, 14, 64, 1 },
		{ IMG_PORT_TYUV2O, formats::GREY, kMeL1Size, 12, 576, 432 },

		{ IMG_PORT_TIMGI, formats::GREY, meConf0, 16, 144, 108 },
		{ IMG_PORT_TYUV5O, formats::GREY, meConf4, 12, 144, 108 },
		{ IMG_PORT_TYUV5O, formats::GREY, meConf5, 12, 144, 108 },

		// vbi[2]
		{ IMG_PORT_TIMGI, formats::GREY, mcnrSizes[2], 4, 16, 1 },
		{ IMG_PORT_TYUV2O, formats::GREY, mcnrSizes[3], 4, 16, 1 },
		{ IMG_PORT_TYUV3O, formats::GREY, mcnrSizes[4], 4, 16, 1 },
		{ IMG_PORT_TYUV4O, formats::GREY, mcnrSizes[5], 4, 16, 1 },

		// HW_LTR_F1
		{ IMG_PORT_WPE_WPEI, formats::NV12_12P_MTISP, mcnrSizes[1], 8, 1, 1 },

		{ IMG_PORT_WPE_WPEI, formats::GREY, wtSizes[3], 48, 192, 192 },
		{ IMG_PORT_WPE_WPEI, formats::GREY, wtSizes[4], 12, 192, 192 },
		{ IMG_PORT_WPE_WPEI, formats::GREY, wtSizes[5], 12, 192, 192 },

		{ IMG_PORT_WPE_VECI, formats::WARP2P_MTISP, kMeMapSize0, 40, 1168, 217 },
		{ IMG_PORT_WPE_VECI, formats::WARP2P_MTISP, kMeMapSize1, 40, 1168, 217 },
		{ IMG_PORT_WPE_VECI, formats::WARP2P_MTISP, kMeMapSize2, 24, 1168, 217 },
		{ IMG_PORT_WPE_VECI, formats::WARP2P_MTISP, kMeMapSize3, 24, 1168, 217 },

		{ IMG_PORT_WPE_WPEO, formats::NV12_12P_MTISP, mcnrSizes[1], 16, 1, 64 },
		{ IMG_PORT_WPE_WPEO, formats::GREY, wtSizes[3], 48, 192, 192 },
		{ IMG_PORT_WPE_WPEO, formats::GREY, wtSizes[4], 12, 192, 192 },
		{ IMG_PORT_WPE_WPEO, formats::GREY, wtSizes[5], 12, 192, 192 },

		{ IMG_PORT_WPE_TNR_WPEI, formats::NV12_10P_MTISP, mcnrSizes[0], 8, 1, 1 },
		{ IMG_PORT_WPE_TNR_VECI, formats::WARP2P_MTISP, kMeMapSize0, 24, 1168, 217 },

		{ IMG_PORT_ME_L0_IMG0I, formats::GREY, kMeL0Size, 14, 64, 1 },
		{ IMG_PORT_ME_L0_IMG1I, formats::GREY, kMeL0Size, 14, 64, 1 },
		{ IMG_PORT_ME_L1_IMG0I, formats::GREY, kMeL1Size, 12, 576, 432 },
		{ IMG_PORT_ME_L1_IMG1I, formats::GREY, kMeL1Size, 12, 576, 432 },

		{ IMG_PORT_ME_MEMILI, formats::Y8_MTISP, kMeL1Size, 8, 1, 1 },

		{ IMG_PORT_ME_L0_RMVI, formats::Y32_MTISP, kMeL1Size, 128, 1, 1 },
		{ IMG_PORT_ME_L1_RMVI, formats::Y32_MTISP, kFmbSize, 24, 1, 1 },
		{ IMG_PORT_ME_L0_WMVO, formats::Y32_MTISP, kMeL1Size, 24, 1, 1 },
		{ IMG_PORT_ME_L1_WMVO, formats::Y32_MTISP, kFmbSize, 24, 1, 1 },

		{ IMG_PORT_ME_CONFO, formats::GREY, meConf0, 8, 144, 108 },
		{ IMG_PORT_ME_WMAPO, formats::WARP2P_MTISP, kMeMapSize0, 24, 1168, 217 },
		{ IMG_PORT_ME_L0_FMBI, formats::Y32_MTISP, kFmbSize, 8, 1, 1 },
		{ IMG_PORT_ME_L1_FMBI, formats::Y32_MTISP, kFmbSize, 8, 1, 1 },
		{ IMG_PORT_ME_L0_FMBO, formats::Y32_MTISP, kFmbSize, 8, 1, 1 },
		{ IMG_PORT_ME_L1_FMBO, formats::Y32_MTISP, kFmbSize, 8, 1, 1 },
		{ IMG_PORT_ME_FSTO, formats::Y32_MTISP, kFstSize, 8, 1, 1 },
		{ IMG_PORT_ME_LMIO, formats::Y16_MTISP, kMeL1Size, 8, 1, 1 },
	};

	if (!video1.isNull())
		portBufsMcnr.push_back({ IMG_PORT_WDMAO, formats::NV12, video1, 3, 64, 1 });

	if (!video2.isNull())
		portBufsMcnr.push_back({ IMG_PORT_WROTO, formats::NV12, video2, 3, 64, 1 });

	if (meConf0 == meConf4) {
		portBufsMcnr.push_back({ IMG_PORT_TNRCI, formats::GREY, meConf0, 40, 144, 108 });
		portBufsMcnr.push_back({ IMG_PORT_TNRCI, formats::GREY, meConf5, 24, 144, 108 });
	} else {
		portBufsMcnr.push_back({ IMG_PORT_TNRCI, formats::GREY, meConf0, 24, 144, 108 });
		portBufsMcnr.push_back({ IMG_PORT_TNRCI, formats::GREY, meConf4, 28, 144, 108 });
		portBufsMcnr.push_back({ IMG_PORT_TNRCI, formats::GREY, meConf5, 28, 144, 108 });
	}

	std::vector<Size> lpnrSizes(4);

	/* Assign the size to 1/4 of the previous level.
	 * Align to 2 for hardware's requirement */
	size = sensorFullSize;
	for (size_t i = 0; i < lpnrSizes.size(); i++) {
		lpnrSizes[i] = size;
		size.width = (size.width + 3) / 4;
		size.height = (size.height + 3) / 4;
		size.alignUpTo(2, 2);
	}

	std::vector<PortBuffers> portBufsLpnr = {
		{ IMG_PORT_TIMGI, formats::SRGGB10_MTISP, lpnrSizes[0], 4, 1, 1 },
		{ IMG_PORT_TIMGI, formats::SGRBG10_MTISP, lpnrSizes[0], 4, 1, 1 },
		{ IMG_PORT_IMGI, formats::NV12_10P_MTISP, lpnrSizes[0], 4, 1, 1, true },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, lpnrSizes[1], 4, 1, 1, true },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, lpnrSizes[2], 4, 1, 1, true },
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, lpnrSizes[3], 4, 1, 1, true },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, lpnrSizes[1], 4, 1, 1, true },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, lpnrSizes[2], 4, 1, 1, true },
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, lpnrSizes[3], 4, 1, 1, true },
		{ IMG_PORT_IMG3O, formats::NV12_10P_MTISP, lpnrSizes[0], 4, 1, 1, true },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, lpnrSizes[1], 4, 1, 1, true },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, lpnrSizes[2], 4, 1, 1, true },
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, lpnrSizes[3], 4, 1, 1, true },
		{ IMG_PORT_TYUVO, formats::NV12_10P_MTISP, lpnrSizes[0], 4, 1, 1, true },
		{ IMG_PORT_TYUV2O, formats::NV12_12P_MTISP, lpnrSizes[1], 4, 1, 1, true },
		{ IMG_PORT_TYUV3O, formats::NV12_12P_MTISP, lpnrSizes[2], 4, 1, 1, true },
		{ IMG_PORT_TYUV4O, formats::NV12_12P_MTISP, lpnrSizes[3], 4, 1, 1, true },
	};
	std::vector<PortBuffers> portBufsMfnr;
	std::vector<Size> mfnrSizes(7);
	std::vector<Size> mfnrSizes_aligned16(3);
	/* Assign the size to 1/2 of the previous level.
	 * Align to 2 for hardware's requirement */
	size = sensorFullSize;
	for (size_t i = 0; i < mfnrSizes.size(); i++) {
		mfnrSizes[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignUpTo(2, 2);
	}
	size = sensorFullSize;
	for (size_t i = 0; i < mfnrSizes_aligned16.size(); i++) {
		mfnrSizes_aligned16[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignDownTo(16, 16);
	}
	portBufsMfnr = {
		// Used by BFBLD:4
		{ IMG_PORT_TIMGI, formats::SRGGB10_MTISP, mfnrSizes[0], 4, 1, 1, true },
		{ IMG_PORT_TIMGI, formats::SGRBG10_MTISP, mfnrSizes[0], 4, 1, 1, true },
		// Used by BFBLD:4
		{ IMG_PORT_IMG3O, formats::NV12_10P_MTISP, mfnrSizes[0], 4, 1, 1, true },
		// Used by BFBLD:4, BFME:4
		{ IMG_PORT_IMG2O, formats::NV12_10P_MTISP, mfnrSizes_aligned16[2], 8, 1, 1, true },
		// Used by BFME:4
		{ IMG_PORT_IMGI, formats::NV12_10P_MTISP, mfnrSizes_aligned16[2], 4, 1, 1, true },
		// Used by BFME:4
		{ IMG_PORT_IMG2O, formats::Y8_MTISP, mfnrSizes_aligned16[2], 4, 1, 1, true },
		// Used by MCDS_F1:3
		{ IMG_PORT_WPE_WPEI, formats::NV12_10P_MTISP, mfnrSizes[0], 3, 1, 1, true },
		// Used by MCDS_F1:3
		{ IMG_PORT_WPE_VECI, formats::WARP2P_MTISP, wrappingMapSize, 3, 1, 1, false },
		// Used by MCDS_F1:3
		{ IMG_PORT_WPE_WPEO, formats::NV12_10P_MTISP, mfnrSizes[0], 3, 1, 1, true },
		// Used by MCDS_F1:3, DS:1
		{ IMG_PORT_TYUV2O, formats::NV12_12P_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by  MCDS_F1:3, DS:1
		{ IMG_PORT_TYUV3O, formats::NV12_12P_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MCDS_F1:3, DS:1
		{ IMG_PORT_TYUV4O, formats::NV12_12P_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MCDS_F1:3
		{ IMG_PORT_TYUV5O, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },

		// Used by DS:1
		{ IMG_PORT_TIMGI, formats::NV12_10P_MTISP, mfnrSizes[0], 1, 1, 1, true },
		// Used by DS:1
		{ IMG_PORT_TIMGI, formats::NV12_12P_MTISP, mfnrSizes[3], 4, 1, 1, true },
		// Used by DS:4
		{ IMG_PORT_TYUV2O, formats::NV12_12P_MTISP, mfnrSizes[4], 4, 1, 1, true },
		// Used by DS:4
		{ IMG_PORT_TYUV3O, formats::NV12_12P_MTISP, mfnrSizes[5], 4, 1, 1, true },
		// Used by DS:4
		{ IMG_PORT_TYUV4O, formats::NV12_12P_MTISP, mfnrSizes[6], 4, 1, 1, true },

		// Used by DSVBI_V2:3
		{ IMG_PORT_TIMGI, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by DSVBI_V2:3
		{ IMG_PORT_TYUV2O, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by DSVBI_V2:3
		{ IMG_PORT_TYUV3O, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by DSVBI_V2:3
		{ IMG_PORT_TYUV4O, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by DSVBI_V5:3
		{ IMG_PORT_TIMGI, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by DSVBI_V5:3
		{ IMG_PORT_TYUV2O, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },

		// Used by MSBLD/AFBLD:18
		{ IMG_PORT_TNRSI, formats::Y32_MTISP, kTnrsoSize, 18, 1, 1, true },
		// Used by MSBLD/AFBLD:18
		{ IMG_PORT_TNRSO, formats::Y32_MTISP, kTnrsoSize, 18, 1, 1, true },
		// Used by MSBLD/AFBLD;15
		{ IMG_PORT_TNRLFDI, formats::NV21, mfnrSizes[6], 15, 1, 1, true },
		// Used by MSBLD/AFBLD;18
		{ IMG_PORT_TNRCI, formats::Y8_MTISP, confMapSize, 18, 1, 1, false },

		// Used by MSBLD_F6:2, AFBLD_F6:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[6], 3, 1, 1, true },
		// Used by MSBLD_F6:2, AFBLD_F6:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[6], 3, 1, 1, true },
		// Used by MSBLD_F6:2, AFBLD_F6:1
		{ IMG_PORT_IMG4O, formats::NV21, mfnrSizes[6], 3, 1, 1, true },

		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[6], 3, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_TNRVBI, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F5:2
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mfnrSizes[5], 2, 1, 1, true },
		// Used by AFBLD_F5:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[5], 1, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F5:2, AFBLD_F5:1
		{ IMG_PORT_TNRMO, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },

		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_TNRVBI, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_TNRMI, formats::Y8_MTISP, mfnrSizes[5], 3, 1, 1, true },
		// Used by MSBLD_F4:2
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mfnrSizes[4], 2, 1, 1, true },
		// Used by AFBLD_F4:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[4], 1, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F4:2, AFBLD_F4:1
		{ IMG_PORT_TNRMO, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },

		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_TNRVBI, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_TNRMI, formats::Y8_MTISP, mfnrSizes[4], 3, 1, 1, true },
		// Used by MSBLD_F3:2
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mfnrSizes[3], 2, 1, 1, true },
		// Used by AFBLD_F3:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[3], 1, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F3:2, AFBLD_F3:1
		{ IMG_PORT_TNRMO, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },

		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_TNRVBI, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_TNRMI, formats::Y8_MTISP, mfnrSizes[3], 3, 1, 1, true },
		// Used by MSBLD_F2:2
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mfnrSizes[2], 2, 1, 1, true },
		// Used by AFBLD_F2:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[2], 1, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F2:2, AFBLD_F2:1
		{ IMG_PORT_TNRMO, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },

		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_VIPI, formats::NV12_12P_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_IMGI, formats::NV12_12P_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1, MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_TNRVBI, formats::Y8_MTISP, mfnrSizes[1], 6, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_TNRMI, formats::Y8_MTISP, mfnrSizes[2], 3, 1, 1, true },
		// Used by MSBLD_F1:2
		{ IMG_PORT_IMG4O, formats::NV12_12P_MTISP, mfnrSizes[1], 2, 1, 1, true },
		// Used by AFBLD_F1:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[1], 1, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F1:2, AFBLD_F1:1
		{ IMG_PORT_TNRMO, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },

		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_VIPI, formats::NV12_10P_MTISP, mfnrSizes[0], 3, 1, 1, true },
		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_IMGI, formats::NV12_10P_MTISP, mfnrSizes[0], 3, 1, 1, true },
		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_REC_DSI, formats::NV12_12P_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_TNRWI, formats::Y8_MTISP, mfnrSizes[0], 3, 1, 1, true },
		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_TNRMI, formats::Y8_MTISP, mfnrSizes[1], 3, 1, 1, true },
		// Used by MSBLD_F0:2
		{ IMG_PORT_IMG4O, formats::NV12_10P_MTISP, mfnrSizes[0], 2, 1, 1, true },
		// Used by AFBLD_F0:1
		{ IMG_PORT_IMG3O, formats::NV12_12P_MTISP, mfnrSizes[0], 2, 1, 1, true },
		// Used by MSBLD_F0:2, AFBLD_F0:1
		{ IMG_PORT_TNRWO, formats::Y8_MTISP, mfnrSizes[0], 3, 1, 1, true },
	};

	if (!still1.isNull()) {
		portBufsLpnr.push_back({ IMG_PORT_WDMAO, formats::NV12, still1, 3, 64, 1 });
		portBufsMfnr.push_back({ IMG_PORT_WDMAO, formats::NV12, still1, 3, 64, 1, true });
	}

	if (!still2.isNull()) {
		portBufsLpnr.push_back({ IMG_PORT_WROTO, formats::NV12, still2, 3, 64, 1 });
		portBufsMfnr.push_back({ IMG_PORT_WROTO, formats::NV12, still2, 3, 64, 1, true });
	}

	std::vector<PortBuffers> portBufsCommon = {
		{ IMG_PORT_IMGSTATO, formats::MTFD_MTISP, kTrawSttSize, 12, 1, 1 },
		{ IMG_PORT_METAI, formats::MTFD_MTISP, kTunSize, 96, 1, 1 },
		{ IMG_PORT_DRV_CTRLMETAI, formats::MTFD_MTISP, kCtrlMetaSize, kCtrlMetaPoolSize, 1, 1 },
	};

	importBufferByList(portBufsMcnr, kUserIdMcnr);
	importBufferByList(portBufsLpnr, kUserIdLpnr);
	importBufferByList(portBufsMfnr, kUserIdMfnr);
	importBufferByList(portBufsCommon, kUserIdMcnr);

	std::set<IMG_PORT> activePorts;
	for (auto &portBuf : portBufsMcnr) {
		IMG_PORT adjustedPort = getDevicePort(static_cast<uint32_t>(portBuf.port));
		activePorts.insert(adjustedPort);
	}
	for (auto &portBuf : portBufsLpnr) {
		IMG_PORT adjustedPort = getDevicePort(static_cast<uint32_t>(portBuf.port));
		activePorts.insert(adjustedPort);
	}
	for (auto &portBuf : portBufsMfnr) {
		IMG_PORT adjustedPort = getDevicePort(static_cast<uint32_t>(portBuf.port));
		activePorts.insert(adjustedPort);
	}
	for (auto &portBuf : portBufsCommon) {
		IMG_PORT adjustedPort = getDevicePort(static_cast<uint32_t>(portBuf.port));
		activePorts.insert(adjustedPort);
	}

	for (auto &[portIdx, device] : allVideoDevices_) {
		if (activePorts.count(portIdx))
			continue;

		device->importBuffers(1);
	}

	return 0;
}

/**
 * Should only be called in case of fatal failure.
 */
int ImgSysDevice::releaseAllBuffers()
{
	int ret = 0;
	for (auto &[_, device] : allVideoDevices_) {
		ret |= device->releaseBuffers();
	}
	return ret;
}

void ImgSysDevice::resetBuffers(uint32_t userId)
{
	for (auto &[_, videoDevice] : allVideoDevices_) {
		videoDevice->resetBuffers(userId);
	}
}

void ImgSysRequestHelper::queueRequest(uint32_t userId, SingleDeviceRequest &sdRequest)
{
#if V4L2_STANDARD_MODE
	for (size_t stage = 0; stage < sdRequest.Stages().size(); ++stage) {
		int frameNumber = sdRequest.frameNumber(stage);
		uint32_t layer = sdRequest.layer(stage);
		imgSysRequests_.push_back({ &sdRequest, stage, 0, userId, frameNumber, layer });
		imgSys_->queueRequestV4L2(&imgSysRequests_.back());
	}
#else
	imgSysRequests_.push_back({ &sdRequest, 0, 0, userId });
	imgSys_->queueRequest(&imgSysRequests_.back());
#endif

	imgSys_->requestCompleted.connect(this, &ImgSysRequestHelper::requestReady);
	startTime_ = std::chrono::steady_clock::now();
}

void ImgSysRequestHelper::requestReady(ImgSysDevice::Request *request)
{
	for (auto iter = imgSysRequests_.begin(); iter != imgSysRequests_.end(); iter++) {
		if (request != &*iter)
			continue;

		imgSys_->claimCompletedRequest(request);
		imgSysRequests_.erase(iter);
		break;
	}

	if (!imgSysRequests_.empty())
		return;

	imgSys_->requestCompleted.disconnect(this, &ImgSysRequestHelper::requestReady);

	/* Sample running time of the task */
	if (request_->sequence() % 30 == 0) {
		std::chrono::steady_clock::time_point finish =
			std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration d = finish - startTime_;
		std::chrono::milliseconds milliseconds =
			std::chrono::duration_cast<std::chrono::milliseconds>(d);
		LOG(MtkISP7, Debug) << task_->id()
				    << " runs " << milliseconds.count() << "ms";
	}
	task_->notifyDone();
}

} /* namespace libcamera */
