/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * post_processor_jpeg.cpp - JPEG Post Processor
 */

#include "post_processor_jpeg.h"

#include <cstdint>
#include <cstring>
#include <ios>
#include <vector>

#include "../camera_device.h"
#include "../camera_metadata.h"
#include "../camera_request.h"
#if defined(OS_CHROMEOS)
#include "encoder_jea.h"
#else /* !defined(OS_CHROMEOS) */
#include "encoder_libjpeg.h"
#endif
#include "exif.h"

#include <libcamera/base/log.h>

#include <libcamera/control_ids.h>
#include <libcamera/formats.h>


using namespace libcamera;
using namespace std::chrono_literals;

LOG_DEFINE_CATEGORY(JPEG)

namespace {

const size_t kJpegMarkerSize = 2;
const size_t kJpegMetadataLengthSize = 2;
constexpr uint16_t kJpegSOF0 = 0xFFC0;
constexpr uint16_t kJpegSOF2 = 0xFFC2;
constexpr uint16_t kJpegDHT = 0xFFC4;
constexpr uint16_t kJpegRST0 = 0xFFD0;
constexpr uint16_t kJpegRST1 = 0xFFD1;
constexpr uint16_t kJpegRST2 = 0xFFD2;
constexpr uint16_t kJpegRST3 = 0xFFD3;
constexpr uint16_t kJpegRST4 = 0xFFD4;
constexpr uint16_t kJpegRST5 = 0xFFD5;
constexpr uint16_t kJpegRST6 = 0xFFD6;
constexpr uint16_t kJpegRST7 = 0xFFD7;
constexpr uint16_t kJpegSOF = 0xFFD8;
constexpr uint16_t kJpegEOI = 0xFFD9;
constexpr uint16_t kJpegSOS = 0xFFDA;
constexpr uint16_t kJpegDQT = 0xFFDB;
constexpr uint16_t kJpegDRI = 0xFFDD;
constexpr uint16_t kJpegAPP0 = 0xFFE0;
constexpr uint16_t kJpegAPP1 = 0xFFE1;
constexpr uint16_t kJpegAPP2 = 0xFFE2;
constexpr uint16_t kJpegAPP3 = 0xFFE3;
constexpr uint16_t kJpegAPP4 = 0xFFE4;
constexpr uint16_t kJpegAPP5 = 0xFFE5;
constexpr uint16_t kJpegAPP6 = 0xFFE6;
constexpr uint16_t kJpegAPP7 = 0xFFE7;
constexpr uint16_t kJpegAPP8 = 0xFFE8;
constexpr uint16_t kJpegAPP9 = 0xFFE9;
constexpr uint16_t kJpegAPP10 = 0xFFEA;
constexpr uint16_t kJpegAPP11 = 0xFFEB;
constexpr uint16_t kJpegAPP12 = 0xFFEC;
constexpr uint16_t kJpegAPP13 = 0xFFED;
constexpr uint16_t kJpegAPP14 = 0xFFEE;
constexpr uint16_t kJpegAPP15 = 0xFFEF;
constexpr uint16_t kJpegCOM = 0xFFFE;

} // namespace

PostProcessorJpeg::PostProcessorJpeg(CameraDevice *const device)
	: cameraDevice_(device)
{
}

int PostProcessorJpeg::configure(const StreamConfiguration &inCfg,
				 const StreamConfiguration &outCfg)
{
	if (inCfg.size != outCfg.size) {
		LOG(JPEG, Error) << "Mismatch of input and output stream sizes";
		return -EINVAL;
	}

	if (outCfg.pixelFormat != formats::MJPEG) {
		LOG(JPEG, Error) << "Output stream pixel format is not JPEG";
		return -EINVAL;
	}

	streamSize_ = outCfg.size;

	thumbnailer_.configure(inCfg, inCfg.pixelFormat);

#if defined(OS_CHROMEOS)
	encoder_ = std::make_unique<EncoderJea>();
#else /* !defined(OS_CHROMEOS) */
	encoder_ = std::make_unique<EncoderLibJpeg>();
#endif

	return encoder_->configure(inCfg);
}

void PostProcessorJpeg::generateThumbnail(const FrameBuffer &source,
					  const Size &targetSize,
					  unsigned int quality,
					  std::vector<unsigned char> *thumbnail)
{
	/* Stores the raw scaled-down thumbnail bytes. */
	std::vector<unsigned char> rawThumbnail;

	thumbnailer_.createThumbnail(source, targetSize, &rawThumbnail);

	StreamConfiguration thCfg;
	thCfg.size = targetSize;
	thCfg.pixelFormat = thumbnailer_.pixelFormat();
	thCfg.stride = targetSize.width;
	int ret = thumbnailEncoder_.configure(thCfg);

	if (!rawThumbnail.empty() && !ret) {
		/*
		 * \todo Avoid value-initialization of all elements of the
		 * vector.
		 */
		thumbnail->resize(rawThumbnail.size());

		/*
		 * Split planes manually as the encoder expects a vector of
		 * planes.
		 *
		 * \todo Pass a vector of planes directly to
		 * Thumbnailer::createThumbnailer above and remove the manual
		 * planes split from here.
		 */
		std::vector<Span<uint8_t>> thumbnailPlanes;
		const PixelFormatInfo &formatNV12 = PixelFormatInfo::info(formats::NV12);
		size_t yPlaneSize = formatNV12.planeSize(targetSize, 0);
		size_t uvPlaneSize = formatNV12.planeSize(targetSize, 1);
		thumbnailPlanes.push_back({ rawThumbnail.data(), yPlaneSize });
		thumbnailPlanes.push_back({ rawThumbnail.data() + yPlaneSize, uvPlaneSize });

		int jpeg_size = thumbnailEncoder_.encode(thumbnailPlanes,
							 *thumbnail, {}, quality);
		thumbnail->resize(jpeg_size);

		LOG(JPEG, Debug)
			<< "Thumbnail compress returned "
			<< jpeg_size << " bytes";
	}
}

inline uint16_t parseWord(uint8_t *addr)
{
	return (*addr << 8) + *(addr + 1);
}

inline uint8_t *writeTwoBytes(uint8_t *dst, uint16_t value)
{
	dst[0] = (value >> 8) & 0xFF;
	dst[1] = value & 0xFF;
	return dst + 2;
}

/**
 * \brief Inserts JPEG app segments into existing JPEG blob.
 * \param[in,out] jpegBlob The JPEG blob
 * \param[in] jpegSize Current JPEG size (not buffer size)
 * \param[in] metadata Metadata that may contains the JPEG app segments
 *
 * If buffer size is not enough, then not all app segments will be inserted.
 *
 * \return JPEG size after inserting APP segments.
 */
int PostProcessorJpeg::insertAppSegments(
	libcamera::Span<uint8_t> jpegBlob, int jpegSize,
	const libcamera::ControlList &metadata)
{
	const auto &appSegmentLengthCtrl =
		metadata.get(controls::JpegApplicationSegmentLength);

	if (!appSegmentLengthCtrl.has_value()) {
		return jpegSize;
	}

	const auto &appSegmentLength = *appSegmentLengthCtrl;

	const auto &appSegmentArray =
		metadata.get(controls::JpegApplicationSegmentContent);

	if (!appSegmentArray.has_value()) {
		LOG(JPEG, Error)
			<< "JpegApplicationSegmentLength was not empty but "
			<< "JpegApplicationSegmentContent is empty";
		return jpegSize;
	}

	size_t totalLength = 0;
	for (const auto &length : appSegmentLength) {
		totalLength += length;
	}

	if (totalLength != appSegmentArray->size()) {
		LOG(JPEG, Error) << "JPEG app segment ctrls have inconsistent "
				 << "length. JpegApplicationSegmentLength: "
				 << totalLength
				 << ". JpegApplicationSegmentContent: "
				 << appSegmentArray->size();
		return jpegSize;
	}

	// Verify APP0 and APP1 are not there
	if (appSegmentLength[0] != 0 || appSegmentLength[1] != 0) {
		LOG(JPEG, Error) << "Libcamera reserves APP0 and APP1!";
		return jpegSize;
	}

	int inputAppSegNum = 0;
	// Start from APP2.
	for (int i = 2; i < 16; i++) {
		if (appSegmentLength[i] != 0) {
			inputAppSegNum = i;
			break;
		}
	}

	uint8_t *inputAppSegPtr =
		const_cast<uint8_t *>(appSegmentArray->data());
	const uint8_t *inputAppSegEnd = appSegmentArray->end();

	uint8_t *jpegRemainingAppSegPtr = nullptr;

	uint8_t *jpegPtr = jpegBlob.begin();
	uint8_t *jpegEnd = jpegBlob.begin() + jpegSize;
	const size_t bufferSize = jpegBlob.size();

	while (jpegPtr < jpegEnd) {
		size_t nextAppSegmentSize = appSegmentLength[inputAppSegNum] +
					    kJpegMarkerSize +
					    kJpegMetadataLengthSize;
		if (jpegSize + nextAppSegmentSize > bufferSize) {
			LOG(JPEG, Warning) << "Not enough JPEG buffer for "
					   << "remaining app segments.";
			return jpegSize;
		}

		std::stringstream sstream;
		sstream << " at " << std::hex << (jpegPtr - jpegBlob.begin());
		const std::string logSuffix = sstream.str();

		if (jpegPtr + kJpegMarkerSize > jpegEnd) {
			LOG(JPEG, Error) << "Incomplete marker" << logSuffix;
			return jpegSize;
		}

		uint16_t marker = parseWord(jpegPtr);
		switch (marker) {
		case kJpegSOF:
		case kJpegRST0:
		case kJpegRST1:
		case kJpegRST2:
		case kJpegRST3:
		case kJpegRST4:
		case kJpegRST5:
		case kJpegRST6:
		case kJpegRST7:
		case kJpegEOI:
			// Skip the marker as there's no payload.
			jpegPtr += kJpegMarkerSize;
			break;

		case kJpegSOF0:
		case kJpegSOF2:
		case kJpegDHT:
		case kJpegDQT:
		case kJpegDRI:
		case kJpegSOS:
			if (jpegRemainingAppSegPtr == nullptr &&
			    inputAppSegPtr != inputAppSegEnd) {
				// Maybe we'll have to insert all remaining
				// segment here.
				jpegRemainingAppSegPtr = jpegPtr;
			}

			if (jpegPtr + kJpegMarkerSize + kJpegMetadataLengthSize > jpegEnd) {
				LOG(JPEG, Error) << "Invalid JPEG header"
						 << logSuffix;
				return jpegSize;
			}
			jpegPtr += (kJpegMarkerSize + parseWord(jpegPtr + kJpegMarkerSize));
			break;

		case kJpegAPP0:
		case kJpegAPP1:
		case kJpegAPP2:
		case kJpegAPP3:
		case kJpegAPP4:
		case kJpegAPP5:
		case kJpegAPP6:
		case kJpegAPP7:
		case kJpegAPP8:
		case kJpegAPP9:
		case kJpegAPP10:
		case kJpegAPP11:
		case kJpegAPP12:
		case kJpegAPP13:
		case kJpegAPP14:
		case kJpegAPP15:
		case kJpegCOM: {
			if (jpegPtr + kJpegMarkerSize + kJpegMetadataLengthSize > jpegEnd) {
				LOG(JPEG, Error) << "Invalid JPEG header"
						 << logSuffix;
				return jpegSize;
			}
			size_t segmentSize =
				kJpegMarkerSize + parseWord(jpegPtr + kJpegMarkerSize);
			if (jpegPtr + segmentSize > jpegEnd) {
				LOG(JPEG, Error) << "Invalid JPEG header"
						 << logSuffix;
				return jpegSize;
			}

			int readAppSegmentIdx = marker - kJpegAPP0;

			if (readAppSegmentIdx >= inputAppSegNum) {
				if (readAppSegmentIdx == inputAppSegNum) {
					LOG(JPEG, Warning) << "Skipping APP" << readAppSegmentIdx
							   << " from metadata, prioritizing the one"
							   << " from encoder.";
				} else {
					// Insert BEFORE the already
					// existing app segment.
					size_t insertSegmentSize =
						appSegmentLength[inputAppSegNum] +
						kJpegMarkerSize +
						kJpegMetadataLengthSize;
					uint8_t *insertBegin = jpegPtr;
					jpegPtr += insertSegmentSize;

					// Move
					size_t moveSize = static_cast<size_t>(
						jpegEnd - insertBegin + 1);
					std::memmove(jpegPtr, insertBegin, moveSize);
					jpegSize += insertSegmentSize;
					jpegEnd += insertSegmentSize;

					// Copy the app segment in
					writeAppSegment(inputAppSegNum,
							insertBegin,
							inputAppSegPtr,
							appSegmentLength[inputAppSegNum]);

					inputAppSegPtr += appSegmentLength[inputAppSegNum];
					inputAppSegNum++;
				}

				for (; inputAppSegNum < 16; inputAppSegNum++) {
					if (appSegmentLength[inputAppSegNum] > 0) {
						break;
					}
				}
			}

			jpegPtr += segmentSize;
			break;
		}

		default:
			LOG(JPEG, Error) << "Invalid JPEG marker: 0x"
					 << std::hex << marker
					 << logSuffix;
			return jpegSize;
		}
		// Assuming that the APPn markers always appear before SOS.
		if (marker == kJpegSOS || marker == kJpegEOI) {
			break;
		}
	}

	if (inputAppSegPtr != inputAppSegEnd) {
		if (jpegRemainingAppSegPtr == nullptr) {
			LOG(JPEG, Error) << "Invalid JPEG format";
			return jpegSize;
		}

		size_t reservedAppSegSize = 0;
		int maxSegmentIdx = 0;
		for (int i = inputAppSegNum; i < 16; i++) {
			if (appSegmentLength[i] != 0) {
				reservedAppSegSize += appSegmentLength[i] +
						      kJpegMarkerSize +
						      kJpegMetadataLengthSize;
				if (jpegSize + reservedAppSegSize > bufferSize) {
					LOG(JPEG, Warning)
						<< "Not enough JPEG buffer for "
						<< "all app segments.";
					break;
				}
				maxSegmentIdx = i;
			}
		}

		size_t moveSize = static_cast<size_t>(jpegEnd - jpegRemainingAppSegPtr + 1);
		std::memmove(jpegRemainingAppSegPtr + reservedAppSegSize,
			     jpegRemainingAppSegPtr, moveSize);

		std::vector<uint8_t> headerBuffer(kJpegMarkerSize + kJpegMetadataLengthSize);
		for (; inputAppSegNum <= maxSegmentIdx; inputAppSegNum++) {
			if (appSegmentLength[inputAppSegNum] == 0) {
				continue;
			}
			writeAppSegment(inputAppSegNum, jpegRemainingAppSegPtr,
					inputAppSegPtr,
					appSegmentLength[inputAppSegNum]);
			inputAppSegPtr += appSegmentLength[inputAppSegNum];
			jpegRemainingAppSegPtr +=
				headerBuffer.size() +
				appSegmentLength[inputAppSegNum];
		}

		jpegSize += reservedAppSegSize;
	}

	return jpegSize;
}

void PostProcessorJpeg::process(StreamBuffer *streamBuffer)
{
	ASSERT(encoder_);

	const FrameBuffer &source = *streamBuffer->srcBuffer;
	CameraBuffer *destination = streamBuffer->dstBuffer.get();
	const std::optional<StreamBuffer::JpegExifMetadata> &jpegExifMetadata =
		streamBuffer->jpegExifMetadata;

	ASSERT(destination->numPlanes() == 1);
	ASSERT(jpegExifMetadata.has_value());

	const CameraMetadata &requestMetadata = streamBuffer->request->settings_;
	CameraMetadata *resultMetadata = streamBuffer->result->resultMetadata_.get();
	camera_metadata_ro_entry_t entry;
	int ret;

	/* Set EXIF metadata for various tags. */
	Exif exif;
	exif.setMake(cameraDevice_->maker());
	exif.setModel(cameraDevice_->model());

	ret = requestMetadata.getEntry(ANDROID_JPEG_ORIENTATION, &entry);

	const uint32_t jpegOrientation = ret ? *entry.data.i32 : 0;
	resultMetadata->addEntry(ANDROID_JPEG_ORIENTATION, jpegOrientation);
	exif.setOrientation(jpegOrientation);

	exif.setSize(streamSize_);
	/*
	 * We set the frame's EXIF timestamp as the time of encode.
	 * Since the precision we need for EXIF timestamp is only one
	 * second, it is good enough.
	 */
	exif.setTimestamp(std::time(nullptr), 0ms);

	/* Exif requires nsec for exposure time */
	exif.setExposureTime(jpegExifMetadata->sensorExposureTime * 1000);
	exif.setISO(jpegExifMetadata->sensorSensitivityISO);

	ret = requestMetadata.getEntry(ANDROID_LENS_APERTURE, &entry);
	if (ret)
		exif.setAperture(*entry.data.f);

	exif.setFlash(Exif::Flash::FlashNotPresent);
	exif.setWhiteBalance(Exif::WhiteBalance::Auto);

	exif.setFocalLength(jpegExifMetadata->lensFocalLength);

	ret = requestMetadata.getEntry(ANDROID_JPEG_GPS_TIMESTAMP, &entry);
	if (ret) {
		exif.setGPSDateTimestamp(*entry.data.i64);
		resultMetadata->addEntry(ANDROID_JPEG_GPS_TIMESTAMP,
					 *entry.data.i64);
	}

	ret = requestMetadata.getEntry(ANDROID_JPEG_THUMBNAIL_SIZE, &entry);
	if (ret) {
		const int32_t *data = entry.data.i32;
		Size thumbnailSize = { static_cast<uint32_t>(data[0]),
				       static_cast<uint32_t>(data[1]) };

		ret = requestMetadata.getEntry(ANDROID_JPEG_THUMBNAIL_QUALITY, &entry);
		uint8_t quality = ret ? *entry.data.u8 : 95;
		resultMetadata->addEntry(ANDROID_JPEG_THUMBNAIL_QUALITY, quality);

		if (thumbnailSize != Size(0, 0)) {
			std::vector<unsigned char> thumbnail;
			generateThumbnail(source, thumbnailSize, quality, &thumbnail);
			if (!thumbnail.empty())
				exif.setThumbnail(std::move(thumbnail), Exif::Compression::JPEG);
		}

		resultMetadata->addEntry(ANDROID_JPEG_THUMBNAIL_SIZE, data, 2);
	}

	ret = requestMetadata.getEntry(ANDROID_JPEG_GPS_COORDINATES, &entry);
	if (ret) {
		exif.setGPSLocation(entry.data.d);
		resultMetadata->addEntry(ANDROID_JPEG_GPS_COORDINATES,
					 entry.data.d, 3);
	}

	ret = requestMetadata.getEntry(ANDROID_JPEG_GPS_PROCESSING_METHOD, &entry);
	if (ret) {
		std::string method(entry.data.u8, entry.data.u8 + entry.count);
		exif.setGPSMethod(method);
		resultMetadata->addEntry(ANDROID_JPEG_GPS_PROCESSING_METHOD,
					 entry.data.u8, entry.count);
	}

	ret = requestMetadata.getEntry(ANDROID_CONTROL_AE_MODE, &entry);
	if (ret) {
		if (entry.data.u8[0] == ANDROID_CONTROL_AE_MODE_OFF) {
			exif.setShort(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM, 1);
			exif.setShort(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, 2);
		} else {
			exif.setShort(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM, 2);
			exif.setShort(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, 1);
		}
	}
	ret = requestMetadata.getEntry(ANDROID_SENSOR_EXPOSURE_TIME, &entry);
	if (ret && entry.data.i64[0] > 0) {
		// us -> second
		double tv = static_cast<double>(entry.data.i64[0]) / 1000000;
		// exposure time -> shutter speed
		tv = -1.0 * (log(tv) / log(2.0));
		ExifRational rational = { static_cast<ExifLong>(tv * 65536), 65536};
		exif.setRational(EXIF_IFD_EXIF, EXIF_TAG_SHUTTER_SPEED_VALUE, rational);
	} else {
		ExifRational rational = { static_cast<ExifLong>(0), 1};
		exif.setRational(EXIF_IFD_EXIF, EXIF_TAG_SHUTTER_SPEED_VALUE, rational);
	}


	if (exif.generate() != 0)
		LOG(JPEG, Error) << "Failed to generate valid EXIF data";

	ret = requestMetadata.getEntry(ANDROID_JPEG_QUALITY, &entry);
	const uint8_t quality = ret ? *entry.data.u8 : 95;
	resultMetadata->addEntry(ANDROID_JPEG_QUALITY, quality);

	int jpeg_size = encoder_->encode(streamBuffer, exif.data(), quality);
	if (jpeg_size < 0) {
		LOG(JPEG, Error) << "Failed to encode stream image";
		processComplete.emit(streamBuffer, PostProcessor::Status::Error);
		return;
	}

	jpeg_size = insertAppSegments(destination->plane(0), jpeg_size,
				      streamBuffer->request->request_->metadata());

	/* Fill in the JPEG blob header. */
	uint8_t *resultPtr = destination->plane(0).data()
			   + destination->jpegBufferSize(cameraDevice_->maxJpegBufferSize())
			   - sizeof(struct camera3_jpeg_blob);
	auto *blob = reinterpret_cast<struct camera3_jpeg_blob *>(resultPtr);
	blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
	blob->jpeg_size = jpeg_size;

	/* Update the JPEG result Metadata. */
	resultMetadata->addEntry(ANDROID_JPEG_SIZE, jpeg_size);
	processComplete.emit(streamBuffer, PostProcessor::Status::Success);
}

void PostProcessorJpeg::writeAppSegment(int appSegmentIdx, uint8_t *dest,
					uint8_t *src, size_t size)
{
	LOG(JPEG, Info) << "Write APP" << appSegmentIdx;
	std::vector<uint8_t> header(kJpegMarkerSize + kJpegMetadataLengthSize);
	writeTwoBytes(header.data(), kJpegAPP0 + appSegmentIdx);
	writeTwoBytes(header.data() + kJpegMarkerSize,
		      kJpegMetadataLengthSize + size);
	std::memcpy(dest, header.data(), header.size());
	std::memcpy(dest + header.size(), src, size);
}
