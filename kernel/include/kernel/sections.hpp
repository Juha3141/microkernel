#ifndef _SECTION_HPP_
#define _SECTION_HPP_

#define __entry_function__ __attribute__ ((section(".entry")))
#define __singleton_ptr__  __attribute__ ((section(".singleton_obj_ptrs")))

#endif