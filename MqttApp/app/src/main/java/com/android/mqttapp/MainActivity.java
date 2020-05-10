package com.android.mqttapp;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;

import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;


import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class MainActivity extends AppCompatActivity {

    private  ImageView image1;
    private TextView  text1;
     private TextView  text2;
    private String host = "tcp://183.230.40.39:6002";
    private String userName = "340236";
    private String passWord = "wangdao123";
    private String Mqtt_Id="595358255";
  private String mqtt_sub_toptic="KFB_Topic";
  private String mqtt_pub_toptic="APP_Topic";
    private MqttClient client;
  //  private  int led_flag=0;
    private MqttConnectOptions options;
    private Handler handler;
    private ScheduledExecutorService scheduler;
    @SuppressLint("HandlerLeak")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        text1=(TextView) findViewById(R.id.text1);
        text2=(TextView)findViewById(R.id.text2);
        image1=(ImageView) findViewById(R.id.img_1);


        image1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                publishmessage(mqtt_pub_toptic,"open_led");


            }
        });
        init();
        startReconnect();
        handler = new Handler() {
            @SuppressLint("SetTextI18n")
            @Override
            public void handleMessage(Message msg) {
              super.handleMessage(msg);
              switch (msg.what){
                  case 1:
                      break;
                  case 2:
                      break;
                  case 3:
                    //  Toast.makeText(MainActivity.this,msg.obj.toString(),Toast.LENGTH_SHORT).show();
                     String tem_tx=msg.obj.toString().substring(msg.obj.toString().indexOf("temperature\"")+13,msg.obj.toString().indexOf(","));
                     String humi_tx=msg.obj.toString().substring(msg.obj.toString().indexOf("humity\"")+8,msg.obj.toString().indexOf("}"));
                     String tx_text="温度:"+tem_tx;
                     String tx1_text="湿度:"+humi_tx;
                      text1.setText(tx_text);
                      text2.setText(tx1_text);

                      break;
                  case 30:   //连接失败的case
                      Toast.makeText(MainActivity.this,"连接失败",Toast.LENGTH_SHORT).show();
                      break;
                  case 31:    //连接成功的case
                      Toast.makeText(MainActivity.this,"连接成功",Toast.LENGTH_SHORT).show();
                      try {
                          client.subscribe(mqtt_sub_toptic,1);
                      } catch (MqttException e) {
                          e.printStackTrace();
                      }
                      break;
                  default:
                      break;

              }
            }
        };



    }
    private void init() {
        try {
            //host为主机名，test为clientid即连接MQTT的客户端ID，一般以客户端唯一标识符表示，MemoryPersistence设置clientid的保存形式，默认为以内存保存
            client = new MqttClient(host, Mqtt_Id,
                    new MemoryPersistence());
            //MQTT的连接设置
            options = new MqttConnectOptions();
            //设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
            options.setCleanSession(true);
            //设置连接的用户名
            options.setUserName(userName);
            //设置连接的密码
            options.setPassword(passWord.toCharArray());
            // 设置超时时间 单位为秒
            options.setConnectionTimeout(10);
            // 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
            options.setKeepAliveInterval(20);
            //设置回调
            client.setCallback(new MqttCallback() {

                @Override
                public void connectionLost(Throwable cause) {
                    //连接丢失后，一般在这里面进行重连
                    System.out.println("connectionLost----------");
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    //publish后会执行到这里
                    System.out.println("deliveryComplete---------"
                            + token.isComplete());
                }

                @Override
                public void messageArrived(String topicName, MqttMessage message)
                        throws Exception {
                    //subscribe后得到的消息会执行到这里面
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    msg.what = 3;
                    msg.obj = topicName + "---" + message.toString();
                    handler.sendMessage(msg);
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    //开始连接的函数
    private void connect() {
        new Thread(new Runnable() {

            @Override
            public void run() {
                try {
                    if (!(client.isConnected())){
                        client.connect(options);
                        Message msg = new Message();
                        msg.what = 31;
                        handler.sendMessage(msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Message msg = new Message();
                    msg.what = 30;
                    handler.sendMessage(msg);
                }
            }
        }).start();
    }
    //重新开始连接的函数
    private void startReconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(new Runnable() {

            @Override
            public void run() {
                if (!client.isConnected()) {
                    connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

//    private void publishMessage(String toptic,String message){
//        if (client==null||!client.isConnected()) {
//            return;
//        }
//            MqttMessage mqttMessage=new MqttMessage();
//            mqttMessage.setPayload(message.getBytes());
//            try {
//                client.publish(toptic,mqttMessage);
//            } catch (MqttException e) {
//                e.printStackTrace();
//            }
//
//        }
private void publishmessage(String topic,String msg){
    Integer qos = 0;
    Boolean retained = false;
    try {
        if (client != null){
            client.publish(topic, msg.getBytes(), qos.intValue(), retained.booleanValue());
        }
    } catch (MqttException e) {
        e.printStackTrace();
    }
}

    }





