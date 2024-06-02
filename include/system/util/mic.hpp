#if !defined(DEF_MIC_HPP)
#define DEF_MIC_HPP

extern "C"
{
#include <stdbool.h>
#include <stdint.h>

enum Mic_sample_rate
{
	MIC_SAMPLE_RATE_INVALID = -1,

	MIC_SAMPLE_RATE_8182HZ,
	MIC_SAMPLE_RATE_10909HZ,
	MIC_SAMPLE_RATE_16364HZ,
	MIC_SAMPLE_RATE_32728HZ,

	MIC_SAMPLE_RATE_MAX,
};

#if defined(DEF_ENABLE_MIC_API)

/**
 * @brief Initialize a mic.
 * @param buffer_size (in) Internal mic buffer size.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_mic_init(uint32_t buffer_size);

/**
 * @brief Start recording.
 * @param sample_rate_mode (in) Audio sample rate.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_mic_start_recording(Mic_sample_rate sample_rate_mode);

/**
 * @brief Stop recording.
 * Do nothing if mic api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_mic_stop_recording(void);

/**
 * @brief Check whether recording in progress.
 * Always return false if mic api is not initialized.
 * @return True if recording in progress, otherwise false.
 * @warning Thread dangerous (untested)
*/
bool Util_mic_is_recording(void);

/**
 * @brief Query remaining (approximately) time.
 * Call Util_mic_get_audio_data() at least before it gets less than 300,
 * otherwise audio data might be overwritten.
 * Always return 0 mic api is not initialized.
 * @return Remaining time (in ms).
 * @warning Thread dangerous (untested)
*/
uint32_t Util_mic_query_remaining_buffer_time(void);

/**
 * @brief Get audio data.
 * @param raw_data (out) Pointer for raw audio data (PCM_S16LE), the pointer will be allocated inside of function.
 * @param size (out) Audio data size.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_mic_get_audio_data(uint8_t** raw_data, uint32_t* size);

/**
 * @brief Uninitialize a mic.
 * Do nothing if mic api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_mic_exit(void);

#else

#define Util_mic_init(...) DEF_ERR_DISABLED
#define Util_mic_start_recording(...) DEF_ERR_DISABLED
#define Util_mic_stop_recording()
#define Util_mic_is_recording() false
#define Util_mic_query_remaining_buffer_time() 0
#define Util_mic_get_audio_data(...) DEF_ERR_DISABLED
#define Util_mic_exit()

#endif //defined(DEF_ENABLE_MIC_API)
}
#endif //!defined(DEF_MIC_HPP)
