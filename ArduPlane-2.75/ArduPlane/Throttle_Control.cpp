#include "Throttle_Control.h"

/*
* bool RC_Channel::get_reverse(void)
* ����1��ʾ���ŷ�ת
* �������Ų���ת
*/

Throttle_Control::Throttle_Control(RC_Channel *p_thr_ch_src)
{
	Set_Throttle_Channel(p_thr_ch_src);
	Throttle_Control();
}

Throttle_Control::Throttle_Control()
{
	//i16_radio_min = m_p_thr_ch->radio_min.get();
	//i16_radio_max = m_p_thr_ch->radio_max.get();

	i16_radio_min = THROTTLE_PWM_OUT_MIN;
	i16_radio_max = THROTTLE_PWM_OUT_MAX;

	i16_throttle_pwm_curr = 1600;//60%+�ĳ�ʼ����
	i16_pwm_of_each_thr_rang = (THROTTLE_PWM_OUT_MAX - THROTTLE_PWM_OUT_MIN) / 100;

	return;
}

void Throttle_Control::Set_Throttle_Range(float _f_throttle_min, float _f_throttle_max, int _i_throttle_reverse)
{
	if (_i_throttle_reverse)
	{
		f_throttle_min = 100 - _f_throttle_max;
		f_throttle_max = 100 - _f_throttle_min;
	}
	else
	{
		f_throttle_min = _f_throttle_min;
		f_throttle_max = _f_throttle_max;
	}

	i16_throttle_pwm_min = THROTTLE_PWM_OUT_MIN + (int)(f_throttle_min * i16_pwm_of_each_thr_rang);
	i16_throttle_pwm_max = THROTTLE_PWM_OUT_MIN + (int)(f_throttle_max * i16_pwm_of_each_thr_rang);
	//i16_throttle_pwm_min = m_p_thr_ch->radio_min.get() + (int)(f_throttle_min * i16_pwm_of_each_thr_rang);
	//i16_throttle_pwm_max = m_p_thr_ch->radio_min.get() + (int)(f_throttle_max * i16_pwm_of_each_thr_rang);

	return;
}

int16_t Throttle_Control::Get_Throttle_Pwm_Out(float f_air_speed_curr, float f_air_speed_dst, int16_t i16_time_ms)
{
	if (i16_time_ms != 0)
	{
		return Get_Throttle_Pwm_Out_With_Time(f_air_speed_curr, f_air_speed_dst, i16_time_ms);
	}
	else
	{
		return Get_Throttle_Pwm_Out_With_Pid(f_air_speed_curr, f_air_speed_dst);
	}
}

int16_t Throttle_Control::Get_Throttle_Pwm_Out_With_Time(float f_air_speed_curr, float f_air_speed_dst, int16_t i16_time_ms)
{
	if (f_air_speed_curr < f_air_speed_dst)
	{
		i16_throttle_pwm_curr = i16_throttle_pwm_curr + 4;// û4��PWMֵ��Ӧ0.5%�����ű仯
	}
	else if (f_air_speed_curr > f_air_speed_dst)
	{
		i16_throttle_pwm_curr = i16_throttle_pwm_curr - 4;
	}

	if (i16_throttle_pwm_curr < i16_throttle_pwm_min)
	{
		i16_throttle_pwm_curr = i16_throttle_pwm_min;
	}
	else if (i16_throttle_pwm_curr > i16_throttle_pwm_max)
	{
		i16_throttle_pwm_curr = i16_throttle_pwm_max;
	}

	return i16_throttle_pwm_curr;
}

int16_t Throttle_Control::Get_Throttle_Pwm_Out_With_Pid(float f_air_speed_curr, float f_air_speed_dst)
{


	return i16_throttle_pwm_curr;
}



int16_t Throttle_Control::Get_Throttle_Pwm_Max()
{
	return i16_throttle_pwm_max;
}


int16_t Throttle_Control::Get_Throttle_Pwm_Min()
{
	return i16_throttle_pwm_min;
}

void Throttle_Control::Set_Throttle_Channel(RC_Channel *p_thr_ch_src)
{
	m_p_thr_ch = p_thr_ch_src;
}

int16_t Throttle_Control::Get_Throttle_Radio_Max()
{
	m_p_thr_ch->radio_max.get();
}

int16_t Throttle_Control::Get_Throttle_Radio_Min()
{
	m_p_thr_ch->radio_min.get();
}


