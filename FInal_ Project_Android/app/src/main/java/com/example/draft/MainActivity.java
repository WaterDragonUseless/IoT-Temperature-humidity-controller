package com.example.draft;

import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "IoT_Spring2024"; // For logging


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        EditText urlInput = findViewById(R.id.etServerAddress);
        EditText maxInput = findViewById(R.id.etMaxhum);
        EditText minInput = findViewById(R.id.etMinhum);


        Button btnMinhum = findViewById(R.id.btnMinhum);
        btnMinhum.setOnClickListener(view ->

        {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    // Retrieve the minimum humidity value from the EditText
                    String minValue = minInput.getText().toString();
                    if (!minValue.isEmpty()) {
                        // Construct the URL with the minimum humidity value as a query parameter
                        //String url = "http://192.168.1.47/Min?min=" + minValue;
                        String url = "http://192.168.1.47/Min?min=" + minValue;
                        // Send the HTTP request
                        String response = sendHTTPRequest(url);
                        // Log the server's response
                        Log.d("Server Response", response != null ? response : "No response from server");
                    } else {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                //Toast.makeText(getApplicationContext(), "Please enter a minimum humidity value.", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                }
            }).start();
        });


        Button btnMaxhum = findViewById(R.id.btnMaxhum);
        btnMaxhum.setOnClickListener(view ->

        {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    // Retrieve the minimum humidity value from the EditText
                    String maxValue = maxInput.getText().toString();
                    if (!maxValue.isEmpty()) {

                        //String url = "http://192.168.1.47/Max?max=" + maxValue;
                        String url = "http://192.168.1.47/Max?max=" + maxValue;
                        // Send the HTTP request
                        String response = sendHTTPRequest(url);

                   } else {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                //Toast.makeText(getApplicationContext(), "Please enter a minimum humidity value.", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                }
            }).start();
        });



        Button btnOn = findViewById(R.id.btnOn);
        btnOn.setOnClickListener(view ->

        {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Log.d("PASS", "Starting the system");

                    //sendHTTPRequest( "http://192.168.1.47/on");
                    sendHTTPRequest( "http://192.168.1.47/on");
                    String responseLED = sendHTTPRequest(urlInput.getText().toString() + "/LEDStatus");

                    //Turn on the system
                    //String responsePassword = sendHTTPRequest("http://192.168.1.47/Password");
                    String responsePassword = sendHTTPRequest("http://192.168.1.47/Password");

                    // Correct way to compare strings in Java
                    if (responsePassword.equals(urlInput.getText().toString())) {
                       // String temp = sendHTTPRequest("http://192.168.1.47/Unlock");LED is ON11111
                        String temp = sendHTTPRequest("http://192.168.1.47/Unlock");
                        Log.d("PASS", "System is unlocked");
                    }else {
                        Log.d("PASS", "Wrong password");
                    }
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                        }
                    });
                }

            }).start();
        });

        Button btnOff = findViewById(R.id.btnOff);
        btnOff.setOnClickListener(view ->
        {
            new Thread(new Runnable() {
                @Override
                public void run() {

                    //sendHTTPRequest(urlInput.getText().toString() + "/off");
                    //sendHTTPRequest( "http://192.168.1.47/off");
                    sendHTTPRequest( "http://192.168.1.47/off");
                    String responseLED = sendHTTPRequest(urlInput.getText().toString() + "/LEDStatus");
                    //String responsePassword = sendHTTPRequest("http://192.168.1.47/Password");
                    String responsePassword = sendHTTPRequest("http://192.168.1.47/Password");

                    // Send http to shutdown the machine
                    if (responsePassword.equals(urlInput.getText().toString())) {
                        //String temp = sendHTTPRequest("http://192.168.1.47/Lock");
                        String temp = sendHTTPRequest("http://192.168.1.47/Lock");
                    }

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                        }
                    });

                }

            }).start();

        });

    }


    private static String sendHTTPRequest(String requestUrl) {

        String res = null;

        try {
            URL url = new URL(requestUrl);
            HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
            urlConnection.setRequestMethod("GET");
            urlConnection.connect();
            int responseCode = urlConnection.getResponseCode();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                // Read the response if needed
                InputStream in = new BufferedInputStream(urlConnection.getInputStream());
                BufferedReader reader = new BufferedReader(new InputStreamReader(in));
                StringBuilder result = new StringBuilder();
                String line;
                while ((line = reader.readLine()) != null) {
                    result.append(line);
                }
                // Process the result here
                res = result.toString();
                Log.d(TAG, String.format("Request Success! URL: %s \n response: %s\n", url, res));
            } else {
                Log.d(TAG, String.format("Request Failed! URL: %s ResponseCode: %d\n", url, responseCode));
            }

        } catch (Exception e) {
            Log.d(TAG, String.format("Request Failed! Exception: %s\n", e.toString()));
        }

        return res;
    }






}

