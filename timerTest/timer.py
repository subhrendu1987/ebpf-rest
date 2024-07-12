from bcc import BPF
from time import strftime

# BPF program
bpf_program = """
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

int trace_write(struct pt_regs *ctx) {
    // Log the process ID and current timestamp
    bpf_trace_printk("write syscall invoked\\n");
    return 0;
}
"""

# Load BPF program
bpf = BPF(text=bpf_program)

# Attach BPF program to the write syscall
bpf.attach_kprobe(event="__x64_sys_write", fn_name="trace_write")

# Print trace output
print("Tracing write syscall... Press Ctrl+C to exit.")
while True:
    try:
        trace_output = bpf.trace_fields()
        # Assuming the last field is the message
        msg = trace_output[-1]
        print(f"[{strftime('%H:%M:%S')}] {msg}")
    except KeyboardInterrupt:
        print("Exiting...")
        break
