package com.stdevcon.coldchain.mapboxSigner;


import androidx.annotation.NonNull;

import com.stdevcon.coldchain.SigV4Interceptor;
import com.amazonaws.auth.CognitoCredentialsProvider;
import com.amazonaws.regions.Regions;
import com.facebook.react.bridge.Callback;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.uimanager.IllegalViewOperationException;
import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.maps.TelemetryDefinition;
import com.mapbox.mapboxsdk.module.http.HttpRequestUtil;

import org.jetbrains.annotations.NotNull;

import okhttp3.OkHttpClient;

public class MapboxSignerModule extends ReactContextBaseJavaModule {
    private final String SERVICE_NAME = "geo";

    public MapboxSignerModule(ReactApplicationContext reactContext) {
        super(reactContext); //required by React Native
    }

    @NonNull
    @NotNull
    @Override
    public String getName() {
        return "MapboxSigner";
    }

    @ReactMethod
    public void setupRequestSigner(String identityPoolId, Callback errorCallback, Callback successCallback) {
        try {
            TelemetryDefinition map = Mapbox.getTelemetry();
            if (map != null) {
                map.disableTelemetrySession();

                // Credential initialization
                CognitoCredentialsProvider credentialProvider = new CognitoCredentialsProvider(
                        identityPoolId,
                        Regions.fromName(identityPoolId.split(":")[0])
                );

                // Assign request signer
                HttpRequestUtil.setOkHttpClient(
                        new OkHttpClient.Builder()
                                .addInterceptor(new SigV4Interceptor(credentialProvider, SERVICE_NAME))
                                .build()
                );

                successCallback.invoke("Done");
            } else {
                errorCallback.invoke("Cannot find map instance. Make sure it is initialized");
            }

        }
        catch (IllegalViewOperationException e) {
            errorCallback.invoke(e);
        }
    }
}
