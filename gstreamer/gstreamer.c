#include <gst/gst.h>

int main(int argc, char** argv) {

    GstElement* pipeline;
    GstBus* bus;
    GstMessage* msg;

    // gstreamer 초기화
    gst_init(&argc, &argv);

    // gst pipeline 생성
    pipeline = gst_parse_launch("playbin uri=http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", NULL);

    // 미디어 재생 시작
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);

    // 에러가 발생하거나 미디어의 끝에(End Of Stream) 도달할 때까지 대기
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // 사용한 끝난 자원 해제
    if (msg != NULL)
        gst_message_unref(msg);

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}