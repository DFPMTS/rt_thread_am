#include <am.h>
#include <klib.h>
#include <rtthread.h>

static Context *ev_handler(Event e, Context *c) {
  Context ***from_to = (Context ***)rt_thread_self()->user_data;
  Context **from, **to;
  switch (e.event) {
  case EVENT_YIELD:
    from = from_to[0];
    to = from_to[1];
    if (from) *from = c;
    c = *to;
    break;
  case EVENT_IRQ_TIMER:
    break;
  default:
    printf("Unhandled event ID = %d\n", e.event);
    assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  rt_ubase_t prev_user_data = rt_thread_self()->user_data;
  rt_ubase_t from_to[2] = {from, to};
  rt_thread_self()->user_data = (rt_ubase_t)from_to;
  yield();
  rt_thread_self()->user_data = prev_user_data;
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  rt_hw_context_switch(0, to);
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

static void wrapper_entry(void *arg) {
  void **args = arg;
  void (*tentry)(void *) = args[0];
  void *parameter = args[1];
  void (*texit)(void) = args[2];
  tentry(parameter);
  texit();
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  stack_addr = (rt_uint8_t*)((uintptr_t)stack_addr & ~(sizeof(uintptr_t) - 1));  
  stack_addr -= 3 * sizeof(uintptr_t);
  void **args = (void **)stack_addr;
  args[0] = tentry;
  args[1] = parameter;
  args[2] = texit;
  return (rt_uint8_t *)kcontext((Area){stack_addr - sizeof(Context) - sizeof(uintptr_t), stack_addr}, wrapper_entry, args);  
}
