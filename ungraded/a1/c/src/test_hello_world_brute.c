/*generate 16-bit code*/
__asm__(".code16gcc\n");
/*jump boot code entry*/
__asm__("jmpl $0x0000, $main\n");

void main() {
     /*print letter 'H' onto the screen*/
     __asm__ __volatile__("movb $'H' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'e' onto the screen*/
     __asm__ __volatile__("movb $'e' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'l' onto the screen*/
     __asm__ __volatile__("movb $'l' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'l' onto the screen*/
     __asm__ __volatile__("movb $'l' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'o' onto the screen*/
     __asm__ __volatile__("movb $'o' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter ',' onto the screen*/
     __asm__ __volatile__("movb $',' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter ' ' onto the screen*/
     __asm__ __volatile__("movb $' ' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'W' onto the screen*/
     __asm__ __volatile__("movb $'W' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'o' onto the screen*/
     __asm__ __volatile__("movb $'o' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'r' onto the screen*/
     __asm__ __volatile__("movb $'r' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'l' onto the screen*/
     __asm__ __volatile__("movb $'l' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");

     /*print letter 'd' onto the screen*/
     __asm__ __volatile__("movb $'d' , %al\n");
     __asm__ __volatile__("movb $0x0e, %ah\n");
     __asm__ __volatile__("int  $0x10\n");
}
