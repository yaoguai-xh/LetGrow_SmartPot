package com.example.xuhao.letgrow;

import android.app.Activity;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by xuhao on 2018/2/26.
 */

public class ActivityCollector {
    //ActivityCollector 活动管理器
    public static List<Activity> activityList=new ArrayList<>();

    public static void addActivity(Activity activity){
        activityList.add(activity);
    }

    public static void removeActivity(Activity activity){
        activityList.remove(activity);
    }

    public static void finishAllActivity(){
        for (Activity activity:activityList){
            if (!activity.isFinishing()){
                activity.finish();
            }
        }
        android.os.Process.killProcess(android.os.Process.myPid());
    }

}
