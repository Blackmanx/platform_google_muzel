INCLUDE PERFETTO MODULE android.memory.heap_profile.summary_tree;

SELECT name, SUM(cumulative_alloc_size) as size
FROM android_heap_profile_summary_tree
WHERE name LIKE 'android.test.javaheapprof.JavaHeapProfTest$Allocator.__'
GROUP BY name;

