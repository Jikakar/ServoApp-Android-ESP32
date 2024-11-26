package com.example.servoapp;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class MainActivity extends AppCompatActivity implements SensorEventListener {

    private String ipAddress = "192.168.4.1";
    private TextView currentAngleTextView, statusTextView;
    private SensorManager sensorManager;
    private Sensor gyroscopeSensor;
    private int currentAngle = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        currentAngleTextView = findViewById(R.id.currentAngleTextView);
        statusTextView = findViewById(R.id.statusTextView);
        Button resetButton = findViewById(R.id.resetButton);

        // Initialize SensorManager and gyroscope sensor
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        if (sensorManager != null) {
            gyroscopeSensor = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        }

        // Add click listener for reset button
        resetButton.setOnClickListener(v -> resetServoAngle());
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Register gyroscope listener
        if (gyroscopeSensor != null) {
            sensorManager.registerListener(this, gyroscopeSensor, SensorManager.SENSOR_DELAY_UI);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Unregister gyroscope listener
        sensorManager.unregisterListener(this);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE) {
            float rotationRate = event.values[1]; // Y-axis rotation rate (rad/s)
            int deltaAngle = Math.round(rotationRate * 10); // Adjust scaling factor as needed
            updateServoAngle(currentAngle + deltaAngle);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Not used
    }

    private void updateServoAngle(int angle) {
        // Clamp the angle within -90 to 90 degrees
        angle = Math.max(-90, Math.min(90, angle));
        if (angle != currentAngle) {
            currentAngle = angle;
            sendAngleToServer(angle);
            runOnUiThread(() -> currentAngleTextView.setText("Current Angle: " + currentAngle + "°"));
        }
    }

    private void resetServoAngle() {
        // Reset the angle to 0 degrees
        currentAngle = 0;
        sendAngleToServer(0);
        runOnUiThread(() -> {
            currentAngleTextView.setText("Current Angle: 0°");
            statusTextView.setText("Angle reset to 0°");
        });
    }

    private void sendAngleToServer(int angle) {
        new Thread(() -> {
            try {
                URL url = new URL("http://" + ipAddress + "/set_angle?angle=" + angle);
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setRequestMethod("GET");

                int responseCode = conn.getResponseCode();
                runOnUiThread(() -> {
                    if (responseCode == HttpURLConnection.HTTP_OK) {
                        statusTextView.setText("Angle updated to: " + angle + "°");
                    } else {
                        statusTextView.setText("Failed to update angle.");
                    }
                });
                conn.disconnect();
            } catch (Exception e) {
                runOnUiThread(() -> statusTextView.setText("Error: " + e.getMessage()));
            }
        }).start();
    }
}
