#!/usr/bin/env python3
"""
Generate a simple test sound (beep) for the audio system.
Creates a short 440Hz sine wave (note A) as a WAV file.
"""

import wave
import math
import struct

# Sound parameters
sample_rate = 44100  # Sample rate (Hz)
duration = 0.15      # Duration in seconds
frequency = 440.0    # Frequency (Hz) - note A
amplitude = 0.3      # Amplitude (0.0 to 1.0)

# Generate samples
num_samples = int(sample_rate * duration)
samples = []

for i in range(num_samples):
    # Generate sine wave
    sample = amplitude * math.sin(2.0 * math.pi * frequency * i / sample_rate)
    # Convert to 16-bit PCM
    pcm_sample = int(sample * 32767.0)
    samples.append(pcm_sample)

# Write WAV file
output_file = "assets/sounds/menu_click.wav"
with wave.open(output_file, 'w') as wav_file:
    # Set parameters: 1 channel (mono), 2 bytes per sample (16-bit), sample rate
    wav_file.setnchannels(1)
    wav_file.setsampwidth(2)
    wav_file.setframerate(sample_rate)

    # Write samples
    for sample in samples:
        wav_file.writeframes(struct.pack('<h', sample))

print(f"Generated test sound: {output_file}")
print(f"Duration: {duration}s, Frequency: {frequency}Hz")
