#if !defined(DEF_HID_TYPES_H)
#define DEF_HID_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_HID_NUM_OF_CALLBACKS	(uint16_t)(4)

typedef struct
{
	//Is button pressed.
	bool p_a;
	bool p_b;
	bool p_x;
	bool p_y;
	bool p_c_up;
	bool p_c_down;
	bool p_c_left;
	bool p_c_right;
	bool p_d_up;
	bool p_d_down;
	bool p_d_left;
	bool p_d_right;
	bool p_l;
	bool p_r;
	bool p_zl;
	bool p_zr;
	bool p_start;
	bool p_select;
	bool p_cs_up;
	bool p_cs_down;
	bool p_cs_left;
	bool p_cs_right;
	bool p_touch;
	bool p_any;
	//Is button held.
	bool h_a;
	bool h_b;
	bool h_x;
	bool h_y;
	bool h_c_up;
	bool h_c_down;
	bool h_c_left;
	bool h_c_right;
	bool h_d_up;
	bool h_d_down;
	bool h_d_left;
	bool h_d_right;
	bool h_l;
	bool h_r;
	bool h_zl;
	bool h_zr;
	bool h_start;
	bool h_select;
	bool h_cs_up;
	bool h_cs_down;
	bool h_cs_left;
	bool h_cs_right;
	bool h_touch;
	bool h_any;
	//Is button released.
	bool r_a;
	bool r_b;
	bool r_x;
	bool r_y;
	bool r_c_up;
	bool r_c_down;
	bool r_c_left;
	bool r_c_right;
	bool r_d_up;
	bool r_d_down;
	bool r_d_left;
	bool r_d_right;
	bool r_l;
	bool r_r;
	bool r_zl;
	bool r_zr;
	bool r_start;
	bool r_select;
	bool r_cs_up;
	bool r_cs_down;
	bool r_cs_left;
	bool r_cs_right;
	bool r_touch;
	bool r_any;
	//CPAD and touch position.
	int cpad_x;
	int cpad_y;
	int touch_x;
	int touch_y;
	int touch_x_move;
	int touch_y_move;
	int held_time;
	//Timestamp for this data.
	uint64_t ts;
} Hid_info;

#endif //!defined(DEF_HID_TYPES_H)
