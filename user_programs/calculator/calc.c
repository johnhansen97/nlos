void print_str(const char *);
void sys_exit(void);

void main(void) {
  print_str("Hello world from C!");
  sys_exit();
}

void print_str(const char *s) {
  asm volatile ("movl $1, %%eax;movl %0, %%ebx;int $0x80"
		:
		:"r"(s)
		:"%eax","%ebx");
}

void sys_exit(void) {
  asm volatile ("movl $0, %%eax;int $0x80" : : : "%eax");
}
