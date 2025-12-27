%def op_aget(load="ldr", shift=2, data_offset="MIRROR_INT_ARRAY_DATA_OFFSET", wide=False, is_object=False):
/*
 * Array get.  vAA <- vBB[vCC].
 *
 * for: aget, aget-boolean, aget-byte, aget-char, aget-short, aget-wide, aget-object
 *
 */
    FETCH_B w9, 1, 0                    // w9<- BB
    lsr     w2, wINST, #8               // w2<- AA
    FETCH_B w3, 1, 1                    // w3<- CC
    GET_VREG w0, w9                     // w0<- vBB (array object)
    GET_VREG w1, w3                     // w1<- vCC (requested index)
    // Note: if x0 is null, this will be handled by our SIGSEGV handler.
    ldr     w3, [x0, #MIRROR_ARRAY_LENGTH_OFFSET]    // w3<- arrayObj->length
    add     x0, x0, w1, uxtw #$shift    // w0<- arrayObj + index*width
    cmp     w1, w3                      // compare unsigned index, length
    bcs     common_errArrayIndex        // index >= length, bail
%   if wide:
      ldr     x0, [x0, #$data_offset]   // x0<- vBB[vCC]
      FETCH_ADVANCE_INST 2              // advance rPC, load rINST
      SET_VREG_WIDE x0, w2
%   elif is_object:
      $load   w0, [x0, #$data_offset]   // w0<- vBB[vCC]
      UNPOISON_HEAP_REF w0
      // Shorten the handler by using the code from `op_iget_object` slow path.
%     iget_opcode = re.sub("op_aget", "op_iget", opcode)
      TEST_IF_MARKING \
          .L${iget_opcode}_slow_path_mark_and_store_r0_to_vreg_r2_fetch_advance_2_dispatch
      FETCH_ADVANCE_INST 2              // advance rPC, load rINST
      SET_VREG_OBJECT w0, w2            // vAA<- w0
%   else:
      $load   w0, [x0, #$data_offset]   // w0<- vBB[vCC]
      FETCH_ADVANCE_INST 2              // advance rPC, load rINST
      SET_VREG w0, w2                   // vAA<- w0
%   pass
#ifdef USE_HEAP_POISONING
%   if is_object:
      b .L${iget_opcode}_slow_path_dispatch  // Dispatch would not fit in 64B.
%   else:
      PREPARE_OPCODE_DISPATCH           // insert opcode to xIBASE
      GOTO_OPCODE                       // jump to next instruction
%   pass
#else
    PREPARE_OPCODE_DISPATCH             // insert opcode to xIBASE
    GOTO_OPCODE                         // jump to next instruction
#endif
%   pass

%def op_aget_boolean():
%  op_aget(load="ldrb", shift=0, data_offset="MIRROR_BOOLEAN_ARRAY_DATA_OFFSET")

%def op_aget_byte():
%  op_aget(load="ldrsb", shift=0, data_offset="MIRROR_BYTE_ARRAY_DATA_OFFSET")

%def op_aget_char():
%  op_aget(load="ldrh", shift=1, data_offset="MIRROR_CHAR_ARRAY_DATA_OFFSET")

%def op_aget_object():
%  op_aget(load="ldr", shift=2, data_offset="MIRROR_OBJECT_ARRAY_DATA_OFFSET", is_object=True)

%def op_aget_short():
%  op_aget(load="ldrsh", shift=1, data_offset="MIRROR_SHORT_ARRAY_DATA_OFFSET")

%def op_aget_wide():
%  op_aget(load="ldr", shift=3, data_offset="MIRROR_WIDE_ARRAY_DATA_OFFSET", wide=True)

%def op_aput(store="str", shift=2, data_offset="MIRROR_INT_ARRAY_DATA_OFFSET", wide=False, is_object=False):
/*
 * Array put.  vBB[vCC] <- vAA.
 *
 * for: aput, aput-boolean, aput-byte, aput-char, aput-short, aput-wide, aput-object
 *
 */
    FETCH_B w2, 1, 0                    // w2<- BB
    lsr     w9, wINST, #8               // w9<- AA
    FETCH_B w3, 1, 1                    // w3<- CC
    GET_VREG w0, w2                     // w0<- vBB (array object)
    GET_VREG w1, w3                     // w1<- vCC (requested index)
    // Note: if x0 is null, this will be handled by our SIGSEGV handler.
    ldr     w3, [x0, #MIRROR_ARRAY_LENGTH_OFFSET]     // w3<- arrayObj->length
%   if not is_object:
      add     x0, x0, w1, uxtw #$shift  // w0<- arrayObj + index*width
    cmp     w1, w3                      // compare unsigned index, length
    bcs     common_errArrayIndex        // index >= length, bail
%   if is_object:
      EXPORT_PC                         // Export PC before overwriting it.
    FETCH_ADVANCE_INST 2                // advance rPC, load rINST
%   if wide:
      GET_VREG_WIDE x2, w9              // x2<- vAA
%   else:
      GET_VREG w2, w9                   // w2<- vAA
%   if wide:
      $store  x2, [x0, #$data_offset]   // vBB[vCC]<- x2
%   elif is_object:
      bl art_quick_aput_obj
%   else:
      $store  w2, [x0, #$data_offset]   // vBB[vCC]<- w2
    PREPARE_OPCODE_DISPATCH             // insert opcode to xIBASE
    GOTO_OPCODE                         // jump to next instruction

%def op_aput_boolean():
%  op_aput(store="strb", shift=0, data_offset="MIRROR_BOOLEAN_ARRAY_DATA_OFFSET")

%def op_aput_byte():
%  op_aput(store="strb", shift=0, data_offset="MIRROR_BYTE_ARRAY_DATA_OFFSET")

%def op_aput_char():
%  op_aput(store="strh", shift=1, data_offset="MIRROR_CHAR_ARRAY_DATA_OFFSET")

%def op_aput_short():
%  op_aput(store="strh", shift=1, data_offset="MIRROR_SHORT_ARRAY_DATA_OFFSET")

%def op_aput_wide():
%  op_aput(store="str", shift=3, data_offset="MIRROR_WIDE_ARRAY_DATA_OFFSET", wide=True)

%def op_aput_object():
%  op_aput(store="str", shift=2, data_offset="MIRROR_INT_ARRAY_DATA_OFFSET", is_object=True)

%def op_array_length():
    /*
     * Return the length of an array.
     */
    lsr     w1, wINST, #12              // w1<- B
    ubfx    w2, wINST, #8, #4           // w2<- A
    GET_VREG w0, w1                     // w0<- vB (object ref)
    cbz     w0, common_errNullObject    // bail if null
    FETCH_ADVANCE_INST 1                // advance rPC, load rINST
    ldr     w3, [x0, #MIRROR_ARRAY_LENGTH_OFFSET]    // w3<- array length
    PREPARE_OPCODE_DISPATCH             // insert opcode to xIBASE
    SET_VREG w3, w2                     // vB<- length
    GOTO_OPCODE                         // jump to next instruction

%def op_fill_array_data():
    /* fill-array-data vAA, +BBBBBBBB */
    EXPORT_PC
    FETCH   w0, 1                       // x0<- 000000000000bbbb (lo)
    FETCH_S x1, 2                       // x1<- ssssssssssssBBBB (hi)
    lsr     w3, wINST, #8               // w3<- AA
    orr     x0, x0, x1, lsl #16         // x0<- ssssssssBBBBbbbb
    GET_VREG w1, w3                     // w1<- vAA (array object)
    add     x0, xPC, x0, lsl #1         // x0<- PC + ssssssssBBBBbbbb*2 (array data off.)
    bl      art_quick_handle_fill_data
    FETCH_ADVANCE_INST 3                // advance rPC, load rINST
    PREPARE_OPCODE_DISPATCH             // insert opcode to xIBASE
    GOTO_OPCODE                         // jump to next instruction

%def op_filled_new_array(helper="nterp_filled_new_array"):
/*
 * Create a new array with elements filled from registers.
 *
 * for: filled-new-array, filled-new-array/range
 */
    /* op vB, {vD, vE, vF, vG, vA}, class@CCCC */
    /* op {vCCCC..v(CCCC+AA-1)}, type@BBBB */
    EXPORT_PC
    mov     x0, xSELF
    ldr     x1, [sp]
    mov     x2, xFP
    mov     x3, xPC
    bl      $helper
    FETCH_ADVANCE_INST 3                // advance rPC, load rINST
    PREPARE_OPCODE_DISPATCH             // insert opcode to xIBASE
    GOTO_OPCODE                         // jump to next instruction

%def op_filled_new_array_range():
%  op_filled_new_array(helper="nterp_filled_new_array_range")

%def op_new_array():
%  slow_path = add_slow_path(op_new_array_slow_path)
   /* new-array vA, vB, class@CCCC */
   EXPORT_PC
   // Fast-path which gets the class from thread-local cache.
%  fetch_from_thread_cache("x0", miss_label=slow_path)
   TEST_IF_MARKING .L${opcode}_slow_path_read_barrier
.L${opcode}_resume:
   lsr     w1, wINST, #12              // w1<- B
   GET_VREG w1, w1                     // w1<- vB (array length)
   ldr lr, [xSELF, #THREAD_ALLOC_ARRAY_ENTRYPOINT_OFFSET]
   blr lr
   dmb ishst                           // need fence for making array's class visible
   ubfx    w2, wINST, #8, #4           // w2<- A
   // Shorten the handler by using the dispatch from `op_iget_object` slow path.
%  iget_object_opcode = re.sub("op_new_array", "op_iget_object", opcode)
   b .L${iget_object_opcode}_slow_path_store_r0_to_vreg_r2_fetch_advance_2_dispatch

%def op_new_array_slow_path():
   mov x0, xSELF
   ldr x1, [sp, 0]
   mov x2, xPC
   bl nterp_get_class
   b .L${opcode}_resume
.L${opcode}_slow_path_read_barrier:
   bl art_quick_read_barrier_mark_reg00
   b .L${opcode}_resume
