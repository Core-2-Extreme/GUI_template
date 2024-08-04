#if !defined(DEF_ENCODER_HPP)
#define DEF_ENCODER_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/encoder_types.h"
#include "system/util/media_types.h"

#if DEF_ENABLE_VIDEO_AUDIO_ENCODER_API

/**
 * @brief Create the output file.
 * @param path (in) Output file path.
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_encoder_create_output_file(const char* path, uint8_t session);

/**
 * @brief Initialize a audio encoder.
 * @param codec (in) Audio codec.
 * @param original_sample_rate (in) Input audio sample rate (in Hz).
 * @param encode_sample_rate (in) Encode audio sample rate (in Hz) (audio data will be converted
 * original_sample_rate -> encode_sample_rate internally when you call Util_audio_encoder_encode()).
 * @param bitrate (in) Audio bitrate (in bit).
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_audio_encoder_init(Audio_codec codec, uint32_t original_sample_rate, uint32_t encode_sample_rate, uint32_t bitrate, uint8_t session);

/**
 * @brief Initialize a video encoder.
 * @param codec (in) Video codec.
 * @param width (in) Video width.
 * @param height (in) Video height.
 * @param bitrate (in) Video bitrate (in bit).
 * @param fps (in) Video framerate.
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_video_encoder_init(Video_codec codec, uint32_t width, uint32_t height, uint32_t bitrate, uint32_t fps, uint8_t session);

/**
 * @brief Write a header (if needed by container).
 * Call it after calling Util_*_encoder_init().
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_encoder_write_header(uint8_t session);

/**
 * @brief Encode audio.
 * @param size (in) Raw audio data size (in byte).
 * @param raw_audio (in) Pointer for raw audio data (PCM_S16LE).
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_audio_encoder_encode(uint32_t size, uint8_t* raw_data, uint8_t session);

/**
 * @brief Encode a image.
 * @param raw_image (in) Pointer for raw image data (yuv420p).
 * @param session (in) Session number.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_video_encoder_encode(uint8_t* raw_image, uint8_t session);

/**
 * @brief Uninitialize encoders and close the output file.
 * Do nothing if file is not opend.
 * @param session (in) Session number.
 * @warning Thread dangerous (untested)
*/
void Util_encoder_close_output_file(uint8_t session);

#else

#define Util_encoder_create_output_file(...) DEF_ERR_DISABLED
#define Util_audio_encoder_init(...) DEF_ERR_DISABLED
#define Util_video_encoder_init(...) DEF_ERR_DISABLED
#define Util_encoder_write_header(...) DEF_ERR_DISABLED
#define Util_audio_encoder_encode(...) DEF_ERR_DISABLED
#define Util_video_encoder_encode(...) DEF_ERR_DISABLED
#define Util_encoder_close_output_file(...)

#endif //DEF_ENABLE_VIDEO_AUDIO_ENCODER_API

#if DEF_ENABLE_IMAGE_ENCODER_API

/**
 * @brief Encode image file.
 * @param path (in) File path.
 * @param raw_data (in) Pointer for raw image (RGB888).
 * @param width (in) Image width.
 * @param height (in) Image height.
 * @param codec (in) Image codec.
 * @param quality (in) Image quality (jpg only).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_image_encoder_encode(const char* path, uint8_t* raw_data, uint32_t width, uint32_t height, Image_codec codec, uint8_t quality);

#else

#define Util_image_encoder_encode(...) DEF_ERR_DISABLED

#endif //DEF_ENABLE_IMAGE_ENCODER_API

#endif //!defined(DEF_ENCODER_HPP)
