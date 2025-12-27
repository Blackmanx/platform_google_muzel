%def op_check_cast():
%  slow_path = add_slow_path(op_check_cast_slow_path)
   // Fast-path which gets the class from thread-local cache.
%  fetch_from_thread_cache("x1", miss_label=slow_path)
.L${opcode}_resume:
   lsr     w2, wINST, #8               // w2<- A
   GET_VREG w0, w2                     // w0<- vA (object)
   cbz     w0, .L${opcode}_next
   ldr     w2, [x0, #MIRROR_OBJECT_CLASS_OFFSET]
   UNPOISON_HEAP_REF w2
   // Fast path: do a comparison without read barrier.
   cmp     w1, w2
   bne     .L${opcode}_slow_path_check_hierarchy
.L${opcode}_next:
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_check_cast_slow_path():
   EXPORT_PC
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   bl      nterp_get_class
   mov     x1, x0
   b       .L${opcode}_resume
.L${opcode}_slow_path_check_hierarchy:
   // We don't do read barriers for simplicity. However, this means that x1
   // (and all other fetched objects) may be a from-space reference. That's OK as
   // we only fetch constant information from the references.
   // This also means that some of the comparisons below may lead to false negative,
   // but it will eventually be handled in the runtime.
   ldr     w3, [x1, #MIRROR_CLASS_ACCESS_FLAGS_OFFSET]
   tbnz    w3, #MIRROR_CLASS_IS_INTERFACE_FLAG_BIT, 2f
   ldr     w3, [x1, #MIRROR_CLASS_COMPONENT_TYPE_OFFSET]
   UNPOISON_HEAP_REF w3
   cbnz    w3, 5f
1:
   ldr     w2, [x2, #MIRROR_CLASS_SUPER_CLASS_OFFSET]
   UNPOISON_HEAP_REF w2
   cmp     w1, w2
   beq     .L${opcode}_next
   cbnz    w2, 1b
2:
   TEST_IF_MARKING 4f
3:
   EXPORT_PC
   bl      art_quick_check_instance_of
   b       .L${opcode}_next
4:
   bl      art_quick_read_barrier_mark_reg01
   b       3b
5:
   // Class in w1 is an array, w3 is the component type.
   ldr     w2, [x2, #MIRROR_CLASS_COMPONENT_TYPE_OFFSET]
   UNPOISON_HEAP_REF w2
   // Check if object is an array.
   cbz     w2, 2b
   ldr     w4, [x3, #MIRROR_CLASS_SUPER_CLASS_OFFSET]
   UNPOISON_HEAP_REF w4
   // If the super class of the component type is not null, go slow path.
   cbnz    w4, 2b
   ldrh    w3, [x3, #MIRROR_CLASS_OBJECT_PRIMITIVE_TYPE_OFFSET]
   // If the component type is primitive, go slow path.
   cbnz    w3, 2b
   // Check if the object is a primitive array.
   ldrh    w2, [x2, #MIRROR_CLASS_OBJECT_PRIMITIVE_TYPE_OFFSET]
   cbz     w2, .L${opcode}_next
   // Go slow path for throwing the exception.
   b 2b

%def op_instance_of():
%  slow_path = add_slow_path(op_instance_of_slow_path)
%  check_hierarchy = add_slow_path_for_last_prefix( \
%      op_instance_check_hierarchy, suffix="_check_hierarchy")
   /* instance-of vA, vB, class@CCCC */
   // Fast-path which gets the class from thread-local cache.
%  fetch_from_thread_cache("x1", miss_label=slow_path)
.L${opcode}_resume:
   lsr     w2, wINST, #12              // w2<- B
   GET_VREG w0, w2                     // w0<- vB (object)
   cbz     w0, .L${check_hierarchy}_store_result
   ldr     w2, [x0, #MIRROR_OBJECT_CLASS_OFFSET]
   UNPOISON_HEAP_REF w2
   // Fast path: do a comparison without read barrier.
   cmp     w1, w2
   bne     ${check_hierarchy}
   mov     w0, #1
   b       .L${check_hierarchy}_store_result

%def op_instance_of_slow_path():
   EXPORT_PC
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   bl      nterp_get_class
   mov     x1, x0
   b       .L${opcode}_resume

%def op_instance_check_hierarchy():
   // Go slow path if we are marking. Checking now allows
   // not going to slow path if the super class hierarchy check fails.
   TEST_IF_MARKING 4f
   ldr     w3, [x1, #MIRROR_CLASS_ACCESS_FLAGS_OFFSET]
   tbnz    w3, #MIRROR_CLASS_IS_INTERFACE_FLAG_BIT, 5f
   ldr     w3, [x1, #MIRROR_CLASS_COMPONENT_TYPE_OFFSET]
   UNPOISON_HEAP_REF w3
   cbnz    w3, 3f
   // Prepare result value 1 for a match in the loop below.
   mov     w0, #1
1:
   ldr     w2, [x2, #MIRROR_CLASS_SUPER_CLASS_OFFSET]
   UNPOISON_HEAP_REF w2
   cmp     w1, w2
   beq     .L${opcode}_check_hierarchy_store_result
   cbnz    w2, 1b
2:
   mov     w0, #0
   b       .L${opcode}_check_hierarchy_store_result
3:
   // Class in x1 is an array, x3 is the component type of x1, and x2 is the class of the object.
   ldr     w2, [x2, #MIRROR_CLASS_COMPONENT_TYPE_OFFSET]
   UNPOISON_HEAP_REF w2
   // Check if object is an array.
   cbz     w2, 2b
   // Check of x1 is Object[]
   ldr     w4, [x3, #MIRROR_CLASS_SUPER_CLASS_OFFSET]
   UNPOISON_HEAP_REF w4
   // If the super class is not Object, go to slow path.
   cbnz    w4, 5f
   // Super class is null, this could either be a primitive array or Object[].
   ldrh    w3, [x3, #MIRROR_CLASS_OBJECT_PRIMITIVE_TYPE_OFFSET]
   // If x1 is a primitive array class, we know the check is false.
   cbnz    w3, 2b
   // Check if x2 is a primitive array class.
   ldrh    w2, [x2, #MIRROR_CLASS_OBJECT_PRIMITIVE_TYPE_OFFSET]
   cmp     w2, #0
   cset    w0, eq
   b       .L${opcode}_check_hierarchy_store_result
4:
   bl      art_quick_read_barrier_mark_reg01
5:
   EXPORT_PC
   bl      artInstanceOfFromCode
.L${opcode}_check_hierarchy_store_result:
   ubfx    w1, wINST, #8, #4           // w1<- A
   SET_VREG w0, w1
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_iget_boolean():
%  op_iget(load="ldrb", volatile_load="ldarb", maybe_extend="")

%def op_iget_byte():
%  op_iget(load="ldrsb", volatile_load="ldarb", maybe_extend="sxtb w0, w0")

%def op_iget_char():
%  op_iget(load="ldrh", volatile_load="ldarh", maybe_extend="")

%def op_iget_short():
%  op_iget(load="ldrsh", volatile_load="ldarh", maybe_extend="sxth w0, w0")

%def op_iget(load="ldr", volatile_load="ldar", maybe_extend="", wide=False, is_object=False):
%  slow_path = add_slow_path(op_iget_slow_path, volatile_load, maybe_extend, wide, is_object)
   // Fast-path which gets the field from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
.L${opcode}_resume:
   lsr     w2, wINST, #12              // w2<- B
   GET_VREG w3, w2                     // w3<- object we're operating on
   ubfx    w2, wINST, #8, #4           // w2<- A
   cbz     w3, common_errNullObject    // object was null
%  if wide:
     $load   x0, [x3, x0]
     SET_VREG_WIDE x0, w2              // fp[A] <- value
%  elif is_object:
     $load   w0, [x3, x0]
     UNPOISON_HEAP_REF w0
     TEST_IF_MARKING .L${opcode}_slow_path_mark_and_store_r0_to_vreg_r2_fetch_advance_2_dispatch
     SET_VREG_OBJECT w0, w2            // fp[A] <- value
%  else:
     $load   w0, [x3, x0]
     SET_VREG w0, w2                   // fp[A] <- value
   FETCH_ADVANCE_INST 2
#ifdef USE_HEAP_POISONING
%  if is_object:
     b .L${opcode}_slow_path_dispatch  // Dispatch would not fit in 64B.
%  else:
     PREPARE_OPCODE_DISPATCH
     GOTO_OPCODE
%  pass
#else
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE
#endif
%  pass

%def op_iget_slow_path(volatile_load, maybe_extend, wide, is_object):
   ldr     x0, [sp]
   ldr     w1, [x0, #ART_METHOD_ACCESS_FLAGS_OFFSET]
   tbnz    w1, #ART_METHOD_IS_OBSOLETE_FLAG_BIT, .L${opcode}_slow_path_continue
   // We don't do a read barrier for faster execution.
   // See comment in `NterpGetLocalInstanceFieldInternal`.
   ldr     w0, [x0, ART_METHOD_DECLARING_CLASS_OFFSET]
   mov     x1, xPC
   bl      NterpGetLocalInstanceField
   tbz     w0, #31, .L${opcode}_resume
.L${opcode}_slow_path_continue:
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   mov     x3, xFP
   EXPORT_PC
   bl      nterp_get_instance_field_offset
   // Zero extension (nterp_get_instance_field_offset returns uint32_t) of the return value is
   // needed as the value is used below via wider X0 register - AARCH64 AAPCS specifies that
   // "... any unused bits in the register have unspecified value" (see 6.8.2, 6.9).
   mov     w0, w0
   tbz     w0, #31, .L${opcode}_resume
   CLEAR_INSTANCE_VOLATILE_MARKER w0
   lsr     w2, wINST, #12              // w2<- B
   GET_VREG w3, w2                     // w3<- object we're operating on
   ubfx    w2, wINST, #8, #4           // w2<- A
   cbz     w3, common_errNullObject    // object was null
   add     x3, x3, x0
%  if wide:
     $volatile_load x0, [x3]
     SET_VREG_WIDE x0, w2              // fp[A] <- value
%  elif is_object:
     $volatile_load w0, [x3]
     UNPOISON_HEAP_REF w0
     TEST_IF_MARKING .L${opcode}_slow_path_mark_and_store_r0_to_vreg_r2_fetch_advance_2_dispatch
   // `new-array` handler jumps here to shorten the code.
.L${opcode}_slow_path_store_r0_to_vreg_r2_fetch_advance_2_dispatch:
     SET_VREG_OBJECT w0, w2            // fp[A] <- value
%  else:
     $volatile_load w0, [x3]
     $maybe_extend
     SET_VREG w0, w2                   // fp[A] <- value
   FETCH_ADVANCE_INST 2
   // `aget-object` handler jumps here to shorten the code for heap poisoning.
.L${opcode}_slow_path_dispatch:
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE
%  if is_object:
     // `aget-object` handler jumps here when marking to shorten the code.
.L${opcode}_slow_path_mark_and_store_r0_to_vreg_r2_fetch_advance_2_dispatch:
     bl      art_quick_read_barrier_mark_reg00
     b       .L${opcode}_slow_path_store_r0_to_vreg_r2_fetch_advance_2_dispatch

%def op_iget_wide():
%  op_iget(load="ldr", volatile_load="ldar", maybe_extend="", wide=True)

%def op_iget_object():
%  op_iget(load="ldr", volatile_load="ldar", maybe_extend="", is_object=True)

%def op_iput_boolean():
%  op_iput(store="strb", volatile_store="stlrb")

%def op_iput_byte():
%  op_iput(store="strb", volatile_store="stlrb")

%def op_iput_char():
%  op_iput(store="strh", volatile_store="stlrh")

%def op_iput_short():
%  op_iput(store="strh", volatile_store="stlrh")

%def op_iput(store="str", volatile_store="stlr", wide=False, is_object=False):
%  slow_path = add_slow_path(op_iput_slow_path, volatile_store, wide, is_object)
   // Fast-path which gets the field from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
.L${opcode}_resume:
   lsr     w2, wINST, #12              // w2<- B
   ubfx    w1, wINST, #8, #4           // w1<- A
   GET_VREG w2, w2                     // vB (object we're operating on)
%  if wide:
     GET_VREG_WIDE x1, w1              // x1<- fp[A]/fp[A+1]
%  else:
     GET_VREG w1, w1                   // w1 <- v[A]
   cbz w2, common_errNullObject
%  if wide:
     $store  x1, [x2, x0]
%  elif is_object:
     POISON_HEAP_REF w1
     $store  w1, [x2, x0]
     cbnz w1, .L${opcode}_slow_path_write_barrier  // Write barrier would not fit in 64B.
%  else:
     $store  w1, [x2, x0]
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_iput_slow_path(volatile_store, wide, is_object):
   ldr     x0, [sp]
   ldr     w1, [x0, #ART_METHOD_ACCESS_FLAGS_OFFSET]
   tbnz    w1, #ART_METHOD_IS_OBSOLETE_FLAG_BIT, .L${opcode}_slow_path_continue
   // We don't do a read barrier for faster execution.
   // See comment in `NterpGetLocalInstanceFieldInternal`.
   ldr     w0, [x0, ART_METHOD_DECLARING_CLASS_OFFSET]
   mov     x1, xPC
%  if is_object:
     bl      NterpGetLocalInstanceFieldForIPutObject
%  else:
     bl      NterpGetLocalInstanceField
   tbz     w0, #31, .L${opcode}_resume
.L${opcode}_slow_path_continue:
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   mov     x3, xFP
   EXPORT_PC
   bl      nterp_get_instance_field_offset
   // Zero extension (nterp_get_instance_field_offset returns uint32_t) of the return value is
   // needed as the value is used below via wider X0 register - AARCH64 AAPCS specifies that
   // "... any unused bits in the register have unspecified value" (see 6.8.2, 6.9).
   mov     w0, w0
   tbz     w0, #31, .L${opcode}_resume
   CLEAR_INSTANCE_VOLATILE_MARKER w0
   lsr     w2, wINST, #12              // w2<- B
   ubfx    w1, wINST, #8, #4           // w1<- A
   GET_VREG w2, w2                     // vB (object we're operating on)
%  if wide:
     GET_VREG_WIDE x1, w1              // x1<- fp[A]/fp[A+1]
%  else:
     GET_VREG w1, w1                   // w1 <- v[A]
   cbz     w2, common_errNullObject
   add     x3, x2, x0
%  if wide:
     $volatile_store x1, [x3]
%  elif is_object:
     POISON_HEAP_REF w1
     $volatile_store w1, [x3]
     cbz     w1, .L${opcode}_slow_path_skip_write_barrier
.L${opcode}_slow_path_write_barrier:
     WRITE_BARRIER_NOT_NULL w2
.L${opcode}_slow_path_skip_write_barrier:
%    pass
%  else:
     $volatile_store w1, [x3]
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_iput_wide():
%  op_iput(store="str", volatile_store="stlr", wide=True)

%def op_iput_object():
%  op_iput(store="str", volatile_store="stlr", is_object=True)

%def op_sget_boolean():
%  op_sget(load="ldrb", volatile_load="ldarb", maybe_extend="")

%def op_sget_byte():
%  op_sget(load="ldrsb", volatile_load="ldarb", maybe_extend="sxtb w0, w0")

%def op_sget_char():
%  op_sget(load="ldrh", volatile_load="ldarh", maybe_extend="")

%def op_sget_short():
%  op_sget(load="ldrsh", volatile_load="ldarh", maybe_extend="sxth w0, w0")

%def op_sget(load="ldr", volatile_load="ldar", maybe_extend="", wide=False, is_object=False):
%  slow_path = add_slow_path(op_sget_slow_path, load, volatile_load, maybe_extend, wide, is_object)
   // Fast-path which gets the field from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
.L${opcode}_resume:
   ldr     w1, [x0, #ART_FIELD_OFFSET_OFFSET]
   lsr     w2, wINST, #8               // w2 <- A
   ldr     w0, [x0, #ART_FIELD_DECLARING_CLASS_OFFSET]
   TEST_IF_MARKING .L${opcode}_read_barrier
.L${opcode}_resume_after_read_barrier:
%  if wide:
     ldr     x0, [x0, x1]
     SET_VREG_WIDE x0, w2              // fp[A] <- value
%  elif is_object:
     $load   w0, [x0, x1]
     UNPOISON_HEAP_REF w0
     // No need to check the marking register, we know it's not set here.
.L${opcode}_after_reference_load:
     SET_VREG_OBJECT w0, w2            // fp[A] <- value
%  else:
     $load   w0, [x0, x1]
     SET_VREG w0, w2                   // fp[A] <- value
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_sget_slow_path(load, volatile_load, maybe_extend, wide, is_object):
   ldr     x0, [sp]
   ldr     w1, [x0, #ART_METHOD_ACCESS_FLAGS_OFFSET]
   tbnz    w1, #ART_METHOD_IS_OBSOLETE_FLAG_BIT, .L${opcode}_slow_path_continue
   // We don't do a read barrier for faster execution.
   // See comment in `NterpGetLocalStaticFieldInternal`.
   ldr     w0, [x0, ART_METHOD_DECLARING_CLASS_OFFSET]
   mov     x1, xPC
   bl      NterpGetLocalStaticField
   cbnz    x0, .L${opcode}_resume
.L${opcode}_slow_path_continue:
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   mov     x3, #0
   EXPORT_PC
   bl      nterp_get_static_field
   tbz     x0, #0, .L${opcode}_resume
   CLEAR_STATIC_VOLATILE_MARKER x0
   ldr     w1, [x0, #ART_FIELD_OFFSET_OFFSET]
   lsr     w2, wINST, #8               // w2 <- A
   ldr     w0, [x0, #ART_FIELD_DECLARING_CLASS_OFFSET]
   TEST_IF_MARKING .L${opcode}_slow_path_read_barrier
.L${opcode}_slow_path_resume_after_read_barrier:
   add     x0, x0, x1
%  if wide:
     ldar    x0, [x0]
     SET_VREG_WIDE x0, w2              // fp[A] <- value
%  elif is_object:
     $volatile_load w0, [x0]
     UNPOISON_HEAP_REF w0
     TEST_IF_MARKING .L${opcode}_mark_after_load
     SET_VREG_OBJECT w0, w2            // fp[A] <- value
%  else:
     $volatile_load w0, [x0]
     $maybe_extend
     SET_VREG w0, w2                   // fp[A] <- value
   FETCH_ADVANCE_INST 2
.L${opcode}_slow_path_dispatch:
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE
.L${opcode}_slow_path_read_barrier:
   bl      art_quick_read_barrier_mark_reg00
   b       .L${opcode}_slow_path_resume_after_read_barrier
.L${opcode}_read_barrier:
   bl      art_quick_read_barrier_mark_reg00
%  if is_object:
     $load   w0, [x0, x1]
     UNPOISON_HEAP_REF w0
.L${opcode}_mark_after_load:
     // Here, we know the marking register is set.
     bl      art_quick_read_barrier_mark_reg00
     b       .L${opcode}_after_reference_load
%  else:
     b       .L${opcode}_resume_after_read_barrier

%def op_sget_wide():
%  op_sget(load="ldr", volatile_load="ldar", maybe_extend="", wide=True)

%def op_sget_object():
%  op_sget(load="ldr", volatile_load="ldar", maybe_extend="", is_object=True)

%def op_sput_boolean():
%  op_sput(store="strb", volatile_store="stlrb")

%def op_sput_byte():
%  op_sput(store="strb", volatile_store="stlrb")

%def op_sput_char():
%  op_sput(store="strh", volatile_store="stlrh")

%def op_sput_short():
%  op_sput(store="strh", volatile_store="stlrh")

%def op_sput(store="str", volatile_store="stlr", wide=False, is_object=False):
%  slow_path = add_slow_path(op_sput_slow_path, volatile_store, wide, is_object)
   lsr     wINST, wINST, #8            // wINST <- A
   // Fast-path which gets the field from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
.L${opcode}_resume:
   ldr     w1, [x0, #ART_FIELD_OFFSET_OFFSET]
   ldr     w0, [x0, #ART_FIELD_DECLARING_CLASS_OFFSET]
   TEST_IF_MARKING .L${opcode}_read_barrier
.L${opcode}_resume_after_read_barrier:
%  if wide:
     GET_VREG_WIDE x2, wINST           // x2 <- v[A]
     $store  x2, [x0, x1]
%  elif is_object:
     GET_VREG w2, wINST                // w2 <- v[A]
     POISON_HEAP_REF w2
     $store  w2, [x0, x1]
     cbnz    w2, .L${opcode}_slow_path_write_barrier  // Write barrier would not fit in 64B.
%  else:
     GET_VREG w2, wINST                // w2 <- v[A]
     $store  w2, [x0, x1]
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE
%  if not wide and not is_object:
.L${opcode}_read_barrier:
     bl      art_quick_read_barrier_mark_reg00
     b       .L${opcode}_resume_after_read_barrier

%def op_sput_slow_path(volatile_store, wide, is_object):
   ldr     x0, [sp]
   ldr     w1, [x0, #ART_METHOD_ACCESS_FLAGS_OFFSET]
   tbnz    w1, #ART_METHOD_IS_OBSOLETE_FLAG_BIT, .L${opcode}_slow_path_continue
   // We don't do a read barrier for faster execution.
   // See comment in `NterpGetLocalStaticFieldInternal`.
   ldr     w0, [x0, ART_METHOD_DECLARING_CLASS_OFFSET]
   mov     x1, xPC
%  if is_object:
     bl      NterpGetLocalStaticFieldForSPutObject
%  else:
     bl      NterpGetLocalStaticField
   cbnz    x0, .L${opcode}_resume
.L${opcode}_slow_path_continue:
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
%  if is_object:
     GET_VREG w3, wINST               // w3 <- v[A]
%  else:
     mov     x3, #0
   EXPORT_PC
   bl      nterp_get_static_field
   tbz     x0, #0, .L${opcode}_resume
   CLEAR_STATIC_VOLATILE_MARKER x0
   ldr     w1, [x0, #ART_FIELD_OFFSET_OFFSET]
   ldr     w0, [x0, #ART_FIELD_DECLARING_CLASS_OFFSET]
   TEST_IF_MARKING .L${opcode}_slow_path_read_barrier
.L${opcode}_slow_path_resume_after_read_barrier:
   add     x1, x0, x1
%  if wide:
     GET_VREG_WIDE x2, wINST           // x2 <- v[A]
     $volatile_store    x2, [x1]
%  elif is_object:
     GET_VREG w2, wINST                // w2 <- v[A]
     POISON_HEAP_REF w2
     $volatile_store    w2, [x1]
     cbz     w2, .L${opcode}_slow_path_skip_write_barrier
.L${opcode}_slow_path_write_barrier:
     WRITE_BARRIER_NOT_NULL w0
.L${opcode}_slow_path_skip_write_barrier:
%    pass
%  else:
     GET_VREG w2, wINST                // w2 <- v[A]
     $volatile_store    w2, [x1]
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE
.L${opcode}_slow_path_read_barrier:
   bl      art_quick_read_barrier_mark_reg00
   b       .L${opcode}_slow_path_resume_after_read_barrier
%  if wide or is_object:
.L${opcode}_read_barrier:
     bl      art_quick_read_barrier_mark_reg00
     b       .L${opcode}_resume_after_read_barrier

%def op_sput_wide():
%  op_sput(store="str", volatile_store="stlr", wide=True)

%def op_sput_object():
%  op_sput(store="str", volatile_store="stlr", is_object=True)

%def op_new_instance():
%  slow_path = add_slow_path(op_new_instance_slow_path)
   EXPORT_PC
   // Fast-path which gets the class from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
   TEST_IF_MARKING .L${opcode}_slow_path_read_barrier
.L${opcode}_resume_after_read_barrier:
   ldr     lr, [xSELF, #THREAD_ALLOC_OBJECT_ENTRYPOINT_OFFSET]
   blr     lr
   dmb     ishst                       // need fence for making object's class visible
.L${opcode}_resume:
   lsr     w1, wINST, #8               // w1 <- A
   SET_VREG_OBJECT w0, w1              // fp[A] <- value
   FETCH_ADVANCE_INST 2
   PREPARE_OPCODE_DISPATCH
   GOTO_OPCODE

%def op_new_instance_slow_path():
   mov     x0, xSELF
   ldr     x1, [sp]
   mov     x2, xPC
   bl      nterp_allocate_object
   b       .L${opcode}_resume
.L${opcode}_slow_path_read_barrier:
   bl      art_quick_read_barrier_mark_reg00
   b       .L${opcode}_resume_after_read_barrier
