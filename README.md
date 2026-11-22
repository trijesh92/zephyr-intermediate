# Zephyr Training Environment

# L6-TASK1: Latency investigation and tracing

# Explanation
Control must start within 10 ms of the timer's ready_ms (start event => start of control work). A trace event to capture the start and end of the trask was implemented, to see the actual task start and end.
The delay came from the 45 ms k_busy_wait in the higher-priority maintenance thread. It ran first after each timer fire and blocked control for a full 45 ms, so control missed its deadline.
The fix: make the control task to run with higher priority. Control now starts at ~0 ms latency and maintenance's busy-wait only runs when control is blocked — no more deadline misses.