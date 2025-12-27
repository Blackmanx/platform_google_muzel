/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_NDD_INDD_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_NDD_INDD_H_

#include <mtkcam-interfaces/utils/metadata/IMetadata.h>
#include <mtkcam-interfaces/utils/ndd/ndd_autogen_def.h>
#include <mtkcam-interfaces/utils/ndd/ndd_def.h>

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;

// Usage
// for simple array  : NDD_DUMP(eCategory, eModule, NddData, void* buffer, size)
// for IImageBuffer : NDD_DUMP(eCategory, eModule, NddData, IImageBuffer*)

#define NDD_DUMP(...)                                                        \
  do {                                                                       \
    NSCam::TuningUtils::INdd* ins = NSCam::TuningUtils::INdd::getInstance(); \
    ins->submit_request(__VA_ARGS__);                                        \
  } while (0);

#define FILE_PATH_SIZE 512

namespace mcam {
  struct IImageBuffer;
}  // namespace mcam

namespace NSCam {
namespace TuningUtils {

/* Design for being used in service.cpp only
** since Ndd dumping threads' lifecycle are aligned with camerahalserver
**/

class NddInitializerImp;

class NddInitializer {
 public:
  NddInitializer();
  ~NddInitializer();

 private:
  std::unique_ptr<NddInitializerImp> ndd_initializer;
};

class INdd {
 public:
  /**
   * @brief Notify NDD that the camera is on
   *
   * @param[in] sensor_id The set of camera that is active
   * @param[in] uniqueKey The uniqueKey of the pipeline, a.k.a. 9-digit Code
   */
  virtual void stream_on(vector<int32_t> sensor_id, uint32_t uniqueKey) = 0;

  /**
   * @brief Notify NDD that the camera is off
   *
   * @param[in] sensor_id The set of camera that is being shut down
   */
  virtual void stream_off(vector<int32_t> sensor_id) = 0;

  /**
   * @brief Notify NDD the start of the certain frame
   *
   * @param[in] sensor_idx The sensor id of the informed frame
   * @param[in] frm_no The frame number of the informed frme
   */
  virtual void frame_begin(int32_t sensor_idx, int32_t frm_no, bool is_mw_process) = 0;

  /**
   * @brief Notify NDD the end of the certain frame
   *
   * @param[in] sensor_idx The sensor id of the informed frame
   * @param[in] frm_no The frame number of the informed frme
   */
  virtual void frame_end(int32_t sensor_idx, int32_t frm_no) = 0;

  /**
   * @brief Query NDD about file name along with file path
   * under given conditions
   *
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @param[in] data The corresponding NddData of dump data
   * @param[out] output The target file path
   * @param[in] file The file of issuing an inquiry
   * @param[in] line The line number of the inquiry file
   * @return int 0 means successfully fetch the file name, while
   * any other integer means error respectively
   */
  virtual int query_file_path(eCategory category,
                              eModule module,
                              NddData& data,
                              char* output,
                              const char* file = __builtin_FILE(),
                              int line = __builtin_LINE()) = 0;

  /**
   * @brief Submit data to NDD as conventional array type for dump
   *
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @param[in] data The corresponding NddData of dump data
   * @param[in] buffer The pointer to the dump data's initial address
   * @param[in] size The size of dump data
   * @param[in] file The file of issuing an dump request
   * @param[in] line The line number of the request file
   */
  virtual void submit_request(eCategory category,
                              eModule module,
                              NddData& data,
                              const void* buffer,
                              int size,
                              const char* file = __builtin_FILE(),
                              int line = __builtin_LINE()) = 0;

  /**
   * @brief Submit data to NDD as ImageBuffer type for dump
   *
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @param[in] data The corresponding NddData of dump data
   * @param[in] buffer The pointer to the dump data's ImageBuffer address
   * @param[in] file The file of issuing an dump request
   * @param[in] line The line number of the request file
   */
  virtual void submit_request(eCategory category,
                              eModule module,
                              NddData& data,
                              mcam::IImageBuffer* buffer,
                              const char* file = __builtin_FILE(),
                              int line = __builtin_LINE()) = 0;

  /**
   * @brief Query if NDD is enabled
   *
   * @return int 0 means disabled, while any other positive integers indicates
   * how many NDD threads are on the go
   */
  virtual int is_enable_ndd() = 0;

  /**
   * @brief Query if the data with given conditions combination is allowed to
   * dump
   *
   * @param[in] data The corresponding NddData of dump data
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @return int 0 means not permitted, while 1 means permitted
   */
  virtual int is_enable_dump(const NddData& data,
                             eCategory category,
                             eModule module) = 0;

  /**
   * @brief middleware notify the NDD utils which module is disabled, it's mean
   * middleware don't call submit_request API with the disabled module.
   *
   * @param[in] data The corresponding NddData of dump data
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @param[in] frm_no The frame number of the informed frme
   */
  virtual void notify_disable(NddData data,
                              eCategory category,
                              eModule module) = 0;

  /**
   * @brief Replace the timestamp and request id of latter metadata
   * with the ones in former
   *
   * @param[in] src The source Hal Metadata
   * @param[out] dest The target Hal Metadata
   * @return int 0 means successfully, while -1 means either querying
   * or updating metadata encounters fail
   */
  virtual int align_uniquekey(const IMetadata& src, IMetadata& dest) = 0;

  /**
   * @brief Compose 944 code based on the input
   *
   * @param[out] dest Hal metadata that will be updated 944 code
   * @param[in] timestamp Device timestamp
   * @param[in] request_id Request number
   * @param[in] frame_id Frame number
   * @return int 0 means successfully updated
   */
  virtual int update_uniquekey(IMetadata& dest,
                               int32_t timestamp,
                               int32_t request_id,
                               int32_t frame_id) = 0;
  /**
   * @brief Query NDD information from HalMetadata
   *
   * @param[in] src Hal metadata
   * @param[out] timestamp Device time stamp
   * @param[out] request_id Request number
   * @param[out] frame_id Frame number
   * @return int Error check. 0 means return successfully, while -1 means any
   * of these three above is not in the given metadata
   */
  virtual int query_uniquekey(const IMetadata& src,
                              int32_t& timestamp,
                              int32_t& request_id,
                              int32_t& frame_id) = 0;
  /**
   * @brief Query NDD information from ndd key
   *
   * @param[in] nddKey key generated by NDD utils
   * @param[out] timestamp Device time stamp
   * @param[out] request_id Request number
   * @param[out] frame_id Frame number
   * @return int Error check. 0 means return successfully.
   */
  virtual int query_uniquekey(const int64_t nddKey,
                              int32_t& timestamp,
                              int32_t& request_id,
                              int32_t& frame_id) = 0;

  /**
   * @brief Query some of crucial info from source metadata
   *
   * @param[in] src The source Hal metadata
   * @param[out] category The category parsed from metadata
   * @param[out] data The NddData whose 944 code and feature will be updated
   * @return int 0 means success, while -1 means any of above failed
   */
  virtual int query_ndd_info(const IMetadata& src,
                             eCategory& category,
                             NddData& data) = 0;

  /**
   * @brief Query format or selector after user set port-selector with adb cmd
   *
   * @param[out] output The NddFormat whose format or selector will be updated
   * @return it means user need to force format or not
   */
  virtual bool force_buffer_format(NddFormat& output) = 0;

  /**
   * @brief middeware want to force the order of frames at dumpin cfg
   *
   * @param[in] sensor_idx The sensor id of the informed frame
   * @param[in] order is the list of frame id.
   */
  virtual void force_frame_order(int32_t sensor_id, vector<uint32_t> order) = 0;

  /**
   * @brief push data to hal metadata for dump
   *
   * @param[in] category The category of dump data
   * @param[in] module The module name of dump data
   * @param[in] data The corresponding NddData of dump data
   * @param[in] buffer The pointer to the dump data's ImageBuffer address
   * @param[out] The source Hal Metadata
   * @param[in] file The file of issuing an dump request
   * @param[in] line The line number of the request file
   */
  virtual void push_buffer(eCategory category,
                           eModule module,
                           NddData& data,
                           void* buffer,
                           int size,
                           IMetadata& output,
                           const char* file = __builtin_FILE(),
                           int line = __builtin_LINE()) = 0;

  /**
   * @brief dump data from hal metadata
   *
   * @param[in] the source Hal Metadata
   */
  virtual void dump_buffer(const IMetadata& input) = 0;


  virtual bool need_dump(int sensor_id, int frame_id) = 0;

 protected:
  INdd() {}
  virtual ~INdd() {}

 public:
  static INdd* getInstance();
  INdd(const INdd&) = delete;
  INdd& operator=(const INdd&) = delete;
};
}  // namespace TuningUtils
}  // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_UTILS_NDD_INDD_H_
