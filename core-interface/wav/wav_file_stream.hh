#pragma once
#include "dr_wav.h"
#include "util/lockfree_fifo_spsc_dyn.hh"
#include <array>
#include <cstdio>
#include <string_view>

namespace MetaModule
{

struct WavFileStream {

	// Construct WavFileStream with the size of the pre-buffer
	// Must be a power of 2
	// 1024*1024 is a good starting place
	WavFileStream(size_t max_samples);

	// Sets the maximum size of the buffer in samples.
	// If needed, the buffer may be cleared and reset.
	// Returns true if this happens, false if not.
	bool resize(size_t max_samples);

	size_t max_size() const;
	size_t buffer_size() const;

	////
	/// Load/unloading wav file:
	///

	// Given a file path, loads the wav file
	// The file will be read periodically until you call unload()
	bool load(std::string_view sample_path);

	// Releases use of the wav file
	void unload();

	bool is_loaded() const;

	////
	/// Reading into and out of the pre-buffer
	///

	// Reads audio frames from the wav file on disk
	// You can omit the number of frames if you want to use
	// an optimized amount based on the wav file format.
	// Since this blocks while accessing the disk,
	// DO NOT CALL THIS FROM THE AUDIO CONTEXT
	// Instead, call this periodically when the buffer is low
	// from an Async thread.
	// This gives an error (which is not yet handled) if you overflow the buffer.
	void read_frames_from_file();
	void read_frames_from_file(int num_frames);

	// Call this from the audio context to get the next sample.
	// Keep in mind if your file is stereo, then you should call this
	// twice in a row to get the whole frame.
	float pop_sample();

	////
	/// Current state of playback and buffering
	///

	// Returns the number of samples available in the buffer
	unsigned samples_available() const;

	// Returns the number of samples available in the buffer
	// which is just samples_available / # of channels
	unsigned frames_available() const;

	float sample_seconds() const;

	// Whether or not the file has been read to the end.
	// Useful if you want to loop, then you can seek back to the beginning
	bool is_eof() const;

	bool is_file_error() const;

	// The index of the audio frame that will be returned in the next call to
	// pop_sample().
	// When this equals the total_frames(), the entire sample has been played.
	unsigned current_playback_frame() const;

	////
	/// TRANSPORT
	///

	// Jumps the play head to a particular frame.
	// If the frame is in the pre-buffer, then the read head will just
	// jump to that position in the pre-buffer.
	// Otherwise the pre-buffer will be cleared/reset in anticipation
	// of starting to pre-buffer from this frame.
	// Must only be called by audio thread. The async thread
	// should call seek_frame_in_file after this.
	void reset_playback_to_frame(uint32_t frame_num);

	// Jumps the wav file reading head to a particular frame index in the
	// wav file. If the frame is alraedy in the pre-buffer,
	// then this does nothing at all. Otherwise it will do a file seek.
	// The audio thread should have just called reset_playback_to_frame()
	// when you call this.
	// Must be called by async filesystem thread.
	void seek_frame_in_file(uint32_t frame_num = 0);

	////
	/// Wav file information
	///

	// Whether the file is stereo or not
	bool is_stereo() const;

	// Total number of audio frames in the sample
	unsigned total_frames() const;

	// Sample rate of the wav file
	std::optional<uint32_t> wav_sample_rate() const;

private:
	bool is_frame_in_buffer(uint32_t frame_num) const;
	void reset_prebuff();
	uint32_t first_frame_in_buffer() const;

	size_t max_samples;

	drwav wav;

	bool eof = true;
	bool file_error = false;
	bool loaded = false;

	std::atomic<uint32_t> frames_in_buffer = 0;
	std::atomic<uint32_t> next_frame_to_write = 0;
	std::atomic<uint32_t> next_sample_to_read = 0;

	LockFreeFifoSpscDyn<float> pre_buff;

	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	// 4k: 1.9ms-2.8ms, avg 2.1ms = 1.90 kB/ms -- but lose a lot of time between async calls
	//16k: 11.7-12.3ms = 1.365 kB/ms
	static constexpr unsigned ReadBlockBytes = 16 * 1024;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// Worst case: 4kB of 8-bit mono data will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;
};

} // namespace MetaModule
