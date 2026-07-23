# Kernel CMakeLists.txt
add_library(kernel_objs OBJECT
    boot/entry.asm
    boot/boot.c
    kernel_main.c
    mm/pmm.c
    mm/vmm.c
    mm/heap.c
    sched/scheduler.c
    sched/process.c
    sched/thread.c
    ipc/msgqueue.c
    ipc/sharedmem.c
    syscall/syscall.c
    syscall/syscall_table.c
)

target_include_directories(kernel_objs PRIVATE include)
target_compile_options(kernel_objs PRIVATE ${CMAKE_C_FLAGS})
set(KERNEL_OBJS ${KERNEL_OBJS} $<TARGET_OBJECTS:kernel_objs> PARENT_SCOPE)
