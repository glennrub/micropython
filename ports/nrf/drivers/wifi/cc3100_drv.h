/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Glenn Ruben Bakke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef CC3100_DRV_H
#define CC3100_DRV_H

typedef void (*irq_handler_t)(void *p_val);

void cc3100_drv_enable(void);
void cc3100_drv_disable(void);
void cc3100_drv_init(void *spi, const void *pin_cs, const void *pin_en, const void *pin_irq);
int cc3100_drv_open(char * interface, unsigned long flags);
int cc3100_drv_close(int handle);
int cc3100_drv_read(int handle, unsigned char * p_buffer, int len);
int cc3100_drv_write(int handle, unsigned char * p_buffer, int len);
int cc3100_drv_irq_handler_register(irq_handler_t handler, void * p_val);
void cc3100_drv_irq_handler_mask(void);
void cc3100_drv_irq_handler_unmask(void); 

#endif // CC3100_DRV_H
