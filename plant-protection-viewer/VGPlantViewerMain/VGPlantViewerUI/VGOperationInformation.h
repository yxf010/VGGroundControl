#ifndef VGOPERATIONINFORMATION_H
#define VGOPERATIONINFORMATION_H

#include <QObject>
//我的作业结构类

class VGOperationInformation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString actualDesId READ actualDesId WRITE setActualDesId NOTIFY actualDesIdChanged)
    Q_PROPERTY(QString operatorId READ operatorId WRITE setOperatorId NOTIFY operatorIdChanged)
    Q_PROPERTY(QString registerId READ registerId WRITE setRegisterId NOTIFY registerIdChanged)
    Q_PROPERTY(QString chargerId READ chargerId WRITE setChargerId NOTIFY chargerIdChanged)
    Q_PROPERTY(QString beginTime READ beginTime WRITE setBeginTime NOTIFY beginTimeChanged)
    Q_PROPERTY(QString endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
    Q_PROPERTY(QString operationType READ operationType WRITE setOperationType NOTIFY operationTypeChanged)
    Q_PROPERTY(QString actualOperationId READ actualOperationId WRITE setActualOperationId NOTIFY actualOperationIdChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString pesticideId READ pesticideId WRITE setPesticideId NOTIFY pesticideIdChanged)
    Q_PROPERTY(QString pesticideName READ pesticideName WRITE setPesticideName NOTIFY pesticideNameChanged)
    Q_PROPERTY(QString cropName READ cropName WRITE setCropName NOTIFY cropNameChanged)
    Q_PROPERTY(QString cropId READ cropId WRITE setCropId NOTIFY cropIdChanged)
    Q_PROPERTY(float serviceCharge READ serviceCharge WRITE setServiceCharge NOTIFY serviceChargeChanged)
    Q_PROPERTY(float pesticideCharge READ pesticideCharge WRITE setPesticideCharge NOTIFY pesticideChargeChanged)
    Q_PROPERTY(float otherCharge READ otherCharge WRITE setOtherCharge NOTIFY otherChargeChanged)
    Q_PROPERTY(float sumCharge READ sumCharge WRITE setSumCharge NOTIFY sumChargeChanged)

    Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)

public:
    explicit VGOperationInformation(QObject *parent = 0);
    VGOperationInformation(VGOperationInformation* info);

    QString actualDesId() { return m_actualDesId; }
    void setActualDesId(QString id) { m_actualDesId = id; emit actualDesIdChanged(id); }

    QString operatorId() { return m_operatorId; }
    void setOperatorId(QString lstOperator) { m_operatorId = lstOperator; emit operatorIdChanged(lstOperator); }

    QString registerId() { return m_registerId; }
    void setRegisterId(QString id) { m_registerId = id; emit registerIdChanged(id); }

    QString chargerId() { return m_chargerId; }
    void setChargerId(QString id) { m_chargerId = id; emit chargerIdChanged(id); }

    QString beginTime() { return m_beginTime; }
    void setBeginTime(QString time) { m_beginTime = time;emit beginTimeChanged(time);  }

    QString endTime() { return m_endTime; }
    void setEndTime(QString time) { m_endTime = time; emit endTimeChanged(time); }

    QString operationType() { return m_operationType; }
    void setOperationType(QString type) { m_operationType = type; emit operationTypeChanged(type); }

    QString actualOperationId() { return m_actualOperationId; }
    void setActualOperationId(QString id) { m_actualOperationId = id; emit actualOperationIdChanged(id); }

    QString notes() { return m_notes; }
    void setNotes(QString notes) { m_notes = notes; emit notesChanged(notes); }

    QString deviceId() { return m_deviceId; }
    void setDeviceId(QString id) { m_deviceId = id; emit deviceIdChanged(id); }

    QString deviceName() { return m_deviceName; }
    void setDeviceName(QString name) { m_deviceName = name; emit deviceNameChanged(name); }

    QString pesticideId() { return m_pesticideId; }
    void setPesticideId(QString id) { m_pesticideId = id; emit pesticideIdChanged(id); }

    QString pesticideName() { return m_pesticideName; }
    void setPesticideName(QString name) { m_pesticideName = name; emit pesticideNameChanged(name); }

    QString cropName() { return m_cropName; }
    void setCropName(QString name) { m_cropName = name; emit cropNameChanged(name); }

    QString cropId() { return m_cropId; }
    void setCropId(QString id) { m_cropId = id; emit cropIdChanged(id); }

    float serviceCharge() { return m_serviceCharge; }
    void setServiceCharge(float charge) { m_serviceCharge = charge; emit serviceChargeChanged(charge); }

    float pesticideCharge() { return m_pesticideCharge; }
    void setPesticideCharge(float charge) { m_pesticideCharge = charge; emit pesticideChargeChanged(charge); }

    float otherCharge() { return m_otherCharge; }
    void setOtherCharge(float charge) { m_otherCharge = charge; emit otherChargeChanged(charge); }

    float sumCharge() { return m_sumCharge; }
    void setSumCharge(float charge) { m_sumCharge = charge; emit sumChargeChanged(charge); }

    bool selected() { return m_bSelected; }
    void setSelected(bool selected) { m_bSelected = selected; emit selectedChanged(selected); }

private:
    QString m_actualDesId;          //地块描述ID
    QString m_operatorId;       //操作人员ID
    QString m_registerId;           //登记人员ID
    QString m_chargerId;            //负责人ID
    QString  m_beginTime;           //开始作业时间（计划）
    QString  m_endTime;
    QString m_operationType;        //作业类型
    QString m_actualOperationId;    //作业描述ID
    QString m_notes;                //备注
    QString m_deviceId;             //设备ID
    QString m_deviceName;           //设备名称
    QString m_pesticideId;          //农药ID
    QString m_pesticideName;        //农药规格
    QString m_cropName;             //农作物名称
    QString m_cropId;               //农作物ID
    float   m_serviceCharge;        //服务费用
    float   m_pesticideCharge;      //农药费用
    float   m_otherCharge;          //其它费用
    float   m_sumCharge;            //总费用

    bool    m_bSelected;            //是否选中
signals:
    void    operatorIdChanged(QString id);
    void    actualDesIdChanged(QString id);
    void    registerIdChanged(QString id);
    void    chargerIdChanged(QString id);
    void    beginTimeChanged(QString id);
    void    endTimeChanged(QString id);
    void    operationTypeChanged(QString id);
    void    actualOperationIdChanged(QString id);
    void    notesChanged(QString id);
    void    deviceIdChanged(QString id);
    void    deviceNameChanged(QString id);
    void    pesticideIdChanged(QString id);
    void    pesticideNameChanged(QString id);
    void    cropNameChanged(QString id);
    void    cropIdChanged(QString id);
    void    serviceChargeChanged(float id);
    void    pesticideChargeChanged(float id);
    void    otherChargeChanged(float id);
    void    sumChargeChanged(float id);    

    void    selectedChanged(bool selected);

public slots:

};

#endif // VGOPERATIONINFORMATION_H
