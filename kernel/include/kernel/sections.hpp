#ifndef _SECTION_HPP_
#define _SECTION_HPP_

#define __kernel_setup_text__ __attribute__ ((section(".kernel_setup_stage.text")))
#define __kernel_setup_data__ __attribute__ ((section(".kernel_setup_stage.data")))
#define __entry_function__        __attribute__ ((section(".entry")))
#define __singleton_ptr__         __attribute__ ((section(".singleton_obj_ptrs")))

#endif