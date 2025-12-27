.class public LB28187158_2;

# Regression test for iget with unresolved class.

.super Ljava/lang/Object;

.field public f:I

.method public static run()V
   .registers 1
   const/4 v0, 0
   invoke-static {v0}, LB28187158_2;->test(Ljava/lang/DoesNotExist;)V
   return-void
.end method

.method public static test(Ljava/lang/DoesNotExist;)V
   .registers 2
   iget v0, p0, LB28187158_2;->f:I
   return-void
.end method
