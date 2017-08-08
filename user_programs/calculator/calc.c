void print_str(const char *);
char get_ch(void);
void sys_exit(void);
void parseInput(void);

char inputString[256];
char outputString[20];
int strIndex = 0;

void main(void) {
  print_str("NLOS Calculator\n");
  print_str("Usage: <integer> <operator> <integer>\n");
  print_str("Spaces are required and both operands must be positive.\n");
  print_str("Accepted operators: + - * /\n");

  while (1) {
    strIndex = 0;
    print_str("$ ");

    while (1) {
      inputString[strIndex] = get_ch();

      if (inputString[strIndex] == '\b') {
	//Fix backspace underflow bug
	if (strIndex == 0) {
	  continue;
	}

	strIndex--;
	inputString[strIndex + 1] = 0;
	inputString[strIndex] = ' ';
	print_str("\b");
	continue;
      }

      inputString[strIndex + 1] = 0;
      print_str(inputString + strIndex);

      if (inputString[strIndex] == '\n') {
	inputString[strIndex] = 0;
	parseInput();
	break;
      }
      strIndex++;
    }
  }

  sys_exit();
}

void parseInput(void) {
  int operand1 = 0;
  int operand2 = 0;
  char operator = 0;
  int i = 0;
  int result;

  //parse first operand
  while (inputString[i] != ' ') {
    if (inputString[i] <= '9' && inputString[i] >= '0') {
      operand1 *= 10;
      operand1 += inputString[i] - '0';
      i++;
    } else {
      print_str("Invalid command.\n");
      return;
    }
  }

  //skip the space
  i++;

  //parse the opperator
  if (inputString[i]) {
    operator = inputString[i];
    i++;
  } else {
    print_str("Invalid command.\n");
    return;
  }

  //skip the next space
  if (inputString[i] == ' ') {
    i++;
  } else {
    print_str("Invalid command.\n");
    return;
  }

  //parse second operand
  while (inputString[i] != 0) {
    if (inputString[i] <= '9' && inputString[i] >= '0') {
      operand2 *= 10;
      operand2 += inputString[i] - '0';
      i++;
    } else {
      print_str("Invalid command.\n");
      return;
    }
  }

  switch(operator) {
  case '+':
    result = operand1 + operand2;
    break;
  case '-':
    result = operand1 - operand2;
    break;
  case '*':
    result = operand1 * operand2;
    break;
  case '/':
    result = operand1 / operand2;
    break;
  default:
    print_str("Invalid operator.\n");
    return;
  }

  i = 19;
  outputString[i] = 0;
  i--;
  outputString[i] = '\n';
  i--;

  do {
    outputString[i] = result % 10 + '0';
    result = result / 10;
    i--;
   } while (result != 0);

  print_str(outputString + i + 1);
}

void print_str(const char *s) {
  asm volatile ("movl $1, %%eax;movl %0, %%ebx;int $0x80"
		:
		:"r"(s)
		:"%eax","%ebx");
}

char get_ch(void) {
  char c;
  asm volatile("movl $2, %%eax;movl %0, %%ebx;int $0x80"
	       :
	       :"r"(&c)
	       :"%eax","%ebx","memory");
  return c;
}

void sys_exit(void) {
  asm volatile ("movl $0, %%eax;int $0x80" : : : "%eax");
}
