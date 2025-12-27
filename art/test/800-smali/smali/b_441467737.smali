.class public LB441467737;
.super Ljava/lang/Object;

.field static public a:[I

.method public static main()V
   .registers 4
   const/16 v0, 0x80
   new-array v1, v0, [I
   sput-object v1, LB441467737;->a:[I

   const/4 v1, 0x0
   const-wide v2, 0x3fffffffffffffffL
   if-lt v1, v0, :jump_over
   sget-object v2, LB441467737;->a:[I
   aget v3, v2, v1
   :jump_over
   # This move is the one that caused the crash. Local from register 3 is nullptr.
   # Register 2 is a wide phi (therefore 3 is nullptr since we use two registers for wide locals).
   # At this point the current_locals_->size() is 4. We have:
   # 0: i3 IntConstant dex_pc:n/a block:B0 128 loop:none
   # 1: i4 IntConstant dex_pc:n/a block:B0 0 loop:none
   # 2: j16 Phi [j5,l10] dex_pc:n/a block:B3 reg:2 is_catch_phi:false is_live:true loop:none
   # 3: nullptr
   move v2, v3
   return-void
.end method
