package com.example.xuhao.letgrow;

import android.animation.ValueAnimator;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.StrictMode;
import android.util.Log;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.dd.CircularProgressButton;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.charset.Charset;
import java.util.Calendar;

import com.github.mummyding.colorpickerdialog.ColorPickerDialog;
import com.github.mummyding.colorpickerdialog.OnColorChangedListener;

public class MainActivity extends BaseActivity implements View.OnClickListener {

    //全局变量和控件
    private CircularProgressButton circularButton_connection;
    private CircularProgressButton circularButton_watering;
    private CircularProgressButton circularButton_servoRollback;
    private CircularProgressButton circularButton_openLED;
    private CircularProgressButton circularButton_correctTime;
    private TextView textView_soilTemperature;
    private TextView textView_soilHumidity;
    private TextView textView_airTemperature;
    private TextView textView_airHumidity;
    private TextView textView_waterADC;
    private TextView textView_illumination_1;
    private TextView textView_illumination_2;
    // colors you want to add,colors must not be null!!!
    int[] colors = new int[]{Color.YELLOW, Color.BLACK, Color.BLUE, Color.GRAY,
            Color.GREEN, Color.CYAN, Color.RED, Color.DKGRAY, Color.LTGRAY, Color.MAGENTA,
            Color.rgb(100, 22, 33), Color.rgb(82, 182, 2), Color.rgb(122, 32, 12), Color.rgb(82, 12, 2),
            Color.rgb(89, 23, 200), Color.rgb(13, 222, 23), Color.rgb(222, 22, 2), Color.rgb(2, 22, 222)};
    //传感器数据变量
    float soilTemperature;
    float soilHumidity;
    int airTemperature;
    int airHumidity;
    int waterADC;
    int illumination_1;
    int illumination_2;
    boolean ledIsOpen = false;
    //Socket线程相关
    InputStream in;
    PrintWriter printWriter = null;
    Socket mSocket = null;
    public boolean isConnected = false;
    private MyHandler myHandler = new MyHandler();
    Thread receiverThread;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityCollector.addActivity(this);
        setContentView(R.layout.activity_main);
        //解决android.os.NetworkOnMainThreadException
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        widgetInit();///控件初始化
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ActivityCollector.removeActivity(this);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        ActivityCollector.finishAllActivity();
    }


    //控件初始化
    private void widgetInit() {
        //init widget
        circularButton_connection = (CircularProgressButton) findViewById(R.id.circularButton_connection);
        circularButton_watering = (CircularProgressButton) findViewById(R.id.circularButton_watering);
        circularButton_servoRollback = (CircularProgressButton) findViewById(R.id.circularButton_servoRollback);
        circularButton_openLED = (CircularProgressButton) findViewById(R.id.circularButton_openLED);
        circularButton_correctTime = (CircularProgressButton) findViewById(R.id.circularButton_correctTime);
        textView_soilTemperature = (TextView) findViewById(R.id.textView_soilTemperature);
        textView_soilHumidity = (TextView) findViewById(R.id.textView_soilHumidity);
        textView_airTemperature = (TextView) findViewById(R.id.textView_airTemperature);
        textView_airHumidity = (TextView) findViewById(R.id.textView_airHumidity);
        textView_waterADC = (TextView) findViewById(R.id.textView_waterADC);
        textView_illumination_1 = (TextView) findViewById(R.id.textView_illumination_1);
        textView_illumination_2 = (TextView) findViewById(R.id.textView_illumination_2);

        circularButton_connection.setOnClickListener(this);
        circularButton_watering.setOnClickListener(this);
        circularButton_servoRollback.setOnClickListener(this);
        circularButton_openLED.setOnClickListener(this);
        circularButton_correctTime.setOnClickListener(this);

        //隐藏一些控件
        circularButton_servoRollback.setVisibility(View.GONE);
    }

    //将接收到的数据进行处理并显示
    public void dataDisplay(String result) {
        //字符串判断
        if (result.trim().replace("\r|\n", "").equals("RequestConnection")) {
            sendData("Yes" + '\r' + '\n');
        } else if (result.charAt(0) == 'D' && result.charAt(result.length() - 3) == 'd') {
            Log.i("dataDisplay", "字符串分解：");
            soilTemperature = Float.parseFloat(result.split("\r\n|=")[2]);
            soilHumidity = Float.parseFloat(result.split("\r\n|=")[4]);
            airTemperature = Integer.parseInt(result.split("\r\n|=")[6]);
            airHumidity = Integer.parseInt(result.split("\r\n|=")[8]);
            waterADC = Integer.parseInt(result.split("\r\n|=")[10]);
            illumination_1 = Integer.parseInt(result.split("\r\n|=")[12]);
            illumination_2 = Integer.parseInt(result.split("\r\n|=")[14]);

            textView_soilTemperature.setText("土壤温度: " + String.valueOf(soilTemperature) + " ℃");
            textView_soilHumidity.setText("土壤湿度: " + String.valueOf(soilHumidity) + " %");
            textView_airTemperature.setText("空气温度: " + String.valueOf(airTemperature) + " ℃");
            textView_airHumidity.setText("空气湿度: " + String.valueOf(airHumidity) + " %");
            textView_illumination_1.setText("光照1: " + String.valueOf(illumination_1) + " lux");
            textView_illumination_2.setText("光照2: " + String.valueOf(illumination_2) + " lux");
            double temp = waterADC / 4096 * 3.3;
            textView_waterADC.setText("水位: ");
            if (temp >= 1.6) {
                textView_waterADC.append("满");
            } else if (temp >= 1.5 && temp < 1.6) {
                textView_waterADC.append("高");
            } else if (temp >= 1.4 && temp < 1.5) {
                textView_waterADC.append("中");
            } else if (temp < 1.4) {
                textView_waterADC.append("低");
            }
        } else if (result.trim().replace("\r|\n", "").equals("InstructionReceived")) {
            Toast.makeText(MainActivity.this, "指令接收正常", Toast.LENGTH_SHORT).show();
        }
    }


    //事件监听器
    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.circularButton_connection:
                connectThread();
                break;
            case R.id.circularButton_watering:
                sendData("PUMP_Open" + '\r' + '\n');
                circularButtonDisplay(circularButton_watering);
                break;
            case R.id.circularButton_servoRollback:
                sendData("ServoRollBack" + '\r' + '\n');
                circularButtonDisplay(circularButton_servoRollback);
                break;
            case R.id.circularButton_openLED:
                if (ledIsOpen == false) {
                    ColorPickerDialog dialog =
                        // Constructor,the first argv is Context,second one is the colors you want to add
                        new ColorPickerDialog(MainActivity.this, colors)
                            // Optional, if you want the dialog dismissed after picking,set it to true,otherwise
                            // false. default true
                            .setDismissAfterClick(false)
                            // Optional, Dialog's title,default "Theme"
                            .setTitle("自定义颜色")
                            //Optional, current checked color
                            .setCheckedColor(Color.BLACK)
                            .setOnColorChangedListener(new OnColorChangedListener() {
                                @Override
                                public void onColorChanged(int newColor) {
                                    // do something here
                                    Toast.makeText(getApplicationContext(), "颜色设置为：" + Integer.toHexString(newColor), Toast.LENGTH_SHORT).show();
                                    sendData("setWS2812BColor=#"
                                            + Integer.toHexString(newColor).substring(Integer.toHexString(newColor).length() - 6) + '\r' + '\n');
                                }
                            })
                            // build Dialog,argv means width count of Dialog,default value is 4 if you use build()
                            // without argv
                            .build(6)
                            //.setDismissAfterClick(true)
                            .show();
                    ledIsOpen = true;
                } else if (ledIsOpen == true) {
                    sendData("closeLED" + '\r' + '\n');
                    ledIsOpen = false;
                }
                circularButtonDisplay(circularButton_openLED);
                break;

            case R.id.circularButton_correctTime:
                Calendar calendar = Calendar.getInstance();
                sendData("setTime=" + "#" + calendar.get(Calendar.YEAR) + "#" + Integer.toString(calendar.get(Calendar.MONTH) + 1)
                        + "#" + calendar.get(Calendar.DAY_OF_MONTH) + "#" + calendar.get(Calendar.HOUR_OF_DAY) + "#"
                        + calendar.get(Calendar.MINUTE) + "#" + calendar.get(Calendar.SECOND) + '\r' + '\n');
                circularButtonDisplay(circularButton_correctTime);
                Log.i("onClick()", "setTime=" + "#" + calendar.get(Calendar.YEAR) + "#" + Integer.toString(calendar.get(Calendar.MONTH) + 1)
                        + "#" + calendar.get(Calendar.DAY_OF_MONTH) + "#" + calendar.get(Calendar.HOUR_OF_DAY)
                        + "#" + calendar.get(Calendar.MINUTE) + "#" + calendar.get(Calendar.SECOND) + '\r' + '\n');
                break;

            default:
                break;
        }
    }


    //建立连接按钮的线程
    private void connectThread() {
        if (!isConnected) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Looper.prepare();
                    Log.i(getClass().getSimpleName(), "---->> connect/close server!");
                    connectServer("115.29.109.104", "6578");//单片机模块的TCP协议 ip：192.168.4.1 端口：9000
                    Looper.loop();
                }
            }).start();
        } else {
            try {
                if (mSocket != null) {
                    mSocket.close();
                    mSocket = null;
                    Log.i("connectThread()", "--->>重新连server.");
                    //button_connection.setText("连接");
                    circularButton_connection.setProgress(0);//动态进度条字符串设置为“连接”
                    Toast.makeText(MainActivity.this, "连接已断开", Toast.LENGTH_SHORT).show();
                }
            } catch (IOException e) {
                Log.e("connectThread()", e.toString());
            }
            isConnected = false;
        }
    }

    //与服务器连接
    private void connectServer(String ip, String port) {
        try {
            Log.e("connectServer()", "--->>start connect  server !" + ip + "," + port);
            mSocket = new Socket(ip, Integer.parseInt(port));
            Log.e("connectServer()", "--->>end connect  server!");

            OutputStream outputStream = mSocket.getOutputStream();

            printWriter = new PrintWriter(new BufferedWriter(new OutputStreamWriter(outputStream, Charset.forName("ascii"))));

            in = mSocket.getInputStream();
            myHandler.sendEmptyMessage(2);
            Toast.makeText(MainActivity.this, "连接服务器成功", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            isConnected = false;
            Toast.makeText(MainActivity.this, "连接服务器失败", Toast.LENGTH_SHORT).show();
            Log.e("connectServer()", "exception:" + e.toString());
        }
    }


    public String readFromInputStream(InputStream in) {
        int count = 0;
        byte[] inDatas = null;
        try {
            while (count == 0) {
                count = in.available();
            }
            inDatas = new byte[count];
            in.read(inDatas);
            return new String(inDatas, "ascii");
        } catch (Exception e) {
            Log.e("readFromInputStream()", e.toString());
        }
        return null;
    }


    private void receiverData(int flag) {
        if (flag == 2) {
            receiverThread = new Thread(new MyReceiverRunnable());
            receiverThread.start();
            Log.i("receiverData()", "--->>socket 可以通信!");
            //button_connection.setText("已连接");
            isConnected = true;
            circularButtonDisplay(isConnected);//调用动态进度条
        }
    }


    //数据发送
    private void sendData(String context) {

        try {
            if (printWriter == null || context == null) {

                if (printWriter == null) {
                    Toast.makeText(MainActivity.this, "发送出错", Toast.LENGTH_SHORT);
                    return;
                }
                if (context == null) {
                    Toast.makeText(MainActivity.this, "发送为空", Toast.LENGTH_SHORT);
                    return;
                }
            }
            printWriter.print(context);
            printWriter.flush();
            Log.i("sendData()", "--->> client send data:" + context.toString());

        } catch (Exception e) {
            Log.e("sendData()", "--->> send failure!--->>" + e.toString());
        }
    }


    //创建实现Runnable接口的私有MyReceiverRunnable线程
    private class MyReceiverRunnable implements Runnable {

        public void run() {

            //线程要执行的操作
            while (true) {

                Log.i("MyReceiverRunnable--", "---->>client receive....");
                if (isConnected) {
                    if (mSocket != null && mSocket.isConnected()) {

                        String result = readFromInputStream(in); //通过从输入流读取数据，返回给result

                        try {
                            if (!result.equals("")) {

                                Message msg = new Message();  //获取消息
                                msg.what = 1;//设置message的what属的
                                Bundle data = new Bundle();
                                data.putString("msg", result);
                                msg.setData(data);
                                myHandler.sendMessage(msg);   //发消息
                            }

                        } catch (Exception e) //捕获异常，要做如下处
                        {
                            Log.e("MyReceiverRunnable--", "--->>read failure!" + e.toString());
                        }
                    }
                }
                try {
                    Thread.sleep(500L);    //线程休眠
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    Log.e("MyReceiverRunnable--", e.toString());
                }

            }
        }
    }


    private class MyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            receiverData(msg.what);
            if (msg.what == 1) {
                String result = msg.getData().get("msg").toString();
                //editText_receive.append(result);
                Log.i("MyHandler--", "Received:" + result);
                dataDisplay(result);//进行数据处理和显示
            }
        }
    }


    //动态进度条相关
    private void circularButtonDisplay(boolean flag) {
        if (circularButton_connection.getProgress() == 0) {
            if (flag == true) {
                simulateSuccessProgress(circularButton_connection);
            } else if (flag == false) {
                simulateErrorProgress(circularButton_connection);
            }
        } else {
            circularButton_connection.setProgress(0);
        }
    }

    private void circularButtonDisplay(CircularProgressButton circularButton) {
        if (circularButton.getProgress() == 0) {
            circularButton.setProgress(100);
        } else {
            circularButton.setProgress(0);
        }
    }

    //进度条显示成功
    private void simulateSuccessProgress(final CircularProgressButton button) {
        ValueAnimator widthAnimation = ValueAnimator.ofInt(1, 100);
        widthAnimation.setDuration(1500);
        widthAnimation.setInterpolator(new AccelerateDecelerateInterpolator());
        widthAnimation.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                Integer value = (Integer) animation.getAnimatedValue();
                button.setProgress(value);
            }
        });
        widthAnimation.start();
    }

    //进度条显示失败
    private void simulateErrorProgress(final CircularProgressButton button) {
        ValueAnimator widthAnimation = ValueAnimator.ofInt(1, 99);
        widthAnimation.setDuration(1500);
        widthAnimation.setInterpolator(new AccelerateDecelerateInterpolator());
        widthAnimation.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                Integer value = (Integer) animation.getAnimatedValue();
                button.setProgress(value);
                if (value == 99) {
                    button.setProgress(-1);
                }
            }
        });
        widthAnimation.start();
    }

}
