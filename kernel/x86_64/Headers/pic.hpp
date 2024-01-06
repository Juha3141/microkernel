#ifndef _x86_64_PIC_HPP_
#define _x86_64_PIC_HPP_

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA    0x21
#define PIC_SLAVE_COMMAND  0xA0
#define PIC_SLAVE_DATA     0xA1

#define PIC_ICW1_ICW4_P    0x01
#define PIC_ICW1_SINGLE    0x02 // cascade mode
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL     0x08
#define PIC_ICW1_INIT      0x10

#define PIC_ICW4_8086        0x01 // 8086 Mode
#define PIC_ICW4_AUTO        0x02 // Auto EOI
#define PIC_ICW4_BUF_SLAVE   0x08
#define PIC_ICW4_BUF_MSATER  0x0C
#define PIC_ICW4_SFNM        0x10

namespace x86_64 {
    namespace pic {
        void init(void);
        void disable(void);

        void irq_mask(int irq);
        void irq_unmask(int irq);

        void send_EOI(bool master);
    }
}

#endif