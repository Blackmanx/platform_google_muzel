/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "virtual_thread_common.h"

#include "art_field-inl.h"
#include "art_method-inl.h"
#include "art_method.h"
#include "class_linker.h"
#include "class_root-inl.h"
#include "gc/heap.h"
#include "handle.h"
#include "handle_scope-inl.h"
#include "interpreter/shadow_frame.h"
#include "mirror/class-alloc-inl.h"
#include "mirror/object.h"
#include "mirror/object_array-alloc-inl.h"
#include "mirror/object_array-inl.h"
#include "mirror/object_array.h"
#include "mirror/throwable.h"
#include "monitor.h"
#include "obj_ptr.h"
#include "runtime.h"
#include "scoped_thread_state_change.h"
#include "stack.h"
#include "thread.h"
#include "well_known_classes.h"

namespace art HIDDEN {

inline static ArtMethod* GetEnterMethod(bool is_continuation_api) {
  return is_continuation_api ? WellKnownClasses::jdk_internal_vm_Continuation_enterSpecial
                             : nullptr;
}

inline static ArtMethod* GetParkMethod(bool is_continuation_api) {
  if (is_continuation_api) {
    return WellKnownClasses::jdk_internal_vm_Continuation_doYieldNative;
  } else {
    return WellKnownClasses::java_lang_Thread_parkVirtualInternal;
  }
}

struct VirtualThreadParkingVisitor final : public StackVisitor {
  VirtualThreadParkingVisitor(Thread* thread, bool is_continuation_api)
      REQUIRES_SHARED(Locks::mutator_lock_)
      : StackVisitor(thread, nullptr, StackVisitor::StackWalkKind::kIncludeInlinedFrames, true),
        enter_method_(GetEnterMethod(is_continuation_api)),
        park_method_(GetParkMethod(is_continuation_api)),
        shadow_frame_count_(0),
        reason_(kNoReason) {}
  bool VisitFrame() override REQUIRES_SHARED(Locks::mutator_lock_) {
    ShadowFrame* shadow_frame = GetCurrentShadowFrame();

    if (shadow_frame == nullptr) {
      ArtMethod** quick_frame = GetCurrentQuickFrame();
      ArtMethod* method = quick_frame != nullptr ? *quick_frame : nullptr;
      if (method != nullptr && method->IsNative()) {
        if (method == park_method_) {
          // Stack walking continues only if the only non-interpreted frame is
          // a known method parking the virtual thread / yielding the continuation.
          return true;
        } else if (method == enter_method_) {
          // The rest of the stack belongs to the carrier thread.
          return false;
        }

        reason_ = kNativeMethod;
        return false;
      }

      reason_ = kUnsupportedFrame;
      return false;
    }

    ArtMethod* method = shadow_frame->GetMethod();
    DCHECK(method != nullptr);
    if (method->IsClassInitializer()) {
      reason_ = kCriticalSection;
      return false;
    }

    if (!shadow_frame->GetLockCountData().IsEmpty()) {
      reason_ = kMonitor;
      return false;
    }

    // If the verifier was able to verify the locks are balanced, the interpreter won't update the
    // lock cound data. We need to walk the stack to find the locks here. Or should we just have an
    // increment/decrement counter?
    Monitor::VisitLocks(this, LockVisitingCallback, this);
    if (reason_ == kMonitor) {
      return false;
    }
    DCHECK(reason_ == kNoReason);

    shadow_frame_count_++;
    shadow_frames_.push_back(shadow_frame);

    return true;
  }

  static void LockVisitingCallback(ObjPtr<mirror::Object> obj, void* visitor) {
    DCHECK(!obj.IsNull());
    reinterpret_cast<VirtualThreadParkingVisitor*>(visitor)->reason_ = kMonitor;
  }

  const ArtMethod* const enter_method_;
  const ArtMethod* const park_method_;
  std::vector<const ShadowFrame*> shadow_frames_;
  size_t shadow_frame_count_;
  PinningReason reason_;
};

bool VirtualThreadPark(ObjPtr<mirror::Object> v_context,
                       ObjPtr<mirror::Object> parked_states,
                       ObjPtr<mirror::Throwable> vm_error,
                       bool is_continuation_api,
                       PinningReason& reason_) {
  Thread* self = Thread::Current();
  if (self->AreVirtualThreadFlagsEnabled(kContinuation) != is_continuation_api) {
    self->ThrowNewExceptionF("Ljava/lang/IllegalStateException;",
                             "unmatched kContinuation value when Virtual Thread is parking: %d",
                             is_continuation_api);
    return false;
  }

  StackHandleScope<9> hs(self);

  Handle<mirror::Object> v_context_h = hs.NewHandle(v_context);
  Handle<mirror::Object> parked_states_h = hs.NewHandle(parked_states);
  Handle<mirror::Object> opeer_h = hs.NewHandle(self->GetPeer());
  Handle<mirror::Throwable> vm_error_h = hs.NewHandle(vm_error);

  VirtualThreadParkingVisitor dump_visitor(self, is_continuation_api);
  dump_visitor.WalkStack();

  DCHECK_NE(dump_visitor.reason_, kUnsupportedFrame) << "JIT / AOT frame isn't supported.";
  reason_ = dump_visitor.reason_;
  if (dump_visitor.reason_ != kNoReason) {
    WellKnownClasses::dalvik_system_VirtualThreadContext_pinnedCarrierThread->SetObject<false>(
        v_context_h.Get(), opeer_h.Get());
    // Return to the java code to park the carrier thread
    return false;
  }

  size_t num_frames = dump_visitor.shadow_frames_.size();

  DCHECK(!Runtime::Current()->IsActiveTransaction());

  Handle<mirror::ObjectArray<mirror::Object>> frames_h =
      hs.NewHandle(mirror::ObjectArray<mirror::Object>::Alloc(
          self,
          WellKnownClasses::ToClass(WellKnownClasses::dalvik_system_VirtualThreadFrame__array),
          num_frames));

  MutableHandle<mirror::Object> vtf = hs.NewHandle<mirror::Object>(nullptr);
  MutableHandle<mirror::ByteArray> frame_bytes = hs.NewHandle<mirror::ByteArray>(nullptr);
  MutableHandle<mirror::Object> declaring_class = hs.NewHandle<mirror::Object>(nullptr);
  MutableHandle<mirror::ObjectArray<mirror::Object>> refs =
      hs.NewHandle<mirror::ObjectArray<mirror::Object>>(nullptr);

  for (size_t i = 0; i < num_frames; i++) {
    const ShadowFrame* sf = dump_visitor.shadow_frames_[i];
    DCHECK(sf != nullptr);
    sf->CheckConsistentVRegs();

    size_t num_vergs = sf->NumberOfVRegs();
    int32_t non_vref_size = ShadowFrame::ComputeSizeWithoutReferences(num_vergs);

    vtf.Assign(WellKnownClasses::dalvik_system_VirtualThreadFrame.Get()->Alloc(
        self, Runtime::Current()->GetHeap()->GetCurrentAllocator()));
    frame_bytes.Assign(mirror::ByteArray::Alloc(self, non_vref_size));
    declaring_class.Assign(sf->GetMethod()->GetDeclaringClass());
    refs.Assign(mirror::ObjectArray<mirror::Object>::Alloc(
        self, GetClassRoot<mirror::ObjectArray<mirror::Object>>(), num_vergs));
    DCHECK(!vtf.IsNull());
    DCHECK(!frame_bytes.IsNull());
    DCHECK(!refs.IsNull());

    bool areRefsAllNull = true;
    for (uint32_t j = 0; j < sf->NumberOfVRegs(); j++) {
      ObjPtr<mirror::Object> obj = sf->GetVRegReference(j);
      if (obj != nullptr) {
        refs->Set(j, obj);
        areRefsAllNull = false;
      }
    }

    if (!areRefsAllNull) {
      WellKnownClasses::dalvik_system_VirtualThreadFrame_refs->SetObject<false>(vtf.Get(),
                                                                                refs.Get());
    }
    frame_bytes->Memcpy(0, reinterpret_cast<const int8_t*>(sf), 0, non_vref_size);
    WellKnownClasses::dalvik_system_VirtualThreadFrame_frame->SetObject<false>(vtf.Get(),
                                                                               frame_bytes.Get());
    WellKnownClasses::dalvik_system_VirtualThreadFrame_declaringClass->SetObject<false>(
        vtf.Get(), declaring_class.Get());
    frames_h->Set(i, vtf.Get());
  }

  if (self->IsExceptionPending()) {
    // It's likely a OOME. Let's throw it at the java level.
    // Consider handling it in another way, e.g. pinning virtual thread, in the future.
    return false;
  }

  WellKnownClasses::dalvik_system_VirtualThreadParkedStates_frames->SetObject<false>(
      parked_states_h.Get(), frames_h.Get());
  WellKnownClasses::dalvik_system_VirtualThreadContext_parkedStates->SetObject<false>(
      v_context_h.Get(), parked_states_h.Get());
  self->SetVirtualThreadFlags(VirtualThreadFlag::kParking, true);

  // Throw a VirtualThreadParkingError to unwind the stack and park the virtual thread.
  self->SetException(vm_error_h.Get());
  return true;
}

}  // namespace art
