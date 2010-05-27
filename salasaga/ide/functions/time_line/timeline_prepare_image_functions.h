#ifndef TIME_LINE_PREPARE_IMAGE_FUNCTIONS_
#define TIME_LINE_PREPARE_IMAGE_FUNCTIONS_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gboolean time_line_prepare_top_left(TimeLinePrivate *priv, gint width, gint height);
gboolean time_line_prepare_top_right(TimeLinePrivate *priv, gint width, gint height);

gboolean time_line_prepare_bottom_left(TimeLinePrivate *priv, gint width, gint height);
gboolean time_line_prepare_bottom_right(TimeLinePrivate *priv, gint width, gint height);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TIME_LINE_PREPARE_IMAGE_FUNCTIONS_ */
