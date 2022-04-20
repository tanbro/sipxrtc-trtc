# 腾讯的 trtc

<https://webcamtests.com/> 可以用于测试摄像头

这个演示： <https://web.sdk.qcloud.com/trtc/webrtc/demo/quick-demo-js/index.html>

在使用 Chrome/Chromium 打开上面的 URL 进行调测时，参考 <https://webrtc.org/getting-started/testing> ，其参数可以是：

- 有麦克风，但是没有摄像头时

  - 如果需要播放自定义的 Mock video:

    1. 制作一段的 [Y4M][] 格式的 Mock video:

       例如，我们将 TRTC SDK 的例子视频 `SDKs/trtc_7.9.54/examples/resource/ruguo-640x360.mp4`，转为 Raw Video。可以使用 ffmpeg 转换:

       ```bash
       ffmpeg -i SDKs/trtc_7.9.54/examples/resource/ruguo-640x360.mp4 -pix_fmt yuv420p SDKs/trtc_7.9.54/examples/resource/ruguo-420p.mp4.y4m
       ```

    2. 使用 Chrome 或 Chromium ，以上述 [Y4M][] Video 文件作为 Mock Web-Camera 打开(以 Chromium 为例):

       ```bash
       chromium-browser --temp-profile --disable-translate --use-fake-device-for-media-stream --use-fake-ui-for-media-stream --use-file-for-fake-video-capture=$(pwd)/SDKs/trtc_7.9.54/examples/resource/ruguo-420p.y4m https://web.sdk.qcloud.com/trtc/webrtc/demo/quick-demo-js/index.html
       ```

  - 如果只是测试，就不用指定 fake audio/video，Chrome 会自动播放测试音视频，此时，我们可以这样打开 TRTC 测试 WebApp:
  
    ```bash
    chromium-browser --temp-profile --disable-translate --use-fake-device-for-media-stream --use-fake-ui-for-media-stream https://web.sdk.qcloud.com/trtc/webrtc/demo/quick-demo-js/index.html
    ```

> 特别问题:
>
> 经测试，发现在混合时候，如果仅仅混合音频，也需要正确的对每一个抓去音频的远程用户使用 `setRegion` 设置视频区域（可以设置为0大小），否则 `MediaMixer` 被添加音频包时会出错(`10001`)。

[Y4M]: https://wiki.multimedia.cx/index.php/YUV4MPEG2 "YUV4MPEG2"
