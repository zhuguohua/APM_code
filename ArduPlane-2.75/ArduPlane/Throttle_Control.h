#ifndef _THROTTLE_CONTROL_H_
#define _THROTTLE_CONTROL_H_

#include <RC_Channel.h>     // RC Channel Library

typedef int int16_t;

#define CURRENT_MAX_A 30
#define THROTTLE_PWM_OUT_MAX 1900
#define THROTTLE_PWM_OUT_MIN 1100
class Throttle_Control
{
public:
	Throttle_Control();
	Throttle_Control(RC_Channel *p_thr_ch_src);
	~Throttle_Control(){}

public:
	float f_ASpeed;
	float f_throttle_min;
	float f_throttle_max;
	int16_t i16_radio_min;
	int16_t i16_radio_max;
	int16_t i16_throttle_pwm_min;
	int16_t i16_throttle_pwm_max;
	int16_t i16_throttle_pwm_curr;

	// 每1%的油门对应的PWM值
	int16_t i16_pwm_of_each_thr_rang;

	// throttle channel
	RC_Channel *m_p_thr_ch;

public:
	void Set_Throttle_Channel(RC_Channel *p_thr_ch_src);
	void Set_Throttle_Range(float _f_throttle_min, float _f_throttle_max, int _i_throttle_reverse = 1);
	int16_t Get_Throttle_Pwm_Max();
	int16_t Get_Throttle_Pwm_Min();

	// airspeed base control the throttle
	int16_t Get_Throttle_Pwm_Out(float f_air_speed_curr, float f_air_speed_dst, int16_t i16_time_ms = 0);
	int16_t Get_Throttle_Pwm_Out_With_Time(float f_air_speed_curr, float f_air_speed_dst, int16_t i16_time_ms);
	int16_t Get_Throttle_Pwm_Out_With_Pid(float f_air_speed_curr, float f_air_speed_dst);

	int16_t Get_Throttle_Radio_Max();
	int16_t Get_Throttle_Radio_Min();
};


#endif
