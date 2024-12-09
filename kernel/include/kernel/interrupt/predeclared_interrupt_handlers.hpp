#ifndef _PREDECLARED_INTERRUPT_HANDLERS_HPP_
#define _PREDECLARED_INTERRUPT_HANDLERS_HPP_

#include <kernel/essentials.hpp>

#ifdef CONFIG_USE_INTERRUPT

#define INTERRUPT_GENERAL_INT_WRAPPER_MAXCOUNT         256
#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_MAXCOUNT  32

#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(handler_num) interrupt::handler::hardware_specified##handler_num
#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(handler_num) __attribute__ ((naked)) void hardware_specified##handler_num(void);

#define INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(handler_num) interrupt::handler::general_wrapper##handler_num
#define INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(handler_num) __attribute__ ((naked)) void general_wrapper##handler_num(void);

#define INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_ARRAY \
interrupt_handler_t hardware_specified_wrapper_array[INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_MAXCOUNT] = {\
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(0) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(1) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(2) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(3) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(4) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(5) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(6) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(7) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(8) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(9) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(10) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(11) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(12) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(13) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(14) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(15) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(16) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(17) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(18) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(19) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(20) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(21) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(22) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(23) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(24) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(25) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(26) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(27) , \
    (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(28) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(29) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(30) , (interrupt_handler_t)INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER_PTR(31) \
};


#define INTERRUPT_GENERAL_INT_WRAPPER_ARRAY \
interrupt_handler_t general_int_wrapper_array[INTERRUPT_GENERAL_INT_WRAPPER_MAXCOUNT] = {\
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(0) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(1) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(2) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(3) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(4) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(5) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(6) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(7) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(8) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(9) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(10) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(11) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(12) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(13) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(14) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(15) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(16) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(17) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(18) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(19) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(20) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(21) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(22) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(23) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(24) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(25) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(26) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(27) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(28) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(29) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(30) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(31) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(32) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(33) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(34) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(35) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(36) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(37) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(38) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(39) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(40) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(41) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(42) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(43) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(44) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(45) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(46) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(47) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(48) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(49) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(50) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(51) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(52) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(53) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(54) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(55) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(56) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(57) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(58) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(59) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(60) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(61) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(62) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(63) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(64) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(65) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(66) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(67) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(68) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(69) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(70) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(71) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(72) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(73) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(74) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(75) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(76) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(77) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(78) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(79) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(80) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(81) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(82) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(83) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(84) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(85) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(86) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(87) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(88) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(89) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(90) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(91) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(92) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(93) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(94) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(95) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(96) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(97) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(98) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(99) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(100) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(101) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(102) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(103) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(104) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(105) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(106) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(107) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(108) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(109) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(110) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(111) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(112) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(113) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(114) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(115) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(116) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(117) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(118) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(119) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(120) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(121) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(122) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(123) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(124) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(125) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(126) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(127) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(128) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(129) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(130) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(131) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(132) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(133) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(134) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(135) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(136) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(137) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(138) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(139) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(140) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(141) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(142) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(143) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(144) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(145) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(146) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(147) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(148) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(149) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(150) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(151) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(152) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(153) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(154) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(155) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(156) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(157) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(158) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(159) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(160) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(161) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(162) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(163) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(164) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(165) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(166) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(167) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(168) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(169) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(170) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(171) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(172) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(173) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(174) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(175) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(176) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(177) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(178) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(179) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(180) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(181) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(182) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(183) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(184) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(185) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(186) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(187) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(188) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(189) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(190) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(191) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(192) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(193) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(194) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(195) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(196) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(197) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(198) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(199) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(200) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(201) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(202) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(203) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(204) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(205) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(206) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(207) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(208) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(209) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(210) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(211) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(212) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(213) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(214) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(215) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(216) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(217) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(218) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(219) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(220) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(221) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(222) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(223) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(224) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(225) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(226) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(227) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(228) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(229) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(230) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(231) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(232) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(233) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(234) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(235) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(236) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(237) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(238) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(239) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(240) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(241) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(242) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(243) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(244) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(245) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(246) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(247) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(248) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(249) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(250) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(251) , \
    (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(252) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(253) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(254) , (interrupt_handler_t)INTERRUPT_GENERAL_INT_WRAPPER_HANDLER_PTR(255)\
};

namespace interrupt {
    namespace handler {
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(0);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(1);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(2);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(3);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(4);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(5);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(6);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(7);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(8);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(9);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(10);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(11);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(12);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(13);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(14);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(15);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(16);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(17);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(18);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(19);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(20);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(21);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(22);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(23);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(24);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(25);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(26);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(27);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(28);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(29);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(30);
        INTERRUPT_HARDWARE_SPECIFIED_WRAPPER_HANDLER(31);

        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(0);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(1);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(2);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(3);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(4);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(5);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(6);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(7);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(8);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(9);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(10);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(11);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(12);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(13);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(14);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(15);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(16);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(17);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(18);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(19);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(20);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(21);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(22);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(23);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(24);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(25);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(26);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(27);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(28);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(29);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(30);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(31);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(32);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(33);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(34);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(35);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(36);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(37);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(38);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(39);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(40);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(41);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(42);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(43);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(44);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(45);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(46);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(47);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(48);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(49);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(50);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(51);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(52);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(53);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(54);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(55);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(56);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(57);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(58);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(59);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(60);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(61);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(62);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(63);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(64);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(65);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(66);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(67);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(68);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(69);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(70);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(71);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(72);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(73);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(74);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(75);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(76);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(77);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(78);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(79);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(80);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(81);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(82);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(83);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(84);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(85);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(86);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(87);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(88);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(89);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(90);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(91);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(92);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(93);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(94);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(95);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(96);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(97);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(98);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(99);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(100);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(101);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(102);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(103);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(104);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(105);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(106);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(107);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(108);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(109);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(110);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(111);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(112);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(113);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(114);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(115);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(116);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(117);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(118);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(119);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(120);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(121);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(122);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(123);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(124);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(125);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(126);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(127);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(128);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(129);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(130);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(131);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(132);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(133);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(134);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(135);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(136);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(137);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(138);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(139);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(140);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(141);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(142);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(143);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(144);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(145);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(146);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(147);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(148);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(149);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(150);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(151);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(152);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(153);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(154);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(155);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(156);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(157);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(158);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(159);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(160);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(161);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(162);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(163);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(164);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(165);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(166);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(167);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(168);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(169);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(170);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(171);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(172);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(173);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(174);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(175);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(176);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(177);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(178);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(179);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(180);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(181);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(182);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(183);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(184);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(185);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(186);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(187);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(188);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(189);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(190);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(191);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(192);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(193);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(194);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(195);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(196);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(197);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(198);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(199);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(200);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(201);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(202);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(203);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(204);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(205);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(206);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(207);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(208);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(209);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(210);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(211);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(212);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(213);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(214);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(215);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(216);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(217);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(218);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(219);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(220);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(221);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(222);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(223);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(224);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(225);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(226);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(227);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(228);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(229);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(230);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(231);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(232);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(233);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(234);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(235);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(236);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(237);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(238);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(239);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(240);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(241);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(242);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(243);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(244);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(245);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(246);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(247);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(248);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(249);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(250);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(251);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(252);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(253);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(254);
        INTERRUPT_GENERAL_INT_WRAPPER_HANDLER(255);

        interrupt_handler_t get_hardware_specified_int_wrapper(int index);
        interrupt_handler_t get_general_int_wrapper(int index);
    };
};

#endif

#endif