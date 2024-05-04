#include "inv_mpu.h"
#include <math.h>
#include "Fall.h"


int SVM(float pitch,float roll,float yaw,float SVM,float SVA)
{
	int fall_value1,fall_value2,fall_value3,fall;
	
	if(fabs(pitch)>40 || fabs(roll)>40 || fabs(yaw)>40)
	{
		fall_value1 = 1; 
	}
	else
	{
		fall_value1 = 0;
	}
	

	if(fall_value1 || fall_value2 || fall_value3)
	{
		fall = 1;
	}	
	else 
	{
		fall = 0;
	}	
	return fall;
}
