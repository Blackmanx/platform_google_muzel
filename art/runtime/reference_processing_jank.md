Jank introduced by ART reference processing
===========================================


The ART garbage collector mostly runs concurrently with client code. But it occasionally has to
block client “mutator” threads, sometimes causing visible delays. One of the most significant
reasons for such delays, (often referred to as “jank”) is the need to block certain client code
during processing of `java.lang.ref.References`. This includes processing of finalizers and
`Cleaner`s.

We have invested a fair amount of effort in the past to partially address this. However, there is
one major issue that is difficult-to-impossible to solve. To our knowledge, this is not widely
described in the research literature, so we briefly discuss it here.


Problem Statement
-----------------

This issue now arises primarily due to an interaction between finalizers (which are handled
internally to ART via `FinalizerReference`s) and “native weak” references that are cleared only
when the referent object is permanently unreachable. The latter include JNI `WeakGlobalRef`s, as
well as ART internal references, such as weak references used to maintain the `String` intern
table.


What distinguishes such native weak references from Java `WeakReference`s and `PhantomReference`s
is that they both:

- Are not cleared while the referent is reachable from a finalizer. E.g. the “Weak Global
References” section of the JNI spec states: “Such a weak global reference will become functionally
equivalent to NULL at the same time as a `PhantomReference` referring to that same object would be
cleared by the garbage collector.”

- Can be dereferenced to yield a strong pointer, unlike `PhantomReference`s.

Since finalizers are deprecated and theoretically should disappear soon, this is theoretically
largely a solved problem. But in practice, finalizers are still pervasive in Android code, which
often mixes Java and C++ code, and uses finalizers to deallocate C++ objects. Thus in practice, we
expect this problem to persist for a while. None of which argues against continuing to replace
finalizers with `SystemCleaner` in Android code.

Processing finalizers requires that we:

1. Complete marking of strongly reachable objects to determine which finalizers should run and which
`WeakReference`s should be cleared.

2. Complete a second marking pass marking objects reachable from finalizers. This prevents such
objects from being reclaimed prematurely, and ensures that `PhantomReference`s are only enqueued
once we know that finalizers will not make their referents reachable again.

Unfortunately, this makes it difficult-to-impossible to dereference native weak references during
this second marking phase:

- If the referent for the weak native reference has not yet been marked, the only possible way to
return immediately would be to return null. That is clearly incorrect, since finalizer marking may
mark the object, which may then, in the worst case, be inserted into a permanent data structure.

- If the object has already been marked, returning a strong reference still appears dangerous at
this point. The object may have recently been marked as part of finalizer marking. But its
referents may not yet have been traced. Creating a strong referent visible to the mutator may
require the mutator to participate in that tracing. GC implementations are often not prepared for
this, since all objects reachable to the mutator are normally marked in the initial marking phase.


Possible improvements
---------------------

The “already marked” case could possibly be addressed by separately remembering the mark state of
the referents at the beginning of finalizer marking. We believe that objects already marked at
that point could be safely returned. But this adds enough extra work and/or complexity to make it
unclear whether it is a win.

It is also conceivable that for specific collector algorithms there may be other ways to handle
this case in a general way. This is probably worth further investigation.

The following is a less complicated, much more limited, improvement (taken from a prior Google
internal discussion at [b/190867430](b/190867430#comment30):

In special cases, we could ensure that if the referent of a native weak reference has been marked,
then no further marking needs to be done. If that’s the case, then it should be acceptable to
return early if the referent is marked, no matter when it was marked. This could probably be
arranged for the `String` intern table, which only refers to `String`s. However it is clearly a
very incomplete solution.
