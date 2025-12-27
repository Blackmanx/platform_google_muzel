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

#ifndef INCLUDE_MTKCAM_HALIF_UTILS_METADATA_1_X_IMETADATA_H_
#define INCLUDE_MTKCAM_HALIF_UTILS_METADATA_1_X_IMETADATA_H_

/******************************************************************************
 * Metadata V2 header files
 ******************************************************************************/
#include <string>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include <functional>

#include "mtkcam-halif/def/BasicTypes.h"
#include "mtkcam-halif/def/BuiltinTypes.h"
#include "mtkcam-halif/def/UITypes.h"
#include "mtkcam-halif/def/TypeManip.h"
#include "mtkcam-halif/def/Errors.h"
//
#define MTKCAM_METADATA_V2

namespace NSCam {
enum {  // for MERROR return value
  METADATA_OK = 0,
  METADATA_FAIL = -1
};
/**
 * IMetadata class is a (key, value array) container, which have those benefit:
 * - one IMetadata object can treat as a handle, almost zero constructor cost
 * - light weight clone/assign
 * - modification with copy-on-write
 * - thread safe protection for multi thread access same IMetadata object
 *
 * @note Returned MERROR type will be
 *       - METADATA_OK = 0, means success
 *       - METADATA_FAIL = -1, means error
 *
 * @note About size_t and ssize_t type, default will use size_t.
 *       ssize_t only use for return error (METADATA_FAIL)
 *
 *
 */
class IMetadata {
 public:
  class BurstEntry;

 protected:
  class Storage;
 private:
  /******************************************************************************
  * Forward declaration
  ******************************************************************************/
  class Content;  // Entry internal storage
  struct ContentLayout;
  struct ContentRawData;
  typedef std::shared_ptr<Content> ContentSP;
  typedef std::shared_ptr<Storage> StorageSP;
  typedef std::vector<MUINT8> BasicData;
  typedef std::shared_ptr<BasicData> BasicDataSP;
  typedef std::vector<StorageSP> VecStorage;
  typedef std::shared_ptr<VecStorage> VecStorageSP;
  typedef std::vector<BasicDataSP> VecBasicData;
  typedef std::shared_ptr<VecBasicData> VecBasicDataSP;

 public:
  /**
   * BurstReader is a function prototype which accept const BurstEntry for entry
   * query and returning query result.
   * @sa BurstEntry
   */
  using BurstReader = std::function<int(const BurstEntry&)>;
  /**
   * BurstWriter is a function prototype which accept BurstEntry for updating
   * the entry value
   * @sa BurstEntry
   */
  using BurstWriter = std::function<void(BurstEntry&)>;

 public:
  /******************************************************************************
  * (Public) IMetadata HAL Interface
  ******************************************************************************/
  class IEntry;
  class Memory;
  /**
   * Type define IMetadata::Tag_t as key type to insert IMetadata
   */
  typedef MUINT32 Tag_t;

  /**
   * Constructor an empty IMetadata
   */
  IMetadata();

  /**
   * Constructor an IMetadata from other IMetadata object
   * @param other the source object for clone
   */
  IMetadata(IMetadata const& other);

  /**
   * Destructor
   */
  ~IMetadata();

  /**
   * Copy from other IMetadata object
   * It use share_ptr to share same data of other IMetadata object
   * @param other other data
   * @return "this" object reference
   */
  IMetadata& operator=(IMetadata const& other);

  /**
   * Append data from other IMetadata object
   * same tag entry will be replaced
   * @param other other data
   * @return "this" object reference
   */
  IMetadata& operator+=(IMetadata const& other);

  /**
   * Modify current object with append other IMetadata object
   * @param other other data
   * @return clone of "this" object
   */
  IMetadata operator+(IMetadata const& other);

  /**
   * Check to see whether it is empty (no entries) or not.
   * @return MTRUE if no entries
   */
  MBOOL isEmpty() const;

  /**
   * Return the number of entries.
   * @return the number of entries
   */
  MUINT count() const;

  /**
   * Clear all entries.
   */
  MVOID clear();

  /**
   * Delete an entry by tag.
   * @param tag remove tag entry
   * @return METADATA_OK when removed successfully,
   *         METADATA_FAIL if no entry to remove.
   */
  MERROR remove(Tag_t tag);

  /**
   * Deprecated API
   * Sort all entries for faster find.
   * Useless due to current implement always keep sorted
   * @return always return METADATA_OK
   */
  MERROR sort();

  /**
   * Update metadata entry. if tag already exists, data will be replaced
   *
   * @param tag Tag key
   * @param entry entry to store
   * @return always return METADATA_OK
   */
  MERROR update(Tag_t tag, IEntry const& entry);

  /**
   * Update metadata entry with entry's tag. if tag already exists, data will be replaced
   *
   * @param entry entry to store
   * @return METADATA_OK for updated successfully, METADATA_FAIL if update fail.
   *         Usually update fail due to the tag of entry is BAD_TAG
   */
  MERROR update(IEntry const& entry);  // use entry's tag value

  /**
   * Query an IEntry with tag key
   *
   * @param tag tag key for search
   * @param isTakeAway remove entry from IMetadata after return
   * @return the result of IEntry object
   */
  IEntry entryFor(Tag_t tag, MBOOL isTakeAway = MFALSE) const;

  /**
   * Query an IEntry using position
   *
   * @param index the position of IEntry , index should < Count()
   * @param isTakeAway remove entry from IMetadata after return
   * @return the result of IEntry object
   */
  IEntry entryAt(MUINT index) const;

  /**
   * Burst read operations of the entries in the given IMetadata container.
   * Since IMetadata is claimed as a thread-safe class, frequently accessing
   * would cause higher CPU usage.
   * This burst read API provide ability to execute multiple read operations in
   * a single lock/unlock protection.
   *
   * @param f User gives query function definition for querying lots of tag keys
   *        and update to its structure.
   * @sa IMetadata::BurstReader
   * @return the result of burstRead
   */
  int burstRead(const BurstReader& f) const;

  /**
   * Burst write operations of the entries in the given IMetadata container.
   * Since IMetadata is claimed as a thread-safe class, frequently accessing
   * would cause higher CPU usage.
   * This burst write API provide ability to execute multiple write operations in
   * a single lock/unlock protection.
   *
   * @param f User gives update function definition for updating lots of tag keys
   *        to Storage.
   * @sa IMetadata::BurstWriter
   */
  void burstWrite(const BurstWriter& f);

  /**
   * Take metadata entry by tag. After invoked this method, the metadata enty of
   * tag in metadata will be removed. Note: Without all element copy.
   * Complexity: O(log N)
   * @param tag query the IEntry object with tag value and remove it from IMetadata
   * @return the result of IEntry object
   */
  IEntry takeEntryFor(Tag_t tag);

  /**
   * Serialize whole IMetadata data into bytes array.
   * No more pointer information store inside the bytes array.
   * This API performance may slow due to traversing whole IMetadata content
   * and copy bytes.
   *
   * @param buf bytes array start address for output
   * @param buf_size the length of bytes array
   * @param witeList whitelist table, an tag arrays
   * @param witeListSize the length of whitelist array
   * - Enable whitelist feature only When whitelist != nullptr && WSize != -1
   * - Whitelist array MUST sorted in ASCENDING order
   * - Whitelist only apply at level 1 of IMetadata. not apply the subtree of metadata
   * - Whitelist filter search cost: O(N)
   * @retval buf the flatten result data
   * @return the total bytes of the flatten result data
   */
  ssize_t flatten(void* buf,
                  size_t buf_size,
                  const Tag_t* whitelist = nullptr,
                  const ssize_t whitelistSize = -1) const;

  /**
   * Un-serialize the bytes array to replace current data
   * "this" IMetadata object will be replaced by unflatten result
   *
   * @param buf bytes array start address
   * @param buf_size the length of bytes array
   * @return the used length of flatten data
   *
   */
  ssize_t unflatten(void* buf, size_t buf_size);

  /**
   * traverse whole IMetadata data to calculate the byte size of flatten
   * data needed.
   *
   * @param whitelist whitelist table, an tag arrays
   * @param whitelistSize the length of whitelist array
   * - Enable whitelist feature only When whitelist != nullptr && WSize != -1
   * - Whitelist array MUST sorted in ASCENDING order
   * - Whitelist only apply at level 1 of IMetadata. not apply the subtree of metadata
   * - Whitelist filter search cost: O(N)
   * @return the total bytes of flatten data needed.
   */
  size_t flattenSize(const Tag_t* whitelist = nullptr, const ssize_t whitelistSize = -1) const;

  /**
   * [Debug API]. Dump whole IMetadata into log
   * @param layer should assign 0 for start dump
   * @param forceOutput force dump() API to write to log without consider the log level
   */
  void dump(int layer = 0, bool forceOutput = false);

  /**
   * [Helper API] to set metadata with given tag and value.
   * Add a pair a tag with its value into metadata (and replace the one that is
   * there)
   *
   * @code
   *    [software flow]
   *    IMetadata::IEntry entry(tag);
   *    entry.push_back(val, Type2Type<T>());
   *    return metadata->update(tag, entry);
   * @endcode
   *
   * @param metadata [in,out]    The metadata to be updated
   * @param tag [in]             The tag to update
   * @param val [in]             The value to update
   * @return                     Entry is set or not (METADATA_OK/METADATA_FAIL)
   * @retval                     OK on success
   * @retval                     INVALID_OPERATION if metadata is null
   * @retval                     BAD_INDEX if out of range
   * @retval                     NO_MEMORY if out of memory
   */
  template <typename T>
  static MERROR setEntry(IMetadata* metadata, Tag_t const tag, T const& val);

  /**
   * [Helper API] to get metadata with given tag and value.
   * If the tag doesn't exist, this function returns false.
   *
   * @code
   *    [software flow]
   *    IMetadata::IEntry entry = metadata->entryFor(tag);
   *    if (entry.count() > index)
   *      val = entry.itemAt(index, Type2Type<T>());
   * @endcode
   *
   * @param metadata [in]    The constant pointer of IMetadata to look up
   * @param tag [in]         The tag to get
   * @param val [out]        Call by reference output if found
   * @param index [in]       Index of item in Entry you want to look up. Default
   * value is 0.
   * @return                 true if the corresponding entry exists
   */
  template <typename T>
  static bool getEntry(const IMetadata* metadata, Tag_t const tag, T& val,
                       size_t index = 0);

  /******************************************************************************
  * (Public) IMetadata::IEntry HAL Interface
  ******************************************************************************/
  /**
   * IMetadata::IEntry class is a std::vector object to handle those metadata type
   * - TYPE_MUINT8 Unsigned 8-bit integer (uint8_t)
   * - TYPE_MINT32 Signed 32-bit integer (int32_t)
   * - TYPE_MFLOAT 32-bit float (float)
   * - TYPE_MINT64 Signed 64-bit integer (int64_t)
   * - TYPE_MDOUBLE 64-bit float (double)
   * - TYPE_MRational A 64-bit fraction  (camera_metadata_rational_t)
   * - TYPE_MPoint Reference to MTK's MPoint data type
   * - TYPE_MSize Reference to MTK's MSize data type
   * - TYPE_MRect Reference to MTK's MRect data type
   * - TYPE_IMetadata Reference to MTK's IMetadata data type
   * - TYPE_Memory Reference to MTK's IMetadata::Memory data type
   */
  class IEntry {
   public:
    /**
     * enum to identify illegal tag
     */
    enum { BAD_TAG = -1U };

    /**
     * Constructor to create an entry
     *
     * @param tag specify the tag value as key for insert to IMetadata.
     * default value is IMetadata::IEntry::BAD_TAG = -1
     */
    explicit IEntry(Tag_t tag = BAD_TAG);

    /**
     * Copy constructor to clone an IEntry, it will us share_ptr to share
     * same data without copy.
     * When entry be modified, copy-on-write will be processed.
     *
     * @param other source for clone
     */
    IEntry(IEntry const& other);

    /**
     * Copy operation to copy an IEntry, it will us share_ptr to share
     * same data without copy.
     * When entry be modified, copy-on-write will be processed.
     *
     * @param other source for copy
     */
    IEntry& operator=(IEntry const& other);

    /**
     * Destructor
     */
    ~IEntry();

    /**
     * Return the tag of this IEntry
     * @return the tag of this IEntry
     */
    Tag_t tag() const;

    /**
     * Return the data type of this IEntry
     * @return the type:
     * - TYPE_MUINT8
     * - TYPE_MINT32
     * - TYPE_MFLOAT
     * - TYPE_MINT64
     * - TYPE_MDOUBLE
     * - TYPE_MRational
     * - TYPE_MPoint
     * - TYPE_MSize
     * - TYPE_MRect
     * - TYPE_IMetadata
     * - TYPE_Memory
     */
    MINT32 type() const;

    /**
     * Return the start address of IEntry's data storage
     * IEntry's data storage is an continuos BYTES array.
     * Use reinterpret_cast<> to correspond data type
     *
     * For example:
     *     if IEntry's type is MRational, you can access the MRational array with
     * @code
     *    auto array = reinterpret_cast<MRational*> IEntry.data();
     * @endcode
     *
     * @return the start address of data
     */
    const void* data() const;

    /**
     * Check to see whether it is empty (no items) or not
     * @return MTRUE means no items store in this IEntry object
     */
    MBOOL isEmpty() const;

    /**
     * Return the number of items
     * @return the number of items
     */
    MUINT count() const;

    /**
     * Clear all items
     */
    MVOID clear();

    /**
     * Delete an item at a given index
     * @param index the index for remove
     * @return return the result for remove operation
     *    - METADATA_OK succeed
     *    - METADATA_FAIL failed
     */
    MERROR removeAt(MUINT index);

    /**
     * Append Type T data at the end of array
     * @param item source data for append
     * @param Type2Type<Type T> to specify data type
     */
    template <typename T,typename U>
    MVOID push_back(T const& item, Type2Type<U>);

    /**
     * Append Type T data array at the end of array
     * @param array source data for append
     * @param size  the number of array (NOT bytes)
     * @param Type2Type<Type T> to specify data type
     */
    template <typename T,typename U>
    MVOID push_back(T const* array, size_t size, Type2Type<U>);

    /**
     * Replace Type T data at the position of array
     * @param index the position of array
     * @param item source data for replace
     * @param Type2Type<Type T> to specify data type
     */
    template <typename T,typename U>
    MVOID replaceItemAt(MUINT index, T const& item, Type2Type<U>);

    /**
     * Replace Type T data array at the position of array
     * @param index the position of array
     * @param array source data for replace
     * @param size  the number of array (NOT bytes)
     * @param Type2Type<Type T> to specify data type
     */
    template <typename T,typename U>
    MVOID replaceItemAt(MUINT index, T const* array, size_t size, Type2Type<U>);

    /**
     * Return Type T data from the position of array
     * @param index the position of array
     * @param Type2Type<Type T> to specify data type
     * @return return the result object
     */
    template <typename T>
    T itemAt(MUINT index, Type2Type<T>) const;

    /**
     * Return Type T data array from the position of array
     * @param index the position of array
     * @param size  the number of array to retrieve (NOT bytes)
     * @param Type2Type<Type T> to specify data type
     * @return MTRUE means query data success
     */
    template <typename T>
    MBOOL itemAt(MUINT index, T* array, size_t size, Type2Type<T>) const;

    /**
     * Return IMetadata::Memory data array from the position of array
     * @param index the position of array
     * @param size  the number of array to retrieve (NOT bytes)
     * @param Type2Type<IMetadata::Memory> to specify data type
     * @return MTRUE means query data success
     */
    template<>
    IMetadata::Memory itemAt(MUINT index, Type2Type<IMetadata::Memory>) const;

    /**
     * Return IMetadata data array from the position of array
     * @param index the position of array
     * @param size  the number of array to retrieve (NOT bytes)
     * @param Type2Type<IMetadata> to specify data type
     * @return MTRUE means query data success
     */
    template<>
    IMetadata itemAt(MUINT index, Type2Type<IMetadata>) const;

    /**
     * [Helper API]
     * Get index of target value in Entry
     * @param entry IEntry object for query
     * @param target the query tag
     * @return return the index of data. -1 means not found
     */
    template <typename T>
    static int indexOf(IEntry& entry, const T& target);

  /******************************************************************************
  * IEntry Private Session BEGIN
  ******************************************************************************/
  private:
    /**
     * Data structure inside the IEntry
     */
    Tag_t mTag;
    mutable ContentSP mContentPtr;
    mutable std::mutex mLock;
     /**
     * Copy constructor to clone an IEntry, it will us share_ptr to share
     * same data without copy.
     * When entry be modified, copy-on-write will be processed.
     *
     * @param source IEntry's contentPtr
     */
    IEntry(Tag_t tag, std::shared_ptr<IMetadata::Content> contentPtr);
     /**
     * IMetadata query API can access IEntry private members
     * - IEntry(Tag_t tag, std::shared_ptr<IMetadata::Content> contentPtr)
     */
    friend IMetadata::IEntry IMetadata::entryFor(Tag_t tag, MBOOL isTakeAway) const;
    friend IMetadata::IEntry IMetadata::entryAt(MUINT index) const;
    friend MERROR IMetadata::update(Tag_t tag, IMetadata::IEntry const& entry);
    // _Content use MINT index type for -1 do append
    ContentLayout* _ContentHeader(const ContentSP& sp) const;
    void _ContentResize(const ContentSP& sp, size_t size);
    void _ContentResize(const ContentSP& sp,
                               size_t offset,
                               size_t oldItemSize,
                               size_t newItemSize);
    ContentSP _ContentNew();                        // new a Content
    ContentSP _ContentClone(const ContentSP& src);  // clone a Content
    MBOOL _ContentIsReadOnly(const ContentSP& sp) const;
    void _ContentSetReadOnly(const ContentSP& sp) const;
    MINT32 _ContentType(const ContentSP& sp) const;
    MUINT32 _ContentCount(const ContentSP& sp) const;
    size_t _ContentTypeSize(MINT32 type) const;
    MBOOL _ContentRemove(ContentSP& sp, MINT index);
    MBOOL _ContentClear(ContentSP& sp);
    MBOOL _ContentUpdate(ContentSP& sp,
                                MINT index,
                                const void* array,
                                size_t size,
                                MINT32 type);
    const void* _ContentGetBasicData(ContentSP& sp,
                                            MINT index,
                                            MINT32 type) const;
    StorageSP _ContentGetIMetadata(ContentSP& sp,
                                          MINT index,
                                          MINT32 type) const;
    BasicDataSP _ContentGetMemory(ContentSP& sp,
                                         MINT index,
                                         MINT32 type) const;
    ContentRawData* _ContentRawDataByOffset(const ContentSP& sp,
                                                   size_t offset) const;
    size_t _ContentRawDataSize(const ContentRawData* p) const;
    size_t _ContentRawDataSize(const size_t size) const;
    ContentSP getContentSP() const;
    void _SwitchWritable();
  };

  /******************************************************************************
  * (Private) IMetadata::Contnt
  ******************************************************************************/
 private:
  class Content {
    MINT32 mType;
    MUINT32 mCount;
    union {
      MUINT8 v0[sizeof(MINT8)];
      MUINT8 v1[sizeof(MINT32)];
      MUINT8 v2[sizeof(MFLOAT)];
      MUINT8 v3[sizeof(MINT64)];
      MUINT8 v4[sizeof(MDOUBLE)];
      MUINT8 v5[sizeof(MRational)];
      MUINT8 v6[sizeof(MPoint)];
      MUINT8 v7[sizeof(MSize)];
      MUINT8 v8[sizeof(MRect)];
      // immediate basic storage without array allocate
    } mImmData;

   private:
    BasicDataSP mBasicDataSP;
    VecStorageSP mMetaSP;
    VecBasicDataSP mMemSP;
    MBOOL mReadOnly;

    ssize_t _typeSize() const;

   public:
    Content();
    ~Content();
    Content(const void* flattenSrc, size_t size, MBOOL& isError);
    Content(Content& src);

    size_t flattenSize();
    ssize_t flatten(void* dest, size_t destSize);
    ssize_t unflatten(const void* src, size_t srcSize);
    MBOOL isReadOnly() { return mReadOnly; }
    void setReadOnly() { mReadOnly = MTRUE; }
    MINT32 getType() { return mType; }
    MUINT32 getCount() { return mCount; }
    MBOOL remove(MINT index);
    MBOOL clear();
    MBOOL update(MINT index, const void* array, size_t size, MINT32 type);
    const void* getBasicData(MINT index, MINT32 type) const;
    StorageSP getMetadata(MINT index, MINT32 type) const;
    BasicDataSP getMemory(MINT index, MINT32 type) const;

   public:
    // global static usage
    static size_t mStatNum;   // obj numbers
    static size_t mStatSize;  // bytes
  };
  /******************************************************************************
  * (Protected) IMetadata::Storage
  ******************************************************************************/
 protected:
  class Storage {
    struct Item {
      Tag_t tag;
      ContentSP contentSP;
      bool operator<(const Item& rhs) const { return tag < rhs.tag; }
      bool operator<(const Tag_t& rtag) const { return tag < rtag; }
    };

    typedef std::vector<Item> ItemTable;
    MUINT32 mGuardPatternBegin = 0x12345678;
    mutable ItemTable mItems;  // keep sorted
    MBOOL mReadOnly;
    MUINT32 mGuardPatternEnd = 0x87654321;

   public:
    Storage();
    ~Storage();
    void setReadOnly();
    MBOOL isReadOnly() const;
    MBOOL inWhitelist(const Tag_t tag,
                      const Tag_t* whitelist,
                      const ssize_t whitelistSize,
                      size_t& pos) const;
    ssize_t unflatten(const void* src, const size_t srcSize);
    ssize_t flatten(void* dest,
                    const size_t destSize,
                    const Tag_t* whitelist,
                    const ssize_t whitelistSize) const;
    size_t flattenSize(const Tag_t* whitelist, const ssize_t whitelistSize) const;
    IMetadata::StorageSP clone() const;
    void update(Tag_t tag, ContentSP contentSP);
    void update(const IMetadata::StorageSP& sp);
    inline size_t count() const {
      return mItems.size();
    }
    MBOOL remove(Tag_t tag);
    MBOOL getContentSP(Tag_t tag,
                       IMetadata::ContentSP& retContentSP,
                       MBOOL isTakeAway = MFALSE) const;
    MBOOL getContentSPByIndex(size_t index,
                              IMetadata::ContentSP& retContentSP,
                              Tag_t& retTag) const;
    MBOOL getTagByIndex(size_t index, Tag_t& retTag) const;
    /**
     *Read the value of the given entry with tag `tag`.
     *  @param tag Tag
     *  @param val The Type T value would be queried from entry
     *  @param index The array index of the given entry
     *  @return The result of entry read, `0` indicates to succeed,
         otherwise failed.
     */
    template <typename T>
    int readEntry(Tag_t tag, T& val, MUINT index = 0) const;

    /**
     *Read values from the given entry of tag `tag`.
     *  @param tag Tag
     *  @param vals The vector will be resized to the values count
     *   of the given entry to store the results.
     *  @return The result of entry read, `0` indicates to succeed,
     *   otherwise failed.
     */
    template <typename T>
    int readEntries(Tag_t tag, std::vector<T>& vals) const;

    /**
     *Write an entry.
     *  @param tag Tag
     *  @param val The Type T value would be updated to an entry
     *  @param index The array index of the given entry
     */
    template <typename T>
    void writeEntry(Tag_t tag, const T& val, MUINT index = -1);

    /**
     *Write an entry with continuous memory chunk data.
     * @param tag Tag
     * @param array The Type T of source data would be updated to an entry
     * @param size Total size of source data
     * @param index The array index of the given entry
     */
    template <typename T>
    void writeEntries(Tag_t tag, const T* array, size_t size, MUINT index = -1);

    template <typename T>
      T read(MUINT index,
          Type2Type<T> type,
          IMetadata::ContentSP& content) const;

    template <>
      IMetadata::Memory read(MUINT index,
        Type2Type<IMetadata::Memory> type,
        IMetadata::ContentSP& content) const;

    template<>
      IMetadata read(MUINT index,
          Type2Type<IMetadata> type,
          IMetadata::ContentSP& content) const;

    template <typename T, typename U>
    IMetadata::ContentSP
      write(const T& val, Type2Type<U> type, MUINT index = -1);

    template <typename T, typename U>
    IMetadata::ContentSP
      write(const T* array, size_t size, Type2Type<U> type, MUINT index = -1);

   public:
    // global static usage
    static size_t mStatNum;   // obj numbers
    static size_t mStatSize;  // bytes
  };

 public:
 /**
  * BurstEntry is a class used for burst read/write of IEntry.
  * Since IEntry operations are all thread safe, the lock/unlock operations
  * are necessary to be applied and may cause higher CPU usage while
  * frequently accessing IEntry. This class provide operations without
  * thread-safe guarantee, but only be used with IMetadata::burstRead and
  * IMetadata::burstWrite.
  */
  class BurstEntry : protected Storage {
   public:
     using Storage::readEntry;
     using Storage::readEntries;
     using Storage::writeEntry;
     using Storage::writeEntries;
     using Storage::getTagByIndex;
     using Storage::count;
  };

 private:
  /******************************************************************************
  * (Private) IMetadata data && member function
  ******************************************************************************/
  /**
   * Data structure inside the IMeadata
   */
  MUINT32 mGuardPatternBegin = 0x56781234;
  mutable unsigned int mValidNum;
  mutable unsigned int mValidNumBackup;
  mutable StorageSP mStorage;
  MUINT32 mGuardPatternMid = 0x24542454;
  mutable std::mutex mLock;
  MUINT32 mGuardPatternEnd = 0x43218765;
  /**
   * Debug global variable for tracking error tag
   */
  static Tag_t mErrorTypeTag;

  /**
   * Debug global variable for debug level setting
   * currently, only for control dump() output
   * Enable via adb shell setprop vendor.debug.camera.metadata 1
   */
  static int mLogLevel;

  /**
   * Debug global variable for counting the IMetadata obj
   */
  static unsigned int mSerialNum;

  /**
   * Query the pointer of Storage object inside the IMetadata
   */
  StorageSP getStorageSP() const;

  /**
   * Constructor an IMetadata from flatten bytes array
   *
   * @param flattenSrc pointer of flatten bytes array
   * @param size the length of flatten bytes array
   */
  IMetadata(const void* flattenSrc, size_t size);
  /**
   * Constructor an IMetadata from other IMetadata's Storage pointer
   *
   * @param src source for clone
   */
  explicit IMetadata(const StorageSP& src);
  /**
   * switch IMetadata Storage object into writeable
   * if current already writable, it do nothing
   * if current is readonly, it will clone another Storage object for writable
   */
  void _SwitchWritable() const;
  /**
   * switch IMetadata Storage object into readonly
   * Once the Storage object become readonly, it can't reverse back to writable
   */
  void _SwitchReadOnly() const;

  /**
   * [Debug API]. to record an error tag into mErrorTagTag variable
   * Only keep smaller tag if two error tag appear
   * @param tag tag to be tracked
   */
  static inline void setErrorTypeTag(Tag_t tag) {
    if (tag < mErrorTypeTag)
      mErrorTypeTag = tag;
  }

  /**
   * [Debug API]. to get the error tag
   */
  static inline Tag_t getErrorTypeTag(void) { return mErrorTypeTag; }

 public:
  /******************************************************************************
  * (Pubic) IMetadata::Memory HAL Interface
  ******************************************************************************/
  /**
   * IMetadata::Memory class is a std::vector object to handle bytes array data
   * Usually, caller will casting their struct data into raw bytes and store into
   * IMetadata::Memory
   *
   * - Note1: before use Memory object to store raw data. you can use IEntry array API first
   * @code
   *    > For example, we want ot store an std::string object
   *
   *      IMetadata::IEntry e;
   *      IMetadata m;
   *      std::string S = "Hello World";
   *      uint8_t* buf = reinterpret_cast<uint8_t*>(S.c_str());
   *      e.push_back(buf, S.size()+1, Type2Type<MUINT8>());
   *
   *    > push_back this API will store whole std::string + (null char) into IEntry
   *
   *      m.update(123,e);  // store e into metadata m
   *
   *    > retrieve the std::string object from metadata m
   *      std::string T((m.entryFor(123).data());
   * @endcode
   * - Note2: Only need to use Memory object if you want store multi bytes array data
   *
   */
  class Memory {
   public:
    /**
     * Memory constructor. create an empty bytes array
     */
    Memory();

    /**
     * Memory constructor. clone data from other Memory object
     * @param other source Memory object
     */
    Memory(const Memory& other);

    /**
     * Memory constructor. clone data from other Memory object
     * @param other source Memory object
     */
    Memory(Memory&& other);

    /**
     * Memory constructor. clone data from bytes array
     * @param data the start address of bytes array
     * @param size the length of bytes array
     */
    Memory(const void* data, size_t size);

    /**
     * Memory destructor
     */
    ~Memory();

    /**
     * Get the total length of bytes array
     * @return the total length
     */
    size_t size() const;

    /**
     * Change the length of bytes array
     * data will be truncated if original length < new length
     * @param size the new length of bytes array
     */
    void resize(const size_t size);

    /**
     * Append another Memory object data to end of bytes array
     * @param other the source data to append
     * @return return the total size of content
     */
    size_t append(const Memory& other);

    /**
     * Get the start address & legnth of bytes array     *
     * @retval retSize get the length of bytes array
     * @return the start address of bytes array
     */
    const uint8_t* array(size_t& retSize) const;

    /**
     * Get the start address of bytes array
     * @return the start address of bytes array
     */
    const uint8_t* array() const;

    /**
     * Get the start address & legnth of bytes array for modification.
     * @retval retSize get the length of bytes array
     * @return the start address of bytes array
     * @deprecated Only for backward compatible
     */
    uint8_t* editArray(size_t& retSize);

    /**
     * Get the start address of bytes array
     * @return the start address of bytes array
     * @deprecated Only for backward compatible
     */
    uint8_t* editArray();

    /**
     * Get the byte data from array with specify position
     * @code
     *    Same result as:
     *       array()[index]
     * @endcode
     * @param index the position of bytes array
     * @return the byte data
     */
    uint8_t itemAt(size_t index) const;

    /**
     * Clear all data and become empty bytes array, and the length will be 0
     */
    void clear();

    /**
     * Append another Memory object data to end of bytes array
     * @param other the source data to append
     * @return the total length of content
     * @deprecated Only for backword compatible use
     */
    size_t appendVector(const Memory& other);

    /**
     * Assign operation
     * clone data from other Memory object
     * @param other the source object to clone
     * @return Return "this" current object
     */
    Memory& operator=(Memory&& other);

    /**
     * Assign operation
     * clone data from other Memory object
     * @param other the source object to clone
     * @return Return "this" current object
     */
    Memory& operator=(const Memory& other);

    /**
     * Compare operation
     * @param other the source object to compare
     * @return return true if memcmp to check two bytes array  == 0
     */
    bool operator==(const Memory& other) const;

    /**
     * Compare operation
     * @param other the source object to compare
     * @return return true if memcmp to check two bytes array  != 0
     */
    bool operator!=(const Memory& other) const;

   private:
    explicit Memory(const BasicDataSP& src);
    const BasicDataSP getDataSP() const;
    friend IMetadata::Memory IEntry::itemAt(MUINT index,
        Type2Type<IMetadata::Memory> type) const;
    friend MBOOL Content::update(MINT index,
        const void* array, size_t size, MINT32 type);
    friend IMetadata::Memory IMetadata::Storage::read(MUINT index,
        Type2Type<IMetadata::Memory> type,
        IMetadata::ContentSP& content) const;
    mutable BasicDataSP mDataSP;
  };
};
}       // namespace NSCam

#endif  //  INCLUDE_MTKCAM_HALIF_UTILS_METADATA_1_X_IMETADATA_H_
