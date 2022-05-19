package com.stdevcon.coldchain;


import com.amazonaws.DefaultRequest;
import com.amazonaws.auth.AWS4Signer;
import com.amazonaws.auth.AWSCredentialsProvider;
import com.amazonaws.http.HttpMethodName;
import com.amazonaws.util.IOUtils;

import org.jetbrains.annotations.NotNull;

import okhttp3.HttpUrl;
import okhttp3.Interceptor;
import okhttp3.Request;
import okhttp3.Response;
import okio.Buffer;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.net.URI;
import java.util.Map;


public class SigV4Interceptor implements Interceptor {
    private AWSCredentialsProvider credentialsProvider;
    private String serviceName;

    public SigV4Interceptor(AWSCredentialsProvider credentialsProvider, String serviceName) {
        this.credentialsProvider = credentialsProvider;
        this.serviceName = serviceName;
    }

    @Override
    public @NotNull Response intercept(@NotNull Chain chain) throws IOException {
        Request originalRequest = chain.request();

        if (originalRequest.url().host().contains("amazonaws.com")) {
            AWS4Signer signer;
            if (originalRequest.url().encodedPath().contains("@")) {
                // the presence of "@" indicates that it doesn't need to be double URL-encoded
                signer = new AWS4Signer(false);
            } else {
                signer = new AWS4Signer();
            }

            DefaultRequest awsRequest = toAWSRequest(originalRequest, serviceName);
            signer.setServiceName(serviceName);
            signer.sign(awsRequest, credentialsProvider.getCredentials());

            return chain.proceed(toSignedOkHttpRequest(awsRequest, originalRequest));
        }

        return chain.proceed(originalRequest);
    }

    public static DefaultRequest toAWSRequest(Request request, String serviceName) {
        // clone the request (AWS-style) so that it can be populated with credentials
        DefaultRequest dr = new DefaultRequest(serviceName);

        // copy request info
        dr.setHttpMethod(HttpMethodName.valueOf(request.method()));

        HttpUrl url = request.url();
        dr.setResourcePath(url.uri().getPath());
        dr.setEndpoint(URI.create(String.format("%s://%s", url.scheme(), url.host())));

        // copy parameters
        for (String p: url.queryParameterNames()) {
            if (p != "") {
                dr.addParameter(p, url.queryParameter(p));
            }
        }

        // copy headers
        for (String h: request.headers().names()) {
            dr.addHeader(h, request.header(h));
        }

        // copy the request body
        byte[] bodyBytes;
        Buffer buffer = new Buffer();
        try {
            request.body().writeTo(buffer);
            bodyBytes = IOUtils.toByteArray(buffer.inputStream());
        } catch (Exception e) {
            bodyBytes = new byte[0];
        }
        dr.setContent(new ByteArrayInputStream(bodyBytes));

        return dr;
    }

    public static Request toSignedOkHttpRequest(DefaultRequest awsRequest, Request originalRequest) {
        // copy signed request back into an OkHttp Request
        Request.Builder builder = new Request.Builder();

        // copy headers from the signed request
        Map<String, String> requestHeaders = awsRequest.getHeaders();
        for (Map.Entry<String, String> header: requestHeaders.entrySet()) {
            //builder.addHeader(k, v)
            builder.addHeader(header.getKey(), header.getValue());
        }

        // start building an HttpUrl
        HttpUrl.Builder urlBuilder = new HttpUrl.Builder()
                .host(awsRequest.getEndpoint().getHost())
                .scheme(awsRequest.getEndpoint().getScheme())
                .encodedPath(awsRequest.getResourcePath());

        // copy parameters from the signed request
        Map<String, String> requestParams = awsRequest.getParameters();
        for (Map.Entry<String, String> param: requestParams.entrySet()) {
            urlBuilder.addQueryParameter(param.getKey(), param.getValue());
        }

        return builder.url(urlBuilder.build())
                .method(originalRequest.method(), originalRequest.body())
                .build();
    }
}
