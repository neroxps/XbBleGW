#include "Lock.h"
/* Lock 消息结构如下
// topic is "event"
// {
//     "time":{
//         "timestamp":1607515347,
//         "formatTime":"2020-12-09 20:06:22"
//     },
//     "option":{
//         "action":"门外开锁",
//         "method":"生物特征"
//     },
//     "keyID":{
//         "value":"0x80010000",
//         "alertMsg":"多种方式频繁开锁失败"
//     }  
// }
//
// topic is attributes
// {
//     "state": "反锁+锁舌弹出",
//     "time":{
//         "timestamp":1607515347,
//         "formatTime":"2020-12-09 20:06:22"
//     }
// }
*/

Lock::Lock(): json_(500) {}

bool Lock::getTimestamp_(const char *edata){
  JsonObject time = this->json_.createNestedObject("time");
  char timestampString[9] = {0};
  char formatTime[40];
  strncpy(timestampString, edata, 8);
  if (strlen(timestampString) == 0){
    Debug::AddInfo(PSTR("[Lock::getTimestamp_] Get timestamp string failed. edata: %s"), edata);
    return false;
  }
  time_t rawtime = strtol(timestampString, NULL, 16);
  struct tm *timeinfo = localtime(&rawtime);
  strftime(formatTime,40,"%Y-%m-%d %H:%M:%S",timeinfo);
  if(strlen(formatTime) == 0 ){
    Debug::AddInfo(PSTR("[Lock::getTimestamp_] Format time failed. edata: %s"), edata);
    return false;
  }
  time["timestamp"] = (int)rawtime;
  time["formatTime"] = formatTime;
  return true;
}

bool Lock::keyIDHandle_(const char *edata){
  JsonObject keyID = this->json_.createNestedObject("keyID");
  char keyIdString[9] = {0};
  strncpy(keyIdString, edata+8, 8);
  if(strlen(keyIdString) == 0){
    Debug::AddInfo(PSTR("[Lock::keyIDHandle_] Get keyId string failed. edata: %s"), edata);
    return false;
  }
  keyID["value"] = keyIdString;
  // 获取 0x80010000 的头部数据 8001
  char keyIDHead[5] = {0};
  // 获取 0x80010000 尾部数据 0000
  char keyIDEnd[5] = {0};
  strncpy(keyIDHead, keyIdString+3, 4);
  strncpy(keyIDEnd, keyIdString+7, 4);
  uint16_t head = strtol(keyIDHead, NULL, 16);
  uint16_t end = strtol(keyIDEnd, NULL, 16);
  switch (head)
  {
  case 0xFFFF:
    keyID["description"] = "未知操作者";
    break;
  case 0xDEAD:
    keyID["description"] = "无效操作者";
    break;
  case 0x8001:
    keyID["description"] = "指纹";
    break;
  case 0x8002:
    keyID["description"] = "密码";
    break;
  case 0x8003:
    keyID["description"] = "钥匙";
    break;
  case 0x8004:
    keyID["description"] = "NFC";
    break;
  case 0x8005:
    keyID["description"] = "双重验证";
    break;
  case 0x8006:
    keyID["description"] = "人脸";
    break;
  case 0x8007:
    keyID["description"] = "指静脉";
    break;
  case 0x8008:
    keyID["description"] = "掌纹";
    break;
  case 0xC0DE:
    switch (end)
    {
    case 0x0000:
      keyID["description"] = "错误密码频繁开锁";
      break;
    case 0x0001:
      keyID["description"] = "错误指纹频繁开锁";
      break;
    case 0x0002:
      keyID["description"] = "密码输入超时";
      break;
    case 0x0003:
      keyID["description"] = "撬锁";
      break;
    case 0x0004:
      keyID["description"] = "重置按键按下";
      break;
    case 0x0005:
      keyID["description"] = "错误钥匙频繁开锁";
      break;
    case 0x0006:
      keyID["description"] = "钥匙孔异物";
      break;
    case 0x0007:
      keyID["description"] = "钥匙未取出";
      break;
    case 0x0008:
      keyID["description"] = "错误NFC频繁开锁";
      break;
    case 0x0009:
      keyID["description"] = "超时未按要求上锁";
      break;
    case 0x000A:
      keyID["description"] = "多种方式频繁开锁失败";
      break;
    case 0x000B:
      keyID["description"] = "人脸频繁开锁失败";
      break;
    case 0x000C:
      keyID["description"] = "静脉频繁开锁失败";
      break;
    case 0x000D:
      keyID["description"] = "劫持报警";
      break;
    case 0x000E:
      keyID["description"] = "布防后门内开锁";
      break;
    case 0x000F:
      keyID["description"] = "掌纹频繁开锁失败";
      break;
    case 0x0010:
      keyID["description"] = "保险箱被移动";
      break;
    case 0x1000:
      keyID["description"] = "电量低于10%";
      break;
    case 0x1001:
      keyID["description"] = "电量低于5%";
      break;
    case 0x1002:
      keyID["description"] = "指纹传感器异常";
      break;
    case 0x1003:
      keyID["description"] = "配件电池电量低";
      break;
    case 0x1004:
      keyID["description"] = "机械故障";
      break;
    case 0x1005:
      keyID["description"] = "锁体传感器故障";
      break;
    default:
      keyID["description"] = "unknow";
      break;
    }
  default:
  {
    if (head == 0x0000){
      keyID["description"] = "锁的管理员";
      break;
    }
    if (head < 0x8000){
      keyID["description"] = "蓝牙";
      break;
    }
    keyID["description"] = "unknow";
    break;
  }
  }
  return true;
}

bool Lock::operateHandle_(const char *edata){
  JsonObject option = this->json_.createNestedObject("option");
  char operateString[3] = {0};
  strncpy(operateString, edata+16, 2);
  uint8_t operateCode = strtol(operateString, NULL, 16);
  uint8_t action = operateCode & 0x0F;
  uint8_t method = operateCode >> 4;

  // method 开锁方式
  switch (method){
  case 0x00:
    option["method"] = "蓝牙";
    break;
  case 0x01:
    option["method"] = "密码";
    break;
  case 0x02:
    option["method"] = "指纹";
    break;
  case 0x03:
    option["method"] = "钥匙";
    break;
  case 0x04:
    option["method"] = "转盘";
    break;
  case 0x05:
    option["method"] = "NFC";
    break;
  case 0x06:
    option["method"] = "一次性密码";
    break;
  case 0x07:
    option["method"] = "双重验证";
    break;
  case 0x08:
    option["method"] = "胁迫";
    break;
  case 0x0A:
    option["method"] = "人工";
    break;
  case 0x0B:
    option["method"] = "自动";
    break;
  case 0x0F:
    option["method"] = "异常";
    break;
  default:
    Debug::AddInfo(PSTR("[Lock::operateHandle_] method is unknown. edata: %s"), edata);
    return false;
  }

  // action 开锁动作
  switch (action)
  {
  case 0x00:
    option["action"] = "门外开锁";
    break;
  case 0x01:
    option["action"] = "门外上锁";
    break;
  case 0x02:
    option["action"] = "开启反锁";
    break;
  case 0x03:
    option["action"] = "解除反锁";
    break;
  case 0x04:
    option["action"] = "门内开锁";
    break;
  case 0x05:
    option["action"] = "门内上锁";
    break;
  case 0x06:
    option["action"] = "开启童锁";
    break;
  case 0x07:
    option["action"] = "关闭童锁";
    break;
  case 0xFF:
    option["action"] = "异常";
    break;
  default:
    Debug::AddInfo(PSTR("[Lock::operateHandle_] action is unknown. edata: %s"), edata);
    return false;
  }
  return true;
}

void Lock::mqttPublish_(int deviceID, const char *eidName, const char *payload){
  char topic[80];
  String did = String(deviceID);
  sprintf(topic,"%s/%s",Mqtt::getStatTopic(did).c_str(),eidName);
  Mqtt::publish(topic, payload, globalConfig.mqtt.retain);
}

bool Lock::eventHandle_(int did, const char *eidName, const char *edata){
  if (strlen(edata) < 18){
    Debug::AddInfo(PSTR("[Lock::eventHandle_] The length of edata is wrong. edata: %s"), edata);
    return false;
  }
  if(!this->getTimestamp_(edata)){
    return false;
  }
  if(!this->keyIDHandle_(edata)){
    return false;
  }
  if(!this->operateHandle_(edata)){
    return false;
  }
  char compressJson[measureJson(this->json_)+1] = {0};
  this->mqttPublish_(did, eidName, compressJson);
  return true;
}

bool Lock::attributesHandle_(int did, const char *eidName, const char *edata){
  // 有可能属性的 edata 里面有时间戳
  if ((strlen(edata) != 10) || (strlen(edata) != 2)){
    Debug::AddInfo(PSTR("[Lock::attributesHandle_] The length of edata is wrong. edata: %s"), edata);
    return false;
  }
  char data[3] = {0};
  if (strlen(edata) == 10){
    if(!this->getTimestamp_(edata)){
      return false;
    }
    strncpy(data, edata+8, 2);
    if (strlen(data) == 0){
      return false;
    }
  }
  else {
    strcpy(data, edata);
    if (strlen(data) == 0){
      return false;
    }
  }
  int dataNum = strtol(data, NULL, 10);
  switch (dataNum)
  {
  case 0:
    this->json_["state"] = "开锁状态";
    break;
  case 4:
    this->json_["state"] = "锁舌弹出";
    break;
  case 5:
    this->json_["state"] = "上锁+锁舌弹出";
    break;
  case 6:
    this->json_["state"] = "反锁+锁舌弹出";
    break;
  case 7:
    this->json_["state"] = "所有锁舌弹出";
    break;
  default:
    Debug::AddInfo(PSTR("[Lock::attributesHandle_] Lock attributes is unknow. edata: %s"), edata);
    return false;
    break;
  }

}

bool Lock::handle(int did, int eid, const char *edata){
  switch (eid)
  {
  case 11:
    return this->eventHandle_(did, "event", edata);
  case 4110:
    return this->attributesHandle_(did, "attributes", edata);
  }
  return false;
}