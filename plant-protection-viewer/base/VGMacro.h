#ifndef __VGMACRO_H_
#define __VGMACRO_H_

#define SINGLE_INSTANCE_PORT   11499
#define VG_APPLICATION_NAME "VGPlantControl"
#define VG_ORG_NAME         "VGPlantControl.org"
#define VG_ORG_DOMAIN       "org.vgplantcontrol"

#define SETTING_FILE_NAME "config.ini"  //配置文件名

#define HEART_TIMEOUT_COUNT     11      //心跳超时次数
#define HEART_START_TIME        10      //启动心跳的时间(秒)
#define HEART_INTERVAL          5       //心跳频率(秒)
#define MINDISTANCE             0.5     //经纬度最小变化距离（精度，单位米）， 低于该值时地图不刷新
#define SYNC_TIMEOUT            60      //航线同步无人机应答超时时间(s)
#define MYPLANE_STATUS_TIMEOUT  4		//我的飞机状态查询定时器


#define OPEN_SPRINKLER -2000            //打开喷头
#define CLOSE_SPRINKLER 2000            //关闭喷头
#define PORT_SPRINKLER 5                //控制端口

#define VGSHeartbeatRateMSecs 1000

#endif // __VGMACRO_H_

