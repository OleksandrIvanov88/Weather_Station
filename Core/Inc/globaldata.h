
#ifndef INC_GLOBALDATA_H_
#define INC_GLOBALDATA_H_

#define QNH_GOT_CHECK -20000


struct sensor_data_global {
	double temp;
	double press;
	double alt;
};
struct sensor_data_global sensor_data_bt;

struct metar_data_global {
	int QNH_m_report;
	int temp_m_report;
	int dewpoint_m_report;
	int visibility_m_report;
	int wind_direction_m_report;
	int wind_speed_m_report;
	char timestamp_m_report[32];
};
struct metar_data_global metar_data_bt;

struct taf_data_global {
	char timestamp_t_report[32];
	char timeissue_t_report[32];
	int visibility_t_report;
	int wind_direction_t_report;
	int wind_speed_t_report;
};
struct taf_data_global taf_data_bt;
int32_t QNH_global; //required for altitude calculation
#endif /* INC_GLOBALDATA_H_ */
